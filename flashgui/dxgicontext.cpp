#include "pch.h"
#include "dxgicontext.h"
#include "include/flashgui.h"

using namespace fgui;

//unhandled
void s_dxgicontext::initialize_hooked() {

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	HRESULT hr = swapchain->GetDesc1(&swapchain_desc);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to get swapchain description, HRESULT: " + std::to_string(hr));
	}

	hooked = true;

	// get buffer count from swapchain description
	buffer_count = swapchain_desc.BufferCount;

	hr = swapchain->GetDevice(IID_PPV_ARGS(&device));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to get device from swapchain, HRESULT: " + std::to_string(hr));
	}

	create_rtv_heap();

	create_resources();
	create_quad_buffers();
}

void s_dxgicontext::initialize_standalone(const uint32_t& target_buf_count) {
	buffer_count = target_buf_count;

	create_device_and_swapchain();
	create_rtv_heap();
	create_resources();
	create_quad_buffers();
}

void s_dxgicontext::create_device_and_swapchain() {
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
	swapchain_desc.Width = process->window.get_width(); // Set the width of the swapchain
	swapchain_desc.Height = process->window.get_height(); // Set the height of the swapchain
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
		throw std::runtime_error("Failed to create swapchain, HRESULT: " + std::to_string(hr));
	}

	hr = dxgi_factory->MakeWindowAssociation(process->window.handle, DXGI_MWA_NO_ALT_ENTER);
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
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.NumDescriptors = buffer_count;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create RTV heap, HRESULT: " + std::to_string(hr));
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
			throw std::runtime_error("Failed to get back buffer " + std::to_string(i) + ", HRESULT: " + std::to_string(hr));
		}
		device->CreateRenderTargetView(back_buffers[i].Get(), nullptr, rtv_handle);
		rtv_handle.ptr += rtv_descriptor_size; // Move to the next descriptor handle
	}
}

void s_dxgicontext::create_srv_heap() {

	//survives resizebuffers
	if (!srv) {
		srv = std::make_unique<c_srv_allocator>();

		// Initialize with a reasonable number of descriptors
		srv->initialize(device, 128, buffer_count);
	}

	for (auto& buffer : back_buffers) {
		if (buffer) {
			srv->allocate_backbuffer_srv(buffer, dxgiformat); // Allocate SRV for each backbuffer
		}
	}
}

void s_dxgicontext::release_resources() {
	// Release backbuffers
	for (auto& buffer : back_buffers) {
		srv->free_if_backbuffer(buffer.Get());
		buffer.Reset();
	}
	back_buffers.clear();
}

void s_dxgicontext::create_resources() {

	create_backbuffers();
	create_srv_heap();
}

void s_dxgicontext::resize_backbuffers(UINT width, UINT height, DXGI_FORMAT format) {
	if (!swapchain) {
		throw std::runtime_error("Swapchain is not initialized, cannot resize backbuffers");
	}

	try {
		//call swapchain resize buffers
		HRESULT hr = swapchain->ResizeBuffers(buffer_count, width, height, format, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to resize swapchain buffers, HRESULT: " + std::to_string(hr));
		}

	}
	catch (...) {
		std::throw_with_nested(std::runtime_error("Error during backbuffer resize"));
	}
}

void s_dxgicontext::create_quad_buffers() {
	// --- Vertex Buffer ---
	const UINT vb_size = sizeof(quad_vertices);

	D3D12_HEAP_PROPERTIES heap_props = {};
	heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vb_desc = {};
	vb_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vb_desc.Width = vb_size;
	vb_desc.Height = 1;
	vb_desc.DepthOrArraySize = 1;
	vb_desc.MipLevels = 1;
	vb_desc.SampleDesc.Count = 1;
	vb_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device->CreateCommittedResource(
		&heap_props,
		D3D12_HEAP_FLAG_NONE,
		&vb_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_quad_vertex_buffer)
	);

	if (FAILED(hr))
		throw std::runtime_error("Failed to create m_quad_vertex_buffer resource HRESULT: " + std::to_string(hr));

	// Upload vertex data
	void* vb_ptr = nullptr;
	m_quad_vertex_buffer->Map(0, nullptr, &vb_ptr);
	memcpy(vb_ptr, quad_vertices, vb_size);
	m_quad_vertex_buffer->Unmap(0, nullptr);

	m_quad_vbv.BufferLocation = m_quad_vertex_buffer->GetGPUVirtualAddress();
	m_quad_vbv.SizeInBytes = vb_size;
	m_quad_vbv.StrideInBytes = sizeof(float) * 2;

	// --- Index Buffer ---
	const UINT ib_size = sizeof(quad_indices);

	D3D12_RESOURCE_DESC ib_desc = vb_desc;
	ib_desc.Width = ib_size;

	hr = device->CreateCommittedResource(
		&heap_props,
		D3D12_HEAP_FLAG_NONE,
		&ib_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_quad_index_buffer)
	);

	if (FAILED(hr))
		throw std::runtime_error("Failed to create m_quad_index_buffer resource HRESULT: " + std::to_string(hr));

	// Upload index data
	void* ib_ptr = nullptr;
	m_quad_index_buffer->Map(0, nullptr, &ib_ptr);
	memcpy(ib_ptr, quad_indices, ib_size);
	m_quad_index_buffer->Unmap(0, nullptr);

	m_quad_ibv.BufferLocation = m_quad_index_buffer->GetGPUVirtualAddress();
	m_quad_ibv.SizeInBytes = ib_size;
	m_quad_ibv.Format = DXGI_FORMAT_R16_UINT;
}

void s_dxgicontext::create_pipeline() {
	if (!device)
		throw std::runtime_error("D3D12 device is not initialized, cannot create root signature and PSO");

	if (!shaders)
		shaders = std::make_unique<c_shader_builder>();

	shaders->initialize(device);

	// One SRV range: t0, space0, 1 descriptor
	D3D12_DESCRIPTOR_RANGE1 srv_range{};
	srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srv_range.NumDescriptors = 1;
	srv_range.BaseShaderRegister = 0; // t0
	srv_range.RegisterSpace = 0;
	srv_range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
	srv_range.OffsetInDescriptorsFromTableStart = 0;

	// One static sampler: s0
	D3D12_STATIC_SAMPLER_DESC samp{};
	samp.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // changed from MIN_MAG_MIP_LINEAR to POINT
	samp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samp.MaxLOD = D3D12_FLOAT32_MAX;
	samp.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samp.ShaderRegister = 0; // s0
	samp.RegisterSpace = 0;
	samp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	root_sig = c_rootsig_builder(device)
		.add_root_cbv(0, D3D12_SHADER_VISIBILITY_ALL)                  // b0
		.add_descriptor_table({ srv_range }, D3D12_SHADER_VISIBILITY_PIXEL) // param 1: SRV table (t0)
		.add_static_sampler(samp)
		.build();

	if (!root_sig) 
		throw std::runtime_error("Failed to create root signature");

	std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout = {
		// Slot 0: unit quad vertex
		{ "POSITION",   0, DXGI_FORMAT_R32G32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,   0 },

		// Slot 1: instance data (shape_instance)
		{ "TEXCOORD",   1, DXGI_FORMAT_R32G32_FLOAT,    1, 0,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // inst_pos
		{ "TEXCOORD",   2, DXGI_FORMAT_R32G32_FLOAT,    1, 8,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // inst_size
		{ "TEXCOORD",   3, DXGI_FORMAT_R32_FLOAT,       1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // inst_rot
		{ "TEXCOORD",   4, DXGI_FORMAT_R32_FLOAT,       1, 20, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // inst_stroke
		{ "TEXCOORD",   5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 24, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // inst_clr
		{ "TEXCOORD",   6, DXGI_FORMAT_R32_UINT,        1, 40, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // inst_type
		{ "TEXCOORD",   7, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, 44, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 } // inst_uv
	};

	pso_triangle = c_pso_builder(device)
		.set_root_signature(root_sig.Get())
		.set_vertex_shader(shaders->get_shader_blob(shader_type::vertex))
		.set_pixel_shader(shaders->get_shader_blob(shader_type::pixel))
		.set_input_layout(input_layout)
		.set_rtv_format(0, dxgiformat)
		.disable_depth()
		.enable_alpha_blending()
		.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.set_dsv_format(DXGI_FORMAT_UNKNOWN) // No depth-stencil view
		.set_num_render_targets(1) // One render target
		.set_sample_desc({ sample_count, num_quality_levels })
		.build();

	if (!pso_triangle)
		throw std::runtime_error("Failed to create PSO for triangles");
}