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
			UINT width = 800;
			UINT height = 600;

			vec2i get_size() const {
				return vec2i(static_cast<int>(width), static_cast<int>(height));
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