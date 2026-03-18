#include "pch.h"
#include "include/flashgui.h"

using namespace fgui;


struct enum_params {
	DWORD target_pid;
	HWND found_hwnd;
	char target_class[256];  // Optional class filter
};

static BOOL CALLBACK enum_callback(HWND hwnd, LPARAM lparam) {
	enum_params* params = reinterpret_cast<enum_params*>(lparam);
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);

	if (pid == params->target_pid) {
		
		char cls[256];
		GetClassNameA(hwnd, cls, 256);
		OutputDebugStringA(("DLL Enum: PID=" + std::to_string(pid) + " Class=" + cls + "\n").c_str());

		// skip console
		if (strcmp(cls, "ConsoleWindowClass") == 0) return TRUE;

		// match class OR title
		if (params->target_class[0] && strcmp(cls, params->target_class) == 0) {
			params->found_hwnd = hwnd;
			return FALSE;  // Stop
		}

		char title[256];
		GetWindowTextA(hwnd, title, 256);
		if (strlen(title) > 3) {  // Has title = main window
			params->found_hwnd = hwnd;
			return FALSE;
		}
	}
	return TRUE;
}

static HWND find_process_window(DWORD pid, const char* class_name = nullptr) {
	enum_params params = { pid, nullptr };
	if (class_name) strcpy_s(params.target_class, class_name);

	EnumWindows(enum_callback, reinterpret_cast<LPARAM>(&params));
	return params.found_hwnd;
}

c_process::c_process(bool create_window, DWORD pid, HINSTANCE module_handle, HWND in_hwnd) {
	if (create_window) {
		WNDCLASSEXA wc = {};
		wc.cbSize = sizeof(WNDCLASSEXA);
		wc.lpfnWndProc = hk::window_procedure; // Set the window procedure
		wc.hInstance = m_hinstance ? m_hinstance : GetModuleHandle(nullptr); // Use provided instance handle or current module handle
		wc.lpszClassName = "FlashGUIWindowClass"; // Set a class name for the window
		wc.style = CS_HREDRAW | CS_VREDRAW; // Redraw on resize
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW); // Set the default cursor
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); // Set a default icon
		wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION); // Set a small icon

		if (!RegisterClassExA(&wc)) {
			throw std::runtime_error("Failed to register window class");
		}

		// Adjust so the CLIENT area is exactly window.width x window.height
		RECT rc = { 0, 0, static_cast<LONG>(window.width), static_cast<LONG>(window.height) };
		AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);

		window.handle = CreateWindowExA(
			0, // No extended styles
			wc.lpszClassName, // Class name	
			"DX12 Window", // Window title
			WS_OVERLAPPEDWINDOW, // Window style
			CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,   // adjusted width (includes borders)
			rc.bottom - rc.top,   // adjusted height (includes title bar + borders)
			nullptr, // No parent window
			nullptr, // No menu
			wc.hInstance, // Use provided instance handle or current module handle
			this // Pass this instance as user data
		);
			
		if (!window.handle) {
			throw std::runtime_error("Failed to create window");
		}

		ShowWindow(window.handle, SW_SHOW); // show the window
		UpdateWindow(window.handle); // update the window to ensure it is drawn
	}
	else if (!in_hwnd) {
		window.handle = find_process_window(pid, "FlashGUIWindowClass");

		if (!window.handle) {
			printf("Failed to find window for PID %d\n", pid);
			throw std::runtime_error("Failed to find window for PID " + std::to_string(pid));
		}
		RECT client_rect;
		GetClientRect(window.handle, &client_rect);
		window.width = client_rect.right - client_rect.left;
		window.height = client_rect.bottom - client_rect.top;
	}
}

void c_process::end_input_frame() {
	for (int i = 0; i < 5; i++) {
		input.mouse_clicked[i] = false;
	}

	for (int i = 0; i < 256; i++) {
		input.key_pressed[i] = false;
		input.key_released[i] = false;
	}

	input.text_input.clear();
}

LRESULT c_process::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
	{
		m_minimized = (wparam == SIZE_MINIMIZED);
		if (m_minimized) return 0;

		int new_width = LOWORD(lparam);
		int new_height = HIWORD(lparam);

		if (new_width == 0 || new_height == 0) return 0;

		if (new_width != window.width || new_height != window.height) {
			window.width = new_width;
			window.height = new_height;
			m_needs_resize = true;
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		vec2i new_pos(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		input.mouse_delta = new_pos - input.mouse_pos;
		input.mouse_pos = new_pos;
		return 0;
	}
	case WM_LBUTTONDOWN:
		input.mouse_down[0] = true;
		input.mouse_down_time[0] = std::chrono::steady_clock::now();
		SetCapture(hwnd);
		return 0;
	case WM_LBUTTONUP:
		input.mouse_down[0] = false;

		if (std::chrono::steady_clock::now() - input.mouse_down_time[0] < input.long_press_threshold) {
			input.mouse_clicked[0] = true;
		}

		ReleaseCapture();
		return 0;
	case WM_RBUTTONDOWN:
		input.mouse_down[1] = true;
		input.mouse_down_time[1] = std::chrono::steady_clock::now();
		SetCapture(hwnd);
		return 0;
	case WM_RBUTTONUP:
		input.mouse_down[1] = false;

		if (std::chrono::steady_clock::now() - input.mouse_down_time[1] < input.long_press_threshold) {
			input.mouse_clicked[1] = true;
		}

		ReleaseCapture();
		return 0;
	case WM_MBUTTONDOWN:
		input.mouse_down[2] = true;
		input.mouse_down_time[2] = std::chrono::steady_clock::now();
		SetCapture(hwnd);
		return 0;
	case WM_MBUTTONUP:
		input.mouse_down[2] = false;
		if (std::chrono::steady_clock::now() - input.mouse_down_time[2] < input.long_press_threshold) {
			input.mouse_clicked[2] = true;
		}
		ReleaseCapture();
		return 0;
	case WM_XBUTTONDOWN: {
		int xbuttondown = GET_XBUTTON_WPARAM(wparam);
		if (xbuttondown == XBUTTON1) {
			input.mouse_down[3] = true;
			input.mouse_down_time[3] = std::chrono::steady_clock::now();
			SetCapture(hwnd);
		}
		else if (xbuttondown == XBUTTON2) {
			input.mouse_down[4] = true;
			input.mouse_down_time[4] = std::chrono::steady_clock::now();
			SetCapture(hwnd);
		}
		return 0;
	}
	case WM_XBUTTONUP: {
		int xbuttonup = GET_XBUTTON_WPARAM(wparam);
		if (xbuttonup == XBUTTON1) {
			input.mouse_down[3] = false;
			if (std::chrono::steady_clock::now() - input.mouse_down_time[3] < input.long_press_threshold) {
				input.mouse_clicked[3] = true;
			}
		}
		else if (xbuttonup == XBUTTON2) {
			input.mouse_down[4] = false;
			if (std::chrono::steady_clock::now() - input.mouse_down_time[4] < input.long_press_threshold) {
				input.mouse_clicked[4] = true;
			}
		}
		return 0;
	}
	case WM_MOUSEWHEEL:
		input.scroll_delta += GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		uint8_t vk = static_cast<uint8_t>(wparam);
		if (!input.key_down[vk])
			input.key_pressed[vk] = true;
		input.key_down[vk] = true;
		return 0;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		uint8_t vk = static_cast<uint8_t>(wparam);
		input.key_down[vk] = false;
		input.key_released[vk] = true;
		return 0;
	}
	case WM_CHAR:
		if (wparam >= 32) // printable
			input.text_input += static_cast<wchar_t>(wparam);
		return 0;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	default:
		return 0;
	}
}