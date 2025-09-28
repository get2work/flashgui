#include "pch.h"

static void init_console() {
	// Try to attach to a parent console (e.g., if launched from cmd). If none, create a new one.
	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
		AllocConsole();
	}

	// Set UTF-8 so output looks right
	SetConsoleOutputCP(CP_UTF8);

	// Reopen C stdio to the console
	FILE* f_in = nullptr;
	FILE* f_out = nullptr;
	FILE* f_err = nullptr;
	freopen_s(&f_in, "CONIN$", "r", stdin);
	freopen_s(&f_out, "CONOUT$", "w", stdout);
	freopen_s(&f_err, "CONOUT$", "w", stderr);

	// Make C stdio unbuffered for immediate prints
	setvbuf(stdout, nullptr, _IONBF, 0);
	setvbuf(stderr, nullptr, _IONBF, 0);

	// Re-sync C++ iostreams with C stdio
	std::ios::sync_with_stdio(true);
	std::wcout.clear(); std::cout.clear(); std::wcerr.clear(); std::cerr.clear(); std::wcin.clear(); std::cin.clear();

	// Smoke test
	std::cout << "[console] Initialized" << std::endl;
}

int __stdcall WinMain(_In_ HINSTANCE h_instance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	init_console();
	fgui::initialize();

	MSG msg{}; bool running = true;
	while (running) {
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message == WM_QUIT) { running = false; break; }
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!running) break;

		fgui::render->begin_frame();
		//draw
		fgui::render->add_quad({ 0.f, 0.f }, { 100.f, 100.f }, { 1.f, 0.f, 1.f, 1.f });
		fgui::render->end_frame();
	}

	return 0;
}