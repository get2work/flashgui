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

	// test
	std::cout << "[console] Initialized" << std::endl;
}

int __stdcall WinMain(_In_ HINSTANCE h_instance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	init_console();
	fgui::initialize(4);

	//enumerate fonts

	auto families = fgui::render->get_font_families();

	std::wcout << L"System font families:\n";
	for (const auto& family : families) {
		std::wcout << L" - " << family << std::endl;
	}

	//initialize fonts
	auto verdanab24 = fgui::render->get_or_create_font(L"Verdana", DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, 30);
	printf("VerdanaBold24 font handle: %u\n", verdanab24);

	auto comicsans16 = fgui::render->get_or_create_font(L"Comic Sans MS", DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, 16);
	printf("ComicSansMS16 font handle: %u\n", comicsans16);

	auto impact32 = fgui::render->get_or_create_font(L"Vladimir Script", DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, 30);
	printf("Impact font handle: %u\n", impact32);
	
	MSG msg{}; bool running = true;

	// Create a background quad that fills the entire window, we will resize it every frame to match the window size
	// This is more efficient than creating a new quad every frame, and allows us to have a consistent background color

	while (running) {
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message == WM_QUIT) { running = false; break; }
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!running) break;

		fgui::render->begin_frame();

		fgui::render->draw_quad({ 0, 0 }, fgui::process->window.get_size(), { 0.05f, 0.05f, 0.1f, 1.f });

		// sample immediate mode circle outline, should be drawn on top of the background quad
		fgui::render->draw_circle_outline({ 140, 30 }, { 50, 50 }, { 1.f, 0.7f, 0.7f, 1.f }, 0.f, 5.f);
		fgui::render->draw_quad_outline({ 140, 90 }, { 50, 50 }, { 0.7f, 1.f, 0.7f, 1.f }, 3.f);

		// Draw some test text with the current FPS in cyan color
		fgui::render->draw_text("DX12 Test Window FPS: " + std::to_string(fgui::render->get_fps()), {200, 50}, verdanab24, {0.f, 1.f, 1.f, 1.f});
		fgui::render->draw_text("Comic Sans Text", {200, 80}, comicsans16, {1.f, 1.f, 0.f, 1.f});

		fgui::render->draw_text("Vladimir Script $", { 200, 140 }, impact32, { 1.f, 1.f, 1.f, 1.f });
		fgui::render->end_frame();
	}

	return 0;
}