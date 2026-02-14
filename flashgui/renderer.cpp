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

void c_renderer::initialize(IDXGISwapChain3* swapchain, ID3D12CommandQueue* cmd_queue, UINT sync_interval, UINT flags) {
	m_mode = (swapchain && cmd_queue) ? render_mode::hooked : render_mode::standalone;

	try {
		if (m_mode == render_mode::hooked) {
			DXGI_SWAP_CHAIN_DESC desc;

			HRESULT hr = swapchain->GetDesc(&desc);
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to get swapchain description, HRESULT: " + std::to_string(hr));
			}

			m_dx.buffer_count = desc.BufferCount;

			m_dx.swapchain = swapchain;
			m_dx.cmd_queue = cmd_queue;
			m_dx.swapchain_flags = flags;
			m_dx.sync_interval = sync_interval;

			if (!process->window.handle) {
				throw std::runtime_error("Window handle cannot be null in hooked mode");
			}

			m_dx.initialize_hooked();
		}
		else {
			m_dx.initialize_standalone(m_dx.buffer_count);
		}

		m_frame_resources.resize(m_dx.buffer_count);

		m_dx.create_pipeline();

		create_resources(false);

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

				int dst_index = ((dst_y + y) * atlas_w + (dst_x + x)) * 4;
				atlas[size_t(dst_index + 0)] = 255;   // R
				atlas[size_t(dst_index + 1)] = 255;   // G
				atlas[size_t(dst_index + 2)] = 255;   // B
				atlas[size_t(dst_index + 3)] = alpha; // A
			}
		}

		font_glyph_info info{};
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
	sub.RowPitch = static_cast<LONG_PTR>(atlas_w) * 4;
	sub.SlicePitch = static_cast<LONG_PTR>(atlas_w * atlas_h) * 4;

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

	// leave cmd closed begin_frame() will reset with PSO later

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

void c_renderer::release_resources() {
	for (auto& fr : m_frame_resources) {
		fr.release();
	}

	m_dx.release_resources();
}

void c_renderer::create_resources(bool create_heap_and_buffers) {

	if (create_heap_and_buffers)
		m_dx.create_resources();

	//store window size as long
	long width = process->window.width, height = process->window.height;

	for (auto& frame : m_frame_resources)
		frame.initialize(m_dx.device, D3D12_COMMAND_LIST_TYPE_DIRECT, size_t(1024 * 1024));

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

// use m_last_frame_time to calculate delta time for fps calculation and animations if needed.
int c_renderer::get_fps() const {
	return m_fps;
}

void c_renderer::wait_for_gpu() {
	m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
	frame_resource& fr = m_frame_resources[m_frame_index];
	fr.signal(m_dx.cmd_queue);
	fr.wait_for_gpu();
}

// Called at the beginning of each frame. Prepares command list and render target.
void c_renderer::begin_frame() {
	auto now = std::chrono::steady_clock::now();

	++m_frame_count;

	if (now - m_last_fps_update >= std::chrono::seconds(1)) {
		m_fps = static_cast<int>(m_frame_count); // frames rendered in the last second
		m_frame_count = 0;
		m_last_fps_update = now;
	}
	
    frame_resource& fr = m_frame_resources[m_frame_index];

    if (process->needs_resize()) {
		if (m_mode == fgui::render_mode::standalone) {
			fr.signal(m_dx.cmd_queue);
			fr.wait_for_gpu();

			release_resources();

			//exception wrapped resize_buffers 
			try {
				m_dx.resize_backbuffers(process->window.width, process->window.height);
			}
			catch (const std::exception& e) {
				std::cerr << "[flashgui] Error during frame resize: " << e.what() << std::endl;
			}

			create_resources();
		}
        return;
    }

	m_dx.srv->begin_frame(m_frame_index);

	if (m_mode == render_mode::standalone)
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
	//cmd->ClearRenderTargetView(rtv_handle, clear_color, 1, &clear_rect);

}

//TODO: optimize by multithreading upload and draw calls, and merging persistent instances into a single buffer updated only when instances change, while keeping immediate instances in a per-frame upload heap.
void c_renderer::end_frame() {
	if (process->needs_resize()) {
		// Reset the resize flag
		process->resize_complete();
		im_instances.clear(); // Clear immediate instances on resize
		return;
	}

	frame_resource& fr = m_frame_resources[m_frame_index];
	auto& cmd = fr.command_list;

	// Upload and draw persistent instances using per-frame upload heap (no merge)
	const size_t persistent_count = instances.size();
	const size_t immediate_count = im_instances.size();

	if (persistent_count >0) {
		D3D12_GPU_VIRTUAL_ADDRESS inst_va = fr.push_bytes(
			instances.data(),
			persistent_count * sizeof(shape_instance),
			16
		);

		D3D12_VERTEX_BUFFER_VIEW vbv_inst = {};
		vbv_inst.BufferLocation = inst_va;
		vbv_inst.SizeInBytes = static_cast<UINT>(persistent_count * sizeof(shape_instance));
		vbv_inst.StrideInBytes = sizeof(shape_instance);

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = { m_dx.m_quad_vbv, vbv_inst };
		cmd->IASetVertexBuffers(0,2, vbvs);
		cmd->IASetIndexBuffer(&m_dx.m_quad_ibv);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmd->DrawIndexedInstanced(
			6,
			static_cast<UINT>(persistent_count),
			0,0,0
		);
	}

	// Upload and draw immediate (per-frame) instances
	if (immediate_count >0) {
		D3D12_GPU_VIRTUAL_ADDRESS inst_va2 = fr.push_bytes(
			im_instances.data(),
			immediate_count * sizeof(shape_instance),
			16
		);

		D3D12_VERTEX_BUFFER_VIEW vbv_inst2 = {};
		vbv_inst2.BufferLocation = inst_va2;
		vbv_inst2.SizeInBytes = static_cast<UINT>(immediate_count * sizeof(shape_instance));
		vbv_inst2.StrideInBytes = sizeof(shape_instance);

		D3D12_VERTEX_BUFFER_VIEW vbvs2[2] = { m_dx.m_quad_vbv, vbv_inst2 };
		cmd->IASetVertexBuffers(0,2, vbvs2);
		cmd->IASetIndexBuffer(&m_dx.m_quad_ibv);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmd->DrawIndexedInstanced(
			6,
			static_cast<UINT>(immediate_count),
			0,0,0
		);

		im_instances.clear(); // immediate are per-frame
	}

	// Transition back to present
	D3D12_RESOURCE_BARRIER to_present = CD3DX12_RESOURCE_BARRIER::Transition(
		m_dx.get_back_buffer(m_frame_index).Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	//set resource barrier for present transition
	cmd->ResourceBarrier(1, &to_present);

	HRESULT hr = cmd->Close();
	if (FAILED(hr)) {
		char buf[256];
		sprintf_s(buf, "Close FAILED: 0x%08X\n", hr);
		OutputDebugStringA(buf);
		return;  // DON'T ExecuteCommandLists on bad list
	}

	// execute the command list
	ID3D12CommandList* cmd_lists[] = { cmd.Get() };
	m_dx.cmd_queue->ExecuteCommandLists(1, cmd_lists);

	fr.signal(m_dx.cmd_queue);

	if (m_mode == render_mode::standalone) {
		// present the swapchain
		HRESULT hr = m_dx.swapchain->Present(m_dx.sync_interval, m_dx.swapchain_flags);

		if (FAILED(hr)) {
			std::cerr << "Failed to present swapchain HRSESULT:" << hr << std::endl;

			//directx device was removed
			if (m_dx.device && hr == DXGI_ERROR_DEVICE_REMOVED) {
				HRESULT reason = m_dx.device->GetDeviceRemovedReason();
				std::cerr << "Device removed reason: " << std::hex << reason << std::endl;
			}
		}

		// In standalone mode, we can wait for the GPU to finish before starting the next frame
		if (m_dx.swapchain)
			m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
	}
}

void c_renderer::post_present(const HRESULT& present_result) {
	// increment the frame index
	if (m_dx.swapchain)
	m_frame_index = m_dx.swapchain->GetCurrentBackBufferIndex();
}

void c_renderer::remove_shape(shape_instance* instance) {
	auto it = std::find_if(instances.begin(), instances.end(),
		[instance](const shape_instance& inst) { return &inst == instance; });
	if (it != instances.end()) {
		instances.erase(it);
	}
}

shape_instance* c_renderer::add_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width, float rotation) {
	instances.push_back(shape_instance(pos, size, clr, rotation, outline_width, shape_type::quad));
		return &instances.back();
}

void c_renderer::draw_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width, float rotation) {
	if (process->needs_resize())
		return;
	im_instances.push_back(shape_instance(pos, size, clr, rotation, outline_width, shape_type::quad));
}

shape_instance* c_renderer::add_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width) {
	instances.push_back(shape_instance(start, end, clr, 0.f, width, shape_type::line));
	return &instances.back();
}

void c_renderer::draw_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width) {
	if (process->needs_resize())
		return;
	im_instances.push_back(shape_instance(start, end, clr, 0.f, width, shape_type::line));
}

shape_instance* c_renderer::add_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width, float rotation) {
	instances.push_back(shape_instance(pos, size, clr, rotation, width, shape_type::quad_outline));
	return &instances.back();
}

void c_renderer::draw_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width, float rotation) {
	if (process->needs_resize())
		return;
	im_instances.push_back(shape_instance(pos, size, clr, rotation, width, shape_type::quad_outline));
}

shape_instance* c_renderer::add_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	instances.push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle));
	return &instances.back();
}

void c_renderer::draw_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	if (process->needs_resize())
		return;
	im_instances.push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle));
}

shape_instance* c_renderer::add_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	instances.push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle_outline));
	return &instances.back();
}

void c_renderer::draw_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle, float outline_width) {
	if (process->needs_resize())
		return;
	im_instances.push_back(shape_instance(pos, size, clr, angle, outline_width, shape_type::circle_outline));
}

void c_renderer::draw_text(const std::string& text, vec2i pos, float scale, DirectX::XMFLOAT4 clr) {
	if (process->needs_resize())
		return;

	vec2i cursor = pos;
	for (char c : text) {
		auto it = m_font_glyphs.find(static_cast<uint32_t>(c));
		if (it == m_font_glyphs.end()) {
			std::cerr << "[flashgui] Warning: Glyph for character '" << c << "' not found in font glyphs.\n";
			continue;
		}
		const font_glyph_info& glyph = it->second;

		im_instances.push_back(shape_instance(cursor,
			vec2i(font8x8_glyph_width * scale, font8x8_glyph_height * scale),
			clr,
			0.f,
			1.f,
			shape_type::text_quad,
			DirectX::XMFLOAT4(glyph.u0, glyph.v0, glyph.u1, glyph.v1)
		));

		cursor.x += static_cast<int>(glyph.advance * scale);
	}
}
