#include "pch.h"
#include "dxgicontext.h"
#include "include/flashgui.h"

using namespace fgui;

void s_dxgicontext::initialize(IDXGISwapChain3* swapchain, ID3D12CommandQueue* cmd_queue, UINT sync_interval, UINT flags) {
	hooked = swapchain && cmd_queue;

	if (hooked) {
		this->swapchain.Attach(swapchain);
		this->cmd_queue.Attach(cmd_queue);
		this->sync_interval = sync_interval;
		this->swapchain_flags = flags;

		DXGI_SWAP_CHAIN_DESC desc;
		HRESULT hr = this->swapchain->GetDesc(&desc);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to get swapchain description, HRESULT: " + std::to_string(hr));
		}

		buffer_count = desc.BufferCount;

		initialize_hooked();
	} else {
		initialize_standalone(buffer_count);
	}

	frame_resources.resize(buffer_count);
	create_pipeline();

	create_resources();
}

void s_dxgicontext::initialize_hooked() {

	HRESULT hr = swapchain->GetDevice(IID_PPV_ARGS(&device));
	if (FAILED(hr)) {
		std::string msg = "Failed to get device from swapchain, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	create_rtv_heap();
	create_backbuffers();
}

void s_dxgicontext::initialize_standalone(const uint32_t& target_buf_count) {
	buffer_count = target_buf_count;

	create_device_and_swapchain();
	create_rtv_heap();
	create_backbuffers();
}

void s_dxgicontext::create_device_and_swapchain() {
	HRESULT hr = S_OK;

	// Create DXGI factory
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory));
	if (FAILED(hr)) {
		std::string msg = "Failed to create DXGI factory, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	// New DXGI factory feature for DirectX 12, cleaner than enuming all adapters
	hr = dxgi_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	if (FAILED(hr) || !adapter) {
		std::string msg = "Failed to enumerate adapter, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	// Create D3D12 device
	const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	for (auto level : levels) {
		hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			feature_level = level;
			break;
		}
	}

	if (FAILED(hr)) {
		std::string msg = "Failed to create D3D12 device, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // No special flags
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Direct command queue

	hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue));
	if (FAILED(hr)) {
		std::string msg = "Failed to create command queue, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	swapchain_desc.BufferCount = buffer_count; // Default buffer count
	swapchain_desc.Width = process->window.width; // Set the width of the swapchain
	swapchain_desc.Height = process->window.height; // Set the height of the swapchain
	swapchain_desc.Format = dxgiformat; // Set the format of the swapchain
	swapchain_desc.Stereo = FALSE; // No stereo rendering
	swapchain_desc.SampleDesc.Count = sample_count; // No multisampling
	swapchain_desc.SampleDesc.Quality = num_quality_levels; // No multisampling quality
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Use the swapchain for rendering
	swapchain_desc.Scaling = DXGI_SCALING_STRETCH; // Stretch the swapchain to fit the window
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Allow mode switching
	swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // No alpha mode
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Use flip discard for better performance
	// Create the swapchain for m_hwnd
	ComPtr<IDXGISwapChain1> swapchain1;
	hr = dxgi_factory->CreateSwapChainForHwnd(
		cmd_queue.Get(), // Command queue for the swapchain
		process->window.handle, // Handle to the overlay window
		&swapchain_desc, // Swapchain description
		nullptr, // No additional parameters
		nullptr, // No restrict to output
		&swapchain1 // Output swapchain pointer
	);
	if (FAILED(hr)) {
		std::string msg = "Failed to create swapchain, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	hr = dxgi_factory->MakeWindowAssociation(process->window.handle, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr)) {
		std::string msg = "Failed to make window association, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	hr = swapchain1.As(&swapchain);
	if (FAILED(hr)) {
		std::string msg = "Failed to query IDXGISwapChain3 from IDXGISwapChain1, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}
}

void s_dxgicontext::create_rtv_heap() {
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.NumDescriptors = buffer_count;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));

	if (FAILED(hr)) {
		std::string msg = "Failed to create RTV heap, HRESULT: " + std::to_string(hr) + "\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	// Get the size of each descriptor in the RTV heap
	rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void s_dxgicontext::create_backbuffers() {
	back_buffers.resize(buffer_count);
	rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < buffer_count; ++i) {
		HRESULT hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&back_buffers[i]));
		if (FAILED(hr)) {
			std::string msg = "Failed to get back buffer " + std::to_string(i) + ", HRESULT: " + std::to_string(hr) + "\n";
			OutputDebugStringA(msg.c_str());
			throw std::runtime_error(msg);
		}
		device->CreateRenderTargetView(back_buffers[i].Get(), nullptr, rtv_handle);
		rtv_handle.ptr += rtv_descriptor_size; // Move to the next descriptor handle
	}
}

void s_dxgicontext::wait_for_gpu() {
	frame_index = swapchain->GetCurrentBackBufferIndex();
	frame_resource& fr = frame_resources[frame_index];
	fr.signal(cmd_queue);
	fr.wait_for_gpu();
}

void s_dxgicontext::create_resources() {

	for (auto& frame : frame_resources)
		frame.initialize(device, D3D12_COMMAND_LIST_TYPE_DIRECT, size_t(1024 * 1024));

	// CRITICAL: Update projection matrix and viewport/scissor for the new window size
	// even on the first frame after resize
	const float fwidth = static_cast<float>(process->window.width);
	const float fheight = static_cast<float>(process->window.height);

	viewport = {
		0.0f, 0.0f,
		fwidth, fheight,
		0.0f, 1.0f
	};

	scissor_rect = {
		0l, 0l,
		static_cast<long>(process->window.width),
		static_cast<long>(process->window.height)
	};

	transform_cb.projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, fwidth,   // left, right
		fheight, 0.0f, // bottom, top
		0.0f, 1.0f   // near, far
	);

	frame_index = swapchain->GetCurrentBackBufferIndex();
}

void s_dxgicontext::release_resources() {
	for (auto& fr : frame_resources) {
		fr.release();
	}

	// Release backbuffers
	for (auto& buffer : back_buffers) {
		//srv->free_if_backbuffer(buffer.Get());
		buffer.Reset();
	}
	back_buffers.clear();
}

void s_dxgicontext::resize_backbuffers(UINT width, UINT height, DXGI_FORMAT format) const {
	if (!swapchain) {
		throw std::runtime_error("Swapchain is not initialized, cannot resize backbuffers");
	}

	try {
		//call swapchain resize buffers
		HRESULT hr = swapchain->ResizeBuffers(buffer_count, width, height, format, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		if (FAILED(hr)) {
			std::string msg = "Failed to resize swapchain buffers, HRESULT: " + std::to_string(hr) + "\n";
			OutputDebugStringA(msg.c_str());
			throw std::runtime_error(msg);
		}

	}
	catch (...) {
		std::throw_with_nested(std::runtime_error("Error during backbuffer resize"));
	}
}

void s_dxgicontext::create_pipeline() {
	if (!device)
		throw std::runtime_error("D3D12 device is not initialized, cannot create root signature and PSO");

	if (!shaders)
		shaders = std::make_unique<c_shader_loader>();

	shaders->initialize(device);

	// Build ImGui-exact root signature
	D3D12_DESCRIPTOR_RANGE1 srv_range{};
	srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srv_range.NumDescriptors = 1;
	srv_range.BaseShaderRegister = 0;
	srv_range.RegisterSpace = 0;
	srv_range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
	srv_range.OffsetInDescriptorsFromTableStart = 0; // CRITICAL

	D3D12_STATIC_SAMPLER_DESC samp{};
	samp.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samp.MaxLOD = D3D12_FLOAT32_MAX;
	samp.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samp.ShaderRegister = 0;
	samp.RegisterSpace = 0;
	samp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	try {
		// use build_safe() to guarantee the ImGui-exact signature serialized with version 1.0
		root_sig = c_rootsig_builder(device)
			.build_safe();
	}
	catch (const std::exception& ex) {
		std::string msg = "Failed to create root signature: ";
		msg += ex.what();
		msg += "\n";
		OutputDebugStringA(msg.c_str());
		std::cerr << msg;
		throw;
	}

	if (!root_sig)
		throw std::runtime_error("Failed to create root signature (unknown reason)");

	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout = {
		{ "POSITION",   0, DXGI_FORMAT_R32G32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD",   1, DXGI_FORMAT_R32G32_FLOAT,    1, 0,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD",   2, DXGI_FORMAT_R32G32_FLOAT,    1, 8,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD",   3, DXGI_FORMAT_R32_FLOAT,       1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD",   4, DXGI_FORMAT_R32_FLOAT,       1, 20, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD",   5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 24, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD",   6, DXGI_FORMAT_R32_UINT,        1, 40, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD",   7, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 44, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	// Defensive RTV validation
	if (dxgiformat == DXGI_FORMAT_UNKNOWN) {
		std::string msg = "Backbuffer format is unknown, cannot create PSO\n";
		OutputDebugStringA(msg.c_str());
		throw std::runtime_error(msg);
	}

	try {
		pso_triangle = c_pso_builder(device)
			.set_root_signature(root_sig.Get())
			.set_vertex_shader(shaders->get_vs_blob())
			.set_pixel_shader(shaders->get_ps_blob())
			.set_input_layout(input_layout)
			.set_rtv_format(0, dxgiformat)
			.disable_depth()
			.enable_alpha_blending(true) // <- enable premultiplied blending
			.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
			.set_dsv_format(DXGI_FORMAT_UNKNOWN)
			.set_num_render_targets(1)
			.set_sample_desc({ sample_count, num_quality_levels })
			.allow_empty_input_layout(true) // tolerate missing input layout in certain hooked environments
			.build();
	}
	catch (const std::exception& ex) {
		std::string msg = "Failed to create PSO for triangles: ";
		msg += ex.what();
		msg += "\n";
		OutputDebugStringA(msg.c_str());
		std::cerr << msg;
		throw;
	}
}

void s_dxgicontext::begin_frame() {
	frame_resource& fr = get_current_frame_resource();

	if (process->needs_resize()) {
		if (!hooked) {
			fr.signal(cmd_queue);
			fr.wait_for_gpu();

			release_resources();

			//exception wrapped resize_buffers 
			try {
				resize_backbuffers(process->window.width, process->window.height);
			}
			catch (const std::exception& e) {
				std::cerr << "[flashgui] Error during frame resize: " << e.what() << std::endl;
			}

			create_backbuffers();
			create_resources();
		}
		return;
	}

	//m_dx->srv->begin_frame(m_frame_index);

	if (!hooked)
		fr.wait_for_gpu();

	fr.reset_upload_cursor();
	fr.reset(pso_triangle);

	auto& cmd = fr.command_list;

	// Bind SRV descriptor heap (font + backbuffer SRVs, etc.)
	ID3D12DescriptorHeap* heaps[] = { fonts->get_font_srv_heap().Get()};
	cmd->SetDescriptorHeaps(1, heaps);

	// Set root signature
	cmd->SetGraphicsRootSignature(root_sig.Get());

	// Set the projection matrix as 16x32-bit constants (ImGui-exact root signature expects this).
	// Build the projection exactly like ImGui: an orthographic projection mapped to clip space.
	// Left = 0, Right = width, Top = 0, Bottom = height
	const float L = 0.0f;
	const float R = static_cast<float>(viewport.Width);
	const float T = 0.0f;
	const float B = static_cast<float>(viewport.Height);

	// ImGui's projection matrix layout (row-major float[4][4]):
	// { { 2/(R-L),      0,        0, 0 },
	//   {      0,  2/(T-B),      0, 0 },
	//   {      0,      0,   0.5f, 0 },
	//   { (R+L)/(L-R), (T+B)/(B-T), 0.5f, 1 } };
	// We'll fill a float[16] in the same order and pass it directly.
	float proj[16];
	ZeroMemory(proj, sizeof(proj));
	proj[0] = 2.0f / (R - L);
	proj[5] = 2.0f / (T - B);
	proj[10] = 0.5f;
	proj[12] = (R + L) / (L - R);
	proj[13] = (T + B) / (B - T);
	proj[14] = 0.5f;
	proj[15] = 1.0f;

	// Set 16 x 32-bit root constants at slot 0 (matches the ImGui DX12 root signature)
	cmd->SetGraphicsRoot32BitConstants(0, 16, proj, 0);

	cmd->RSSetViewports(1, &viewport);
	cmd->RSSetScissorRects(1, &scissor_rect);

	D3D12_RESOURCE_BARRIER to_rtv = CD3DX12_RESOURCE_BARRIER::Transition(
		get_back_buffer(frame_index).Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	cmd->ResourceBarrier(1, &to_rtv);

	auto rtv_handle = get_rtv_handle(frame_index);
	const FLOAT clear_color[4] = { 0.f, 0.f, 0.f, 0.f };

	cmd->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
}

void s_dxgicontext::end_frame(std::vector<std::vector<shape_instance>>& shapes) {
	frame_resource& fr = get_current_frame_resource();

	auto& cmd = fr.command_list;

	// *** DYNAMIC QUAD + INSTANCE ***
	D3D12_GPU_VIRTUAL_ADDRESS quad_verts_va = fr.push_bytes(quad_vertices, sizeof(quad_vertices), 16);
	D3D12_VERTEX_BUFFER_VIEW vbv_quad = {};
	vbv_quad.BufferLocation = quad_verts_va;
	vbv_quad.SizeInBytes = sizeof(quad_vertices);
	vbv_quad.StrideInBytes = 8;  // float2

	D3D12_GPU_VIRTUAL_ADDRESS quad_inds_va = fr.push_bytes(quad_indices, sizeof(quad_indices), 4);
	D3D12_INDEX_BUFFER_VIEW quad_ibv = {};
	quad_ibv.BufferLocation = quad_inds_va;
	quad_ibv.SizeInBytes = sizeof(quad_indices);
	quad_ibv.Format = DXGI_FORMAT_R16_UINT;

	for (uint32_t i = 0; i < shapes.size(); i++) {

		if (shapes[i].empty())
			continue;

		D3D12_GPU_VIRTUAL_ADDRESS inst_va = fr.push_bytes(
			shapes[i].data(), 
			shapes[i].size() * sizeof(shape_instance), 
			16);

		D3D12_VERTEX_BUFFER_VIEW vbv_inst = {};
		vbv_inst.BufferLocation = inst_va;
		vbv_inst.SizeInBytes = static_cast<UINT>(shapes[i].size() * sizeof(shape_instance));
		vbv_inst.StrideInBytes = sizeof(shape_instance);

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = { vbv_quad, vbv_inst };
		cmd->IASetVertexBuffers(0, 2, vbvs);

		D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc = fonts->get_font_srv_gpu(i);
		cmd->SetGraphicsRootDescriptorTable(1, gpu_desc);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd->IASetIndexBuffer(&quad_ibv);

		cmd->DrawIndexedInstanced(
			6,                                  // indices per instance (quad)
			static_cast<UINT>(shapes[i].size()), // instance count
			0,                                   // start index location
			0,                                   // base vertex location
			0                                    // start instance location
		);

		shapes[i].clear(); // Clear shapes after rendering to free up memory for the next frame
	}

	// Transition back to present
	D3D12_RESOURCE_BARRIER to_present = CD3DX12_RESOURCE_BARRIER::Transition(
		get_back_buffer(frame_index).Get(),
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
	cmd_queue->ExecuteCommandLists(1, cmd_lists);

	fr.signal(cmd_queue);

	if (!hooked) {
		// present the swapchain
		HRESULT hr = swapchain->Present(sync_interval, swapchain_flags);

		if (FAILED(hr)) {
			std::cerr << "Failed to present swapchain HRESULT:" << hr << std::endl;

			//directx device was removed
			if (device && hr == DXGI_ERROR_DEVICE_REMOVED) {
				HRESULT reason = device->GetDeviceRemovedReason();
				std::cerr << "Device removed reason: " << std::hex << reason << std::endl;
			}
		}

		// In standalone mode, we can wait for the GPU to finish before starting the next frame
		if (swapchain)
			frame_index = swapchain->GetCurrentBackBufferIndex();
	}
}