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

		// Skip console
		if (strcmp(cls, "ConsoleWindowClass") == 0) return TRUE;

		// Match your class OR title OR largest visible
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

		window.handle = CreateWindowExA(
			0, // No extended styles
			wc.lpszClassName, // Class name	
			"DX12 Window", // Window title
			WS_OVERLAPPEDWINDOW, // Window style
			CW_USEDEFAULT, CW_USEDEFAULT, window.width, window.height, // Default position and size
			nullptr, // No parent window
			nullptr, // No menu
			wc.hInstance, // Use provided instance handle or current module handle
			this // Pass this instance as user data
		);
			
		if (!window.handle) {
			throw std::runtime_error("Failed to create window");
		}

        // Do not force layered style on the main D3D window. Layered windows
        // can interact poorly with DXGI flip-model swapchains on Windows 10
        // and cause visual artifacts (black bars) after resize. Leave the
        // window as a normal top-level window so the swapchain covers the
        // client area correctly.

		ShowWindow(window.handle, SW_SHOW); // Show the window
		UpdateWindow(window.handle); // Update the window to ensure it is drawn
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