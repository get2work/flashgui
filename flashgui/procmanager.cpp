#include "pch.h"
#include "include/flashgui.h"

fgui::process_data::process_data(bool create_window, DWORD pid, HINSTANCE module_handle, HWND in_hwnd, RECT in_rect) {
	if (create_window) {
		WNDCLASSEXA wc = {};
		wc.cbSize = sizeof(WNDCLASSEXA);
		wc.lpfnWndProc = fgui::hk::window_procedure; // Set the window procedure
		wc.hInstance = h_instance ? h_instance : GetModuleHandle(nullptr); // Use provided instance handle or current module handle
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

		SetWindowLongPtr(window.handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)); // Set the user data to this instance

		LONG ex_style = GetWindowLong(window.handle, GWL_EXSTYLE);
		SetWindowLong(window.handle, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
		SetLayeredWindowAttributes(window.handle, 0, 255, LWA_ALPHA); // Optional: use LWA_COLORKEY for full transparency

		ShowWindow(window.handle, SW_SHOW); // Show the window
		UpdateWindow(window.handle); // Update the window to ensure it is drawn
	}
	else if (!in_hwnd) {
		EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
			process_data* data = reinterpret_cast<process_data*>(lParam);
			DWORD pid = 0;
			GetWindowThreadProcessId(hwnd, &pid);

			if (pid == data->dw_pid && !IsIconic(hwnd)) {
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