#pragma once
#include <d3d12.h>
#include <utility>
#include <Windows.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <wrl/client.h>

namespace fgui {
	struct process_data {
		DWORD dw_pid = 0;
		HINSTANCE h_instance = nullptr;

		struct window_info {
			HWND handle = nullptr;
			RECT rect = { 0, 0, 800, 600 }; // Default size

			long get_width() const {
				return rect.right - rect.left;
			}
			long get_height() const {
				return rect.bottom - rect.top;
			}
			long get_posx() const {
				return rect.left;
			}
			long get_posy() const {
				return rect.top;
			}
		} window;

		process_data(bool create_window = true,
			DWORD pid = GetCurrentProcessId(),
			HINSTANCE module_handle = GetModuleHandleA(nullptr),
			HWND in_hwnd = nullptr, RECT in_rect = RECT{});
	};

	struct hook_data {
		void* p_present = nullptr;
		void* p_resizebuffers = nullptr;
		UINT command_queue_offset = 0;
	};
}