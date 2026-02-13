#include "pch.h"
#include "include/flashgui.h"

using Microsoft::WRL::ComPtr;

namespace fgui {

	//global definitions
	std::unique_ptr<c_renderer> render = nullptr;
	uint32_t target_buffer_count = 4;
	std::unique_ptr<c_process> process;
	hook_data hk::hookinfo = {};

	hk::fn_present hk::o_present = nullptr;
	hk::fn_resize_buffers hk::o_resize_buffers = nullptr;

	//
	bool initialize(IDXGISwapChain3* swapchain, ID3D12CommandQueue* cmd_queue) {
		if (!render) {
			// Standalone mode, create our renderer instance
			process = std::make_unique<c_process>();

			render = std::make_unique<c_renderer>(D3D_FEATURE_LEVEL_12_1, target_buffer_count);

			if (!render) {
				std::cerr << "[flashgui] Failed to create renderer instance" << std::endl;
				return false; // Initialization failed
			}
		}

		render->initialize(swapchain, cmd_queue);

		SetWindowLongPtr(process->window.handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(render.get()));

		return true; // Initialization successful
	}

	// VMT indices for IDXGISwapChain3
	enum class idxgi_swapchain_vmt : UINT {
		present = 8,
		resize_buffers = 13
	};

	hook_data hk::get_info(DWORD pid, HINSTANCE module_handle, HWND in_hwnd, RECT in_rect) {
		process = std::make_unique<c_process>(false, pid, module_handle, in_hwnd, in_rect);
		SetWindowLongPtr(process->window.handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(process.get()));

		ComPtr<IDXGIFactory7> dxgi_factory;
		UINT dxgi_factory_flags = 0;

		HRESULT hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory));

		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create DXGI factory, HRESULT: " + std::to_string(hr));
		}

		ComPtr<IDXGIAdapter1> adapter;

		hr = dxgi_factory->EnumAdapterByGpuPreference(
			0, // Adapter index
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, // Prefer high-performance GPU
			IID_PPV_ARGS(&adapter)
		);

		if (FAILED(hr)) {
			throw std::runtime_error("Failed to enumerate adapter, HRESULT: " + std::to_string(hr));
		}

		ComPtr<ID3D12Device> device;

		const D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1; // Default feature level
		for (auto level : levels) {
			hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&device));
			if (SUCCEEDED(hr)) {
				feature_level = level;
				break;
			}
		}

		printf("[flashgui] Created D3D12 device with feature level: 0x%X\n", feature_level);

		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create D3D12 device, HRESULT: " + std::to_string(hr));
		}

		ComPtr<ID3D12CommandQueue> command_queue;

		// Create the command queue
		D3D12_COMMAND_QUEUE_DESC queue_desc = {};
		queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));

		/*
			Create dummy window
		*/
		WNDCLASSA wc = {};
		wc.lpfnWndProc = DefWindowProcA;
		wc.hInstance = process->get_instance();
		wc.lpszClassName = "XBox Game Bar";

		RegisterClassA(&wc);

		HWND hwnd_overlay = CreateWindowExA(
			WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
			wc.lpszClassName,
			"Overlay",
			WS_POPUP,
			process->window.get_posx(), process->window.get_posy(),
			process->window.get_width(), process->window.get_height(),
			nullptr, nullptr, process->get_instance(), nullptr
		);

		if (!hwnd_overlay) {
			throw std::runtime_error("Failed to create dummy window");
		}

		SetLayeredWindowAttributes(hwnd_overlay, RGB(0, 0, 0), 255, LWA_COLORKEY);
		SetWindowLongPtrA(hwnd_overlay, GWL_EXSTYLE, GetWindowLongPtrA(hwnd_overlay, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
		ShowWindow(hwnd_overlay, SW_SHOW);

		// Setup swapchain description
		DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};

		swapchain_desc.Width = process->window.get_width(); // Set the width of the swapchain
		swapchain_desc.Height = process->window.get_height(); // Set the height of the swapchain
		swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Set the format of the swapchain
		swapchain_desc.Stereo = FALSE; // No stereo rendering
		swapchain_desc.SampleDesc.Count = 1; // No multisampling
		swapchain_desc.SampleDesc.Quality = 0; // No multisampling quality

		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.BufferCount = 2; // Double buffering (doesnt matter for getting analysis)

		swapchain_desc.Scaling = DXGI_SCALING_STRETCH; // Stretch the swapchain to fit the window
		swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Allow mode switching
		swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // No alpha mode
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Use flip discard for better performance

		ComPtr<IDXGISwapChain1> swapchain1;

		// Create the swapchain
		hr = dxgi_factory->CreateSwapChainForHwnd(
			command_queue.Get(), // Command queue for the swapchain
			hwnd_overlay, // Handle to the overlay window
			&swapchain_desc, // Swapchain description
			nullptr, // No additional parameters
			nullptr, // No restrict to output
			&swapchain1 // Output swapchain pointer
		);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create swapchain1, HRESULT: " + std::to_string(hr));
		}

		ComPtr<IDXGISwapChain3> swapchain3;

		// Query the swapchain for IDXGISwapChain3
		hr = swapchain1.As(&swapchain3);
		if (FAILED(hr)) {
			throw std::runtime_error("Failed to query IDXGISwapChain3, HRESULT: " + std::to_string(hr));
		}

		//search for command queue offset in IDXGISwapChain3

		// search for the offset to the command queue from the swapchain
		// this offset is different between windows 10 and 11
		unsigned int current = 0;
		while (true)
		{
			if (current > 0x1000)
			{
				throw std::runtime_error("Failed to find command queue offset in IDXGISwapChain3");
				break;
			}

			ID3D12CommandQueue* cc = *(ID3D12CommandQueue**)(swapchain1.Get() + current);
			if (cc == command_queue.Get())
			{
				hookinfo.command_queue_offset = current;
				break;
			}

			current++;
		}

		// swapchain vtable, use index 8 for Present
		void** vmt_table_ptr = *(void***)swapchain3.Get();

		if (!vmt_table_ptr) {
			throw std::runtime_error("Failed to get swapchain vtable pointer");
		}

		hookinfo.p_present = (hk::fn_present)(vmt_table_ptr[(UINT)idxgi_swapchain_vmt::present]);
		hookinfo.p_resizebuffers = (hk::fn_resize_buffers)(vmt_table_ptr[(UINT)idxgi_swapchain_vmt::resize_buffers]);

		if (hwnd_overlay) {
			DestroyWindow(hwnd_overlay);
			UnregisterClassA("XBox Game Bar", process->get_instance());
			hwnd_overlay = nullptr;
		}

		if (!render) {
			render = std::make_unique<c_renderer>(feature_level, target_buffer_count);
		}
		
		return hookinfo; // Return the hook data containing the command queue offset and function pointers
		//ComPtrs are automatically released when they go out of scope, so no need to manually release them.
	}
} // namespace fgui