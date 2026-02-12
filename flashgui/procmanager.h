#pragma once
#include <d3d12.h>
#include <utility>
#include <Windows.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <wrl/client.h>
#include "vec2.h"

namespace fgui {
	class c_process {
	public:
		c_process(bool create_window = true,
			DWORD pid = GetCurrentProcessId(),
			HINSTANCE module_handle = GetModuleHandleA(nullptr),
			HWND in_hwnd = nullptr, RECT in_rect = RECT{});

		bool needs_resize() const { return m_needs_resize; }
		void resize_complete() { m_needs_resize = false; }
		HINSTANCE get_instance() const { return m_hinstance; }
		DWORD get_pid() const { return m_pid; }

		struct window_info {
			HWND handle = nullptr;
			RECT rect = { 0, 0, 800, 600 }; // Default size

			long get_width() const {
				return rect.right - rect.left;
			}

			long get_height() const {
				return rect.bottom - rect.top;
			}

			vec2i get_size() const {
				return vec2i(static_cast<int>(get_width()), static_cast<int>(get_height()));
			}

			long get_posx() const {
				return rect.left;
			}
			long get_posy() const {
				return rect.top;
			}
		} window;

		LRESULT window_proc(HWND h_wnd, UINT msg, WPARAM wparam, LPARAM lparam);
	private:

		DWORD m_pid = 0;
		HINSTANCE m_hinstance = nullptr;
		bool m_needs_resize = false;

		bool m_minimized = false;
	};

	struct hook_data {
		void* p_present = nullptr;
		void* p_resizebuffers = nullptr;
		UINT command_queue_offset = 0;
	};
}