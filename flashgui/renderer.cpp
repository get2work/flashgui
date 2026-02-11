#include "pch.h"
#include "renderer.h"
#include "include/flashgui.h"

using namespace fgui;

LRESULT CALLBACK hk::window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (process) {
		if (process->window_proc(hwnd, msg, wparam, lparam))
			return true;
	}
	return DefWindowProcA(hwnd, msg, wparam, lparam);
}

void c_renderer::initialize(IDXGISwapChain3* swapchain, ID3D12CommandQueue* cmd_queue) {
	m_mode = (swapchain && cmd_queue) ? render_mode::hooked : render_mode::standalone;

	try {
		if (m_mode == render_mode::hooked) {
			m_dx.swapchain = swapchain;
			m_dx.cmd_queue = cmd_queue;

			if (!process->window.handle) {
				throw std::runtime_error("Window handle cannot be null in hooked mode");
			}

			m_dx.initialize_hooked();
		}
		else {
			m_dx.initialize_standalone(target_buffer_count);
		}

		m_frame_resources.resize(m_dx.buffer_count);

		m_dx.create_pipeline();

		const float width = static_cast<float>(process->window.get_width());
		const float height = static_cast<float>(process->window.get_height());

		m_viewport = {
			0.0f, 0.0f,
			width, height,
			0.0f, 1.0f
		};

		m_scissor_rect = {
			0l, 0l,
			process->window.get_width(),
			process->window.get_height()
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

		//initialize fonts
		initialize_fonts();
	}
	catch (const std::exception& e) {
		std::cerr << "Exception thrown during renderer initialization!\n" << e.what() << std::endl;
	}
}

void c_renderer::initialize_fonts()
{
	using namespace fonts;

	const int glyph_w = font8x8_glyph_width;
	const int glyph_h = font8x8_glyph_height;
	const int cols = 32;
	const int rows = (font8x8_glyph_count + cols - 1) / cols;
	const int atlas_w = cols * glyph_w;
	const int atlas_h = rows * glyph_h;

	// 4 bytes per texel (RGBA)
	std::vector<uint8_t> atlas(atlas_w * atlas_h * 4, 0);

	for (int i = 0; i < font8x8_glyph_count; ++i) {
		const auto& g = font8x8_glyphs[i];

		int gx = i % cols;
		int gy = i / cols;
		int dst_x = gx * glyph_w;
		int dst_y = gy * glyph_h;

		for (int y = 0; y < glyph_h; ++y) {
			for (int x = 0; x < glyph_w; ++x) {
				uint32_t v = g.bitmap[y * glyph_w + x];
				uint8_t  alpha = v ? 255u : 0u;

				int dstIndex = ((dst_y + y) * atlas_w + (dst_x + x)) * 4;
				atlas[dstIndex + 0] = 255;   // R
				atlas[dstIndex + 1] = 255;   // G
				atlas[dstIndex + 2] = 255;   // B
				atlas[dstIndex + 3] = alpha; // A
			}
		}

		font_glyph_info info;
		info.u0 = dst_x / float(atlas_w);
		info.v0 = dst_y / float(atlas_h);
		info.u1 = (dst_x + glyph_w) / float(atlas_w);
		info.v1 = (dst_y + glyph_h) / float(atlas_h);
		info.advance = float(glyph_w);

		m_font_glyphs[g.char_code] = info;
	}

	// Texture: R8G8B8A8
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = atlas_w;
	desc.Height = atlas_h;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(m_dx.device->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_font_texture)))) {
		throw std::runtime_error("Failed to create font texture");
	}

	// Upload buffer
	UINT64 uploadSize = GetRequiredIntermediateSize(m_font_texture.Get(), 0, 1);
	ComPtr<ID3D12Resource> upload;
	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
	if (FAILED(m_dx.device->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload)))) {
		throw std::runtime_error("Failed to create font upload buffer");
	}

	// Subresource data (note RowPitch/SlicePitch in bytes)
	D3D12_SUBRESOURCE_DATA sub{};
	sub.pData = atlas.data();
	sub.RowPitch = atlas_w * 4;
	sub.SlicePitch = atlas_w * atlas_h * 4;

	auto& fr = m_frame_resources[m_frame_index];
	auto& cmd = fr.command_list;

	// Reset upload command list
	cmd->Reset(fr.command_allocator.Get(), nullptr);

	// Upload subresource
	UpdateSubresources(cmd.Get(), m_font_texture.Get(), upload.Get(), 0, 0, 1, &sub);

	// Transition to PS-readable
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_font_texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	cmd->ResourceBarrier(1, &barrier);

	// Execute and wait
	cmd->Close();
	ID3D12CommandList* lists[] = { cmd.Get() };
	m_dx.cmd_queue->ExecuteCommandLists(1, lists);
	fr.signal(m_dx.cmd_queue);
	fr.wait_for_gpu();

	// Leave it closed; begin_frame() will reset with PSO later

	// SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
	srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv.Texture2D.MipLevels = 1;

	m_font_srv = m_dx.srv->allocate(m_font_texture, srv, false, 0);

	if (m_font_srv.ptr) {
		std::cout << "[flashgui] Font atlas created successfully. Glyph count: "
			<< m_font_glyphs.size() << std::endl;
	}
	else {
		throw std::runtime_error("Failed to allocate font SRV");
	}
}


void c_renderer::resize_frame() {

	//release all frame specific resources
	for (auto& fr : m_frame_resources) {
		fr.release(); // Reset command allocators and command lists
	}

	//store window size as long
	long width = process->window.get_width(), height = process->window.get_height();

	//exception wrapped resize_buffers 
	try {
		m_dx.resize_backbuffers(width, height);
	}
	catch (const std::exception& e) {
		std::cerr << "[flashgui] Error during frame resize: " << e.what() << std::endl;
	}

	for (auto& frame : m_frame_resources)
		frame.initialize(m_dx.device, D3D12_COMMAND_LIST_TYPE_DIRECT, size_t(1024 * 64));

	const float fwidth = static_cast<float>(width);
	const float fheight = static_cast<float>(height);
	
	m_viewport = {
		0.0f, 0.0f,
		fwidth, fheight,
		0.0f, 1.0f
	};

	m_scissor_rect = {
		0l, 0l,
		width,
		height
	};

	m_transform_cb.projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, fwidth,   // left, right
		fheight, 0.0f, // bottom, top
		0.0f, 1.0f   // near, far
	);

	m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
}

void c_renderer::begin_frame() {
    frame_resource& fr = m_frame_resources[m_frame_index];

    if (process->needs_resize()) {
        fr.signal(m_dx.cmd_queue);
        fr.wait_for_gpu();
        resize_frame();
        return;
    }

	m_dx.srv->begin_frame(m_frame_index);

    fr.wait_for_gpu();
    fr.reset_upload_cursor();
    fr.reset(m_dx.pso_triangle);

    auto& cmd = fr.command_list;

	ID3D12DescriptorHeap* heaps[] = { m_dx.srv->m_heap.Get() };
	cmd->SetDescriptorHeaps(1, heaps);

    cmd->SetGraphicsRootSignature(m_dx.root_sig.Get());
    fr.cb_gpu_va = fr.push_cb(&m_transform_cb, sizeof(m_transform_cb));
    cmd->SetGraphicsRootConstantBufferView(0, fr.cb_gpu_va);
	cmd->SetGraphicsRootDescriptorTable(1, m_font_srv);
    cmd->RSSetViewports(1, &m_viewport);
    cmd->RSSetScissorRects(1, &m_scissor_rect);

    D3D12_RESOURCE_BARRIER to_rtv = CD3DX12_RESOURCE_BARRIER::Transition(
        m_dx.get_back_buffer(m_frame_index).Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmd->ResourceBarrier(1, &to_rtv);

    auto rtv_handle = m_dx.get_rtv_handle(m_frame_index);
	const FLOAT clear_color[4] = { 0.f, 0.f, 0.f, 0.f };

    cmd->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
	D3D12_RECT clear_rect = { 0, 0, process->window.get_width(), process->window.get_height() };
	cmd->ClearRenderTargetView(rtv_handle, clear_color, 1, &clear_rect);
}

void c_renderer::end_frame() {
	if (process->needs_resize()) {
		// Reset the resize flag
		process->resize_complete();
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
		//size of all shapes
		vbv_inst.SizeInBytes = static_cast<UINT>(instances.size() * sizeof(shape_instance));
		//size of each shape
		vbv_inst.StrideInBytes = sizeof(shape_instance);

		//set vertex buffer views
		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = { m_dx.m_quad_vbv, vbv_inst };
		cmd->IASetVertexBuffers(0, 2, vbvs);

		// Index buffer for unit quad
		cmd->IASetIndexBuffer(&m_dx.m_quad_ibv);

		//drawing triangles, lines represented as thin rectangles
		//formed by 2 triangles
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Draw all instances
		// TODO: Store different instance arrays
		// Change index count for each.
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

	//set resource barrier for present transition
	cmd->ResourceBarrier(1, &to_present);

	// close the command list
	if (FAILED(cmd->Close())) {
		throw std::runtime_error("Failed to close command list");
	}

	// execute the command list
	ID3D12CommandList* cmd_lists[] = { cmd.Get() };
	m_dx.cmd_queue->ExecuteCommandLists(1, cmd_lists);

	// present the swapchain
	HRESULT hr = m_dx.swapchain->Present(1, 0);
	
	if (FAILED(hr)) {
		std::cerr << "Failed to present swapchain HRSESULT:" << hr << std::endl;
		
		//directx device was removed
		if (m_dx.device && hr == DXGI_ERROR_DEVICE_REMOVED) {
			HRESULT reason = m_dx.device->GetDeviceRemovedReason();
			std::cerr << "Device removed reason: " << std::hex << reason << std::endl;
		}
	}

	// increment the frame index
	m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
	fr.signal(m_dx.cmd_queue);

	//clear shape instances
	instances.clear();
}

void c_renderer::add_quad(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float outline_width, float rotation) {
	if (process->needs_resize())
		return;

	shape_instance inst;
	inst.pos = pos;           // Beginning position
	inst.size = size;          // Full width and height
	inst.rotation = rotation;      // Rotation in radians
	inst.stroke_width = outline_width;          // Filled quad, no outline
	inst.clr = clr;           // RGBA color (0..1)
	inst.shape_type = shape_type::quad;             // 0 = filled quad

	instances.push_back(inst);
}

void c_renderer::add_line(DirectX::XMFLOAT2 start, DirectX::XMFLOAT2 end, DirectX::XMFLOAT4 clr, float width) {
	if (process->needs_resize())
		return;

	shape_instance inst;
	inst.pos = start;
	inst.size = end;
	inst.rotation = 0;
	inst.stroke_width = width;
	inst.clr = clr;
	inst.shape_type = shape_type::line;
	
	instances.push_back(inst);
}

void c_renderer::add_quad_outline(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float width, float rotation) {
	if (process->needs_resize())
		return;

	shape_instance inst;
	inst.pos = pos;
	inst.size = size;
	inst.clr = clr;
	inst.stroke_width = width;
	inst.rotation = rotation;
	inst.shape_type = shape_type::quad_outline;

	instances.push_back(inst);
}

void c_renderer::add_circle(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	if (process->needs_resize())
		return;

	shape_instance inst;
	inst.pos = pos;
	inst.size = size;
	inst.clr = clr;
	inst.rotation = angle;
	inst.stroke_width = outline_width;
	inst.shape_type = shape_type::circle;

	instances.push_back(inst);
}

void c_renderer::add_circle_outline(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	if (process->needs_resize())
		return;

	shape_instance inst;
	inst.pos = pos;
	inst.size = size;
	inst.rotation = angle;
	inst.clr = clr;
	inst.stroke_width = outline_width;
	inst.shape_type = shape_type::circle_outline;

	instances.push_back(inst);
}

void c_renderer::draw_text(const std::string& text, DirectX::XMFLOAT2 pos, float scale, DirectX::XMFLOAT4 clr) {
	if (process->needs_resize())
		return;

	DirectX::XMFLOAT2 cursor = pos;
	for (char c : text) {
		auto it = m_font_glyphs.find(static_cast<uint32_t>(c));
		if (it == m_font_glyphs.end()) {
			std::cerr << "[flashgui] Warning: Glyph for character '" << c << "' not found in font glyphs.\n";
			continue;
		}
		const font_glyph_info& glyph = it->second;
		shape_instance inst;
		
		inst.pos = cursor;
		
		inst.size = DirectX::XMFLOAT2(
			font8x8_glyph_width * scale,
			font8x8_glyph_height * scale
		);

		inst.clr = clr;
		inst.stroke_width = 1.f; // filled
		inst.rotation = 0.f;
		inst.shape_type = shape_type::text_quad;
		inst.uv = DirectX::XMFLOAT4(glyph.u0, glyph.v0, glyph.u1, glyph.v1);
		instances.push_back(inst);
		cursor.x += glyph.advance * scale;
	}
}
