#pragma once
#include <d3d12.h>
#include <utility>

#include "../renderer.h"

namespace fgui {

	/*
	* @param swapchain - pointer to IDXGISwapChain3, can be nullptr.
	* @param command_queue - pointer to ID3D12CommandQueue, can be nullptr.
	* @brief nullptr parameters used for standalone windowed mode, otherwise used to initialize the swapchain and command queue.
	* @return true if initialization is successful, false otherwise.
	*/
	bool initialize(IDXGISwapChain3* swapchain = nullptr, ID3D12CommandQueue* cmd_queue = nullptr);

	/*
	* @brief namespace for relevant function templates.
	*/
	namespace hk {
		using fn_present = HRESULT(WINAPI*)(IDXGISwapChain* p_this, UINT sync_interval, UINT flags);
		using fn_resize_buffers = HRESULT(WINAPI*)(IDXGISwapChain* p_this, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags);
		extern LRESULT window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		/*
		* @param data - process_data structure containing information about the process.
		* @brief Analyzes the process to find the swapchain and command queue. Wrap in try-catch to handle exceptions.
		* @throws std::runtime_error if the process cannot be analyzed or if the swapchain cannot be found.
		*/
		hook_data get_info(DWORD pid, HINSTANCE module_handle, HWND in_hwnd = nullptr, RECT in_rect = RECT{});
	
	}

	extern std::unique_ptr<c_renderer> render;
	extern uint32_t target_buffer_count; // Default buffer count for swapchain
	extern std::unique_ptr<c_process> process; // Information about the target process
}