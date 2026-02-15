#include "pch.h"
#include "dxgicontext.h"
#include "include/flashgui.h"

using namespace fgui;

void s_dxgicontext::initialize_hooked() {
	hooked = true;

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

void s_dxgicontext::release_backbuffers() {
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
			.enable_alpha_blending()
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