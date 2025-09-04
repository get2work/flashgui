#include "pch.h"
#include "renderer.h"
#include "include/flashgui.h"

using namespace fgui;

LRESULT CALLBACK hk::window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	c_renderer* self = reinterpret_cast<c_renderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (self && self->window_proc(hwnd, msg, wparam, lparam)) {
		return true;
	}
	return DefWindowProcA(hwnd, msg, wparam, lparam);
}

void c_renderer::initialize(IDXGISwapChain3* swapchain, ID3D12CommandQueue* cmd_queue) {
	m_mode = (swapchain && cmd_queue) ? render_mode::hooked : render_mode::standalone;

	if (m_mode == render_mode::hooked) {
		m_dx.swapchain = swapchain;
		m_dx.cmd_queue = cmd_queue;

		if (!m_proc.window.handle) {
			throw std::runtime_error("Window handle cannot be null in hooked mode");
		}

		m_dx.initialize_hooked();
	} else {
		m_dx.initialize_standalone(m_proc, target_buffer_count);
	}

	m_frame_resources.resize(m_dx.buffer_count);

	m_dx.create_pipeline();

	const float width = static_cast<float>(m_proc.window.get_width());
	const float height = static_cast<float>(m_proc.window.get_height());
	
	m_viewport = {
		0.0f, 0.0f,
		width, height,
		0.0f, 1.0f
	};

	m_scissor_rect = {
		0l, 0l,
		m_proc.window.get_width(),
		m_proc.window.get_height()
	};

	// Initialize frame resources
	for (auto& fr : m_frame_resources) {
		fr.initialize(m_dx.device, D3D12_COMMAND_LIST_TYPE_DIRECT, size_t(1024 * 64));
	}

	m_transform_cb.projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, width,   // left, right
		height, 0.0f, // bottom, top
		0.0f, 1.0f   // near, far
	);

	m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
}

void c_renderer::resize_frame() {
	for (auto& fr : m_frame_resources) {
		fr.release(); // Reset command allocators and command lists
	}

	try {
		m_dx.resize_backbuffers(m_proc.window.get_width(), m_proc.window.get_height());
	}
	catch (const std::exception& e) {
		std::cerr << "[flashgui] Error during frame resize: " << e.what() << std::endl;
	}

	for (auto& fr : m_frame_resources) {
		fr.initialize(m_dx.device, D3D12_COMMAND_LIST_TYPE_DIRECT, size_t(1024 * 64));
	}

	const float width = static_cast<float>(m_proc.window.get_width());
	const float height = static_cast<float>(m_proc.window.get_height());
	
	m_viewport = {
		0.0f, 0.0f,
		width, height,
		0.0f, 1.0f
	};

	m_scissor_rect = {
		0l, 0l,
		m_proc.window.get_width(),
		m_proc.window.get_height()
	};

	m_transform_cb.projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, width,   // left, right
		height, 0.0f, // bottom, top
		0.0f, 1.0f   // near, far
	);
}

void c_renderer::begin_frame() {
    frame_resource& fr = m_frame_resources[m_frame_index];

    m_dx.srv->begin_frame(m_frame_index);

    if (m_pending_resize) {
        fr.signal(m_dx.cmd_queue);
        fr.wait_for_gpu();
        resize_frame();
        return;
    }

    fr.wait_for_gpu();
    fr.reset_upload_cursor();
    fr.reset(m_dx.pso_triangle);

    auto& cmd = fr.command_list;

    cmd->SetGraphicsRootSignature(m_dx.root_sig.Get());
    fr.cb_gpu_va = fr.push_cb(&m_transform_cb, sizeof(m_transform_cb));
    cmd->SetGraphicsRootConstantBufferView(0, fr.cb_gpu_va);
    cmd->RSSetViewports(1, &m_viewport);
    cmd->RSSetScissorRects(1, &m_scissor_rect);

    D3D12_RESOURCE_BARRIER to_rtv = CD3DX12_RESOURCE_BARRIER::Transition(
        m_dx.get_back_buffer(m_frame_index).Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmd->ResourceBarrier(1, &to_rtv);

    auto rtv_handle = m_dx.get_rtv_handle(m_frame_index);
    cmd->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
}

void c_renderer::end_frame() {
	if (m_pending_resize) {
		m_pending_resize = false; // Reset the resize flag
		return;
	}

	frame_resource& fr = m_frame_resources[m_frame_index];
	auto& cmd = fr.command_list;

	if (!instances.empty()) {
		// Upload instance data
		D3D12_GPU_VIRTUAL_ADDRESS instance_gpu_va = fr.push_bytes(
			instances.data(),
			instances.size() * sizeof(shape_instance),
			16
		);

		// Prepare vertex buffer views
		D3D12_VERTEX_BUFFER_VIEW vbv_inst = {};
		vbv_inst.BufferLocation = instance_gpu_va;
		vbv_inst.SizeInBytes = static_cast<UINT>(instances.size() * sizeof(shape_instance));
		vbv_inst.StrideInBytes = sizeof(shape_instance);

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = { m_dx.m_quad_vbv, vbv_inst };
		cmd->IASetVertexBuffers(0, 2, vbvs);

		// Index buffer for unit quad
		cmd->IASetIndexBuffer(&m_dx.m_quad_ibv);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Draw all instances
		cmd->DrawIndexedInstanced(
			6, // 6 indices for two triangles (unit quad)
			static_cast<UINT>(instances.size()), // instance count
			0, 0, 0
		);
	}

	// Transition back to present
	D3D12_RESOURCE_BARRIER to_present = CD3DX12_RESOURCE_BARRIER::Transition(
		m_dx.get_back_buffer(m_frame_index).Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	cmd->ResourceBarrier(1, &to_present);

	// close the command list
	if (FAILED(cmd->Close())) {
		throw std::runtime_error("Failed to close command list");
	}

	// execute the command list
	ID3D12CommandList* cmd_lists[] = { cmd.Get() };
	m_dx.cmd_queue->ExecuteCommandLists(1, cmd_lists);

	// present the swapchain
	if (FAILED(m_dx.swapchain->Present(1, 0))) {
		throw std::runtime_error("Failed to present swapchain");
	}

	// increment the frame index
	m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
	fr.signal(m_dx.cmd_queue);

	instances.clear();
}

void c_renderer::add_quad(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float outline_width, float rotation) {
	shape_instance inst;
	inst.pos = pos;           // Center position of the quad
	inst.size = size;          // Half-size of the quad (width/2, height/2)
	inst.rotation = rotation;      // Rotation in radians
	inst.stroke_width = outline_width;          // Filled quad, no outline
	inst.clr = clr;           // RGBA color (0..1)
	inst.shape_type = 0;             // 0 = filled quad

	instances.push_back(inst);
}

LRESULT c_renderer::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		// Handle resizing if needed
		return 0;
	case WM_KEYDOWN:
		// Handle key down events if needed
		return 0;
	case WM_KEYUP:
		// Handle key up events if needed
		return 0;
	case WM_CHAR:
		// Handle character input if needed
		return 0;
	case WM_SETFOCUS:
		// Handle focus events if needed
		return 0;
	case WM_KILLFOCUS:
		// Handle focus loss events if needed
		return 0;
	case WM_LBUTTONDOWN:
		// Handle left mouse button down events if needed
		return 0;
	case WM_RBUTTONDOWN:
		// Handle right mouse button down events if needed
		return 0;
	case WM_MOUSEMOVE:
		// Handle mouse move events if needed
		return 0;
	case WM_CLOSE:
		// Handle close events if needed
		return 0;

	default:
		return 0;
	}
}
