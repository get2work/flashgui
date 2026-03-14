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
			HWND in_hwnd = nullptr);

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

			void set_size(UINT new_width, UINT new_height) {
				width = new_width;
				height = new_height;
			}

		} window;

		// Input states
		struct input_state {
			vec2i mouse_pos;
			vec2i mouse_delta;
			bool mouse_down[5] = {};    // left, right, middle, backbutton, forwardbutton
			bool mouse_clicked[5] = {}; // true for one frame on press
			bool mouse_released[5] = {};// true for one frame on release
			int scroll_delta = 0;

			bool key_down[256] = {};
			bool key_pressed[256] = {};  // true for one frame
			bool key_released[256] = {}; // true for one frame

			// Text input buffer (characters typed this frame)
			std::wstring text_input;
		} input;

		// Call at the start of each frame to reset per-frame flags
		void begin_input_frame();

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