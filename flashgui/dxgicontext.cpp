#include "pch.h"
#include "dxgicontext.h"

using namespace fgui;

void s_dxgicontext::initialize_hooked() {

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	HRESULT hr = swapchain->GetDesc1(&swapchain_desc);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to get swapchain description, HRESULT: " + std::to_string(hr));
	}

	// get buffer count from swapchain description
	buffer_count = swapchain_desc.BufferCount;

	hr = swapchain->GetDevice(IID_PPV_ARGS(&device));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to get device from swapchain, HRESULT: " + std::to_string(hr));
	}

	create_rtv_heap();
	create_backbuffers();
	create_srv_heap();
}

void s_dxgicontext::initialize_standalone(const process_data& data, const uint32_t& target_buf_count) {
	buffer_count = target_buf_count;
	create_device_and_swapchain(data);
	create_rtv_heap();
	create_backbuffers();
	create_srv_heap();
}

void s_dxgicontext::create_device_and_swapchain(const process_data& data) {
	HRESULT hr = S_OK;

	// Create DXGI factory
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgi_factory));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create DXGI factory, HRESULT: " + std::to_string(hr));
	}

	// New DXGI factory feature for DirectX 12, cleaner than enuming all adapters
	hr = dxgi_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	if (FAILED(hr) || !adapter) {
		throw std::runtime_error("Failed to enumerate adapter, HRESULT: " + std::to_string(hr));
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
		throw std::runtime_error("Failed to create D3D12 device, HRESULT: " + std::to_string(hr));
	}

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // No special flags
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Direct command queue

	hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create command queue, HRESULT: " + std::to_string(hr));
	}

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	swapchain_desc.BufferCount = buffer_count; // Default buffer count
	swapchain_desc.Width = data.window.get_width(); // Set the width of the swapchain
	swapchain_desc.Height = data.window.get_height(); // Set the height of the swapchain
	swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Set the format of the swapchain
	swapchain_desc.Stereo = FALSE; // No stereo rendering
	swapchain_desc.SampleDesc.Count = 1; // No multisampling
	swapchain_desc.SampleDesc.Quality = 0; // No multisampling quality
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Use the swapchain for rendering
	swapchain_desc.Scaling = DXGI_SCALING_STRETCH; // Stretch the swapchain to fit the window
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Allow mode switching
	swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // No alpha mode
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Use flip discard for better performance

	// Create the swapchain for m_hwnd
	ComPtr<IDXGISwapChain1> swapchain1;
	hr = dxgi_factory->CreateSwapChainForHwnd(
		cmd_queue.Get(), // Command queue for the swapchain
		data.window.handle, // Handle to the overlay window
		&swapchain_desc, // Swapchain description
		nullptr, // No additional parameters
		nullptr, // No restrict to output
		&swapchain1 // Output swapchain pointer
	);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create swapchain, HRESULT: " + std::to_string(hr));
	}

	hr = dxgi_factory->MakeWindowAssociation(data.window.handle, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to make window association, HRESULT: " + std::to_string(hr));
	}

	hr = swapchain1.As(&swapchain);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to query IDXGISwapChain3 from IDXGISwapChain1, HRESULT: " + std::to_string(hr));
	}
}

void s_dxgicontext::create_rtv_heap() {
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // Render target view heap
	rtv_heap_desc.NumDescriptors = buffer_count; // Number of descriptors equal to buffer count
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // No special flags

	HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create RTV heap, HRESULT: " + std::to_string(hr));
	}

	// Get the size of each descriptor in the RTV heap
	rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void s_dxgicontext::create_backbuffers() {
	back_buffers.resize(buffer_count);
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < buffer_count; ++i) {
		HRESULT hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&back_buffers[i]));
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to get back buffer " + std::to_string(i) + ", HRESULT: " + std::to_string(hr));
		}
		device->CreateRenderTargetView(back_buffers[i].Get(), nullptr, rtv_handle);
		rtv_handle.ptr += rtv_descriptor_size; // Move to the next descriptor handle
	}
}

void s_dxgicontext::create_srv_heap() {

	if (!srv) {
		srv = std::make_unique<c_srv_allocator>();
	}

	//use srv allocator class
	srv->initialize(device, 128, buffer_count); // Initialize with a reasonable number of descriptors

	for (auto& buffer : back_buffers) {
		if (buffer) {
			srv->allocate_backbuffer_srv(buffer, DXGI_FORMAT_R8G8B8A8_UNORM); // Allocate SRV for each backbuffer
		}
	}
}

void s_dxgicontext::resize_backbuffers(UINT width, UINT height, DXGI_FORMAT format) {
	if (!swapchain) {
		throw std::runtime_error("Swapchain is not initialized, cannot resize backbuffers");
	}

	//release backbuffers, rtv_heap, and srv_heap
	for (auto& buffer : back_buffers) {
		srv->free_if_backbuffer(buffer.Get());
		buffer.Reset();
	}
	back_buffers.clear();
	rtv_heap.Reset();

	try {
		HRESULT hr = swapchain->ResizeBuffers(buffer_count, width, height, format, 0);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to resize swapchain buffers, HRESULT: " + std::to_string(hr));
		}
		create_rtv_heap(); // Recreate RTV heap after resizing
		create_backbuffers(); // Recreate backbuffers after resizing

		for (auto& buffer : back_buffers) {
			srv->allocate_backbuffer_srv(buffer, format);
		}
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("Error during backbuffer resize: ") + e.what());
	}
}

void s_dxgicontext::create_pipeline() {
	if (!device)
		throw std::runtime_error("D3D12 device is not initialized, cannot create root signature and PSO");

	if (!shaders)
		shaders = std::make_unique<c_shader_builder>();

	shaders->initialize(device);


	root_sig = c_rootsig_builder(device)
		.add_root_cbv(0, D3D12_SHADER_VISIBILITY_ALL) // Add a root constant buffer view
		.build();

	if (!root_sig) 
		throw std::runtime_error("Failed to create root signature");

	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D12_APPEND_ALIGNED_ELEMENT,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D12_APPEND_ALIGNED_ELEMENT,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pso_triangle = c_pso_builder(device)
		.set_root_signature(root_sig.Get())
		.set_vertex_shader(shaders->get_shader_blob(shader_type::vertex))
		.set_pixel_shader(shaders->get_shader_blob(shader_type::pixel_base))
		.set_input_layout(input_layout)
		.set_rtv_format(0, DXGI_FORMAT_R8G8B8A8_UNORM)
		.disable_depth()
		.enable_alpha_blending()
		.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.set_dsv_format(DXGI_FORMAT_UNKNOWN) // No depth-stencil view
		.set_num_render_targets(1) // One render target
		.set_sample_desc({ 1, 0 }) // No multisampling
		.build();

	if (!pso_triangle)
		throw std::runtime_error("Failed to create PSO for triangles");

	pso_line = c_pso_builder(device)
		.set_root_signature(root_sig.Get())
		.set_vertex_shader(shaders->get_shader_blob(shader_type::vertex))
		.set_pixel_shader(shaders->get_shader_blob(shader_type::pixel_base))
		.set_input_layout(input_layout)
		.set_rtv_format(0, DXGI_FORMAT_R8G8B8A8_UNORM)
		.set_dsv_format(DXGI_FORMAT_UNKNOWN) // No depth-stencil view
		.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
		.disable_depth()
		.enable_alpha_blending()
		.set_num_render_targets(1) // One render target
		.set_sample_desc({ 1, 0 }) // No multisampling
		.build();

	if (!pso_line)
		throw std::runtime_error("Failed to create PSO for lines");
}