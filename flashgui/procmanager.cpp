#include "pch.h"
#include "include/flashgui.h"

using namespace fgui;

c_process::c_process(bool create_window, DWORD pid, HINSTANCE module_handle, HWND in_hwnd, RECT in_rect) {
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

		window.handle = CreateWindowExA(
			0, // No extended styles
			wc.lpszClassName, // Class name
			"FlashGUI Window", // Window title
			WS_OVERLAPPEDWINDOW, // Window style
			CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // Default position and size
			nullptr, // No parent window
			nullptr, // No menu
			wc.hInstance, // Use provided instance handle or current module handle
			this // Pass this instance as user data
		);

		if (!window.handle) {
			throw std::runtime_error("Failed to create window");
		}

		GetClientRect(window.handle, &window.rect); // Get the client rectangle of the created window

		LONG ex_style = GetWindowLong(window.handle, GWL_EXSTYLE);
		SetWindowLong(window.handle, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
		SetLayeredWindowAttributes(window.handle, 0, 255, LWA_ALPHA); // Optional: use LWA_COLORKEY for full transparency

		ShowWindow(window.handle, SW_SHOW); // Show the window
		UpdateWindow(window.handle); // Update the window to ensure it is drawn
	}
	else if (!in_hwnd) {
		EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
			c_process* data = reinterpret_cast<c_process*>(lParam);
			DWORD pid = 0;
			GetWindowThreadProcessId(hwnd, &pid);

			if (pid == data->get_pid() && !IsIconic(hwnd)) {
				char class_name[256]{};
				GetClassNameA(hwnd, class_name, sizeof(class_name));

				if (strcmp(class_name, "ConsoleWindowwClass") != 0) {
					data->window.handle = hwnd;
					return FALSE;
				}
			}
			return TRUE; // Continue enumerating
			}, reinterpret_cast<LPARAM>(this));

		if (window.handle && in_rect.left == 0 && in_rect.top == 0 && in_rect.right == 0 && in_rect.bottom == 0) {
			GetWindowRect(window.handle, &window.rect);
		}
	}

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

		if (new_width != window.get_width() || new_height != window.get_height()) {
			window.rect.right = window.rect.left + new_width;
			window.rect.bottom = window.rect.top + new_height;
			m_needs_resize = true;
		}
		return 0;
	}
	case WM_KEYDOWN:
		// Handle key down events if needed
		return 0;
	case WM_KEYUP:
		// Handle key up events if needed
		return 0;
	case WM_CHAR:
		// Handle character input if needed
		return 0;
	case WM_SETFOCUS:
		// Handle focus events if needed
		return 0;
	case WM_KILLFOCUS:
		// Handle focus loss events if needed
		return 0;
	case WM_LBUTTONDOWN:
		// Handle left mouse button down events if needed
		return 0;
	case WM_RBUTTONDOWN:
		// Handle right mouse button down events if needed
		return 0;
	case WM_MOUSEMOVE:
		// Handle mouse move events if needed
		return 0;
	case WM_CLOSE:
		// Handle close events if needed
		return 0;

	default:
		return 0;
	}
}