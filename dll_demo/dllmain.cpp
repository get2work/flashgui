// dllmain.cpp : Defines the entry point for the DLL application.
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

static void shutdown_console() {
        // Close the console if it was created by this process
    if (GetConsoleWindow()) {
        FreeConsole();
	}
}

static unsigned __stdcall entry_function(void* param) {
    DWORD pid = GetCurrentProcessId();
    HINSTANCE module_handle = (HINSTANCE)param;

    init_console();
    std::cout << "[flashgui] Analyzing process " << pid << std::endl;

    fgui::hook_data hookinfo{};

    try {
       hookinfo = fgui::hk::get_info(pid, module_handle);
    }
    catch (const std::runtime_error& e) {
		//catch any runtime errors and print them to the console
        std::cerr << "[flashgui] Error: " << e.what() << std::endl;
		std::cerr << "[flashgui] Failed to analyze process " << pid << std::endl;
        std::cerr << "[flashgui] Make sure you are running this on a process that uses DirectX 12 and has a valid swapchain." << std::endl;
		system("pause");
        return 1; // Return error code
    }

	std::cout << "[flashgui] Process " << pid << " analyzed successfully." << std::endl;
	std::cout << "[flashgui] Command queue offset: " << hookinfo.command_queue_offset << std::endl;
	std::cout << "[flashgui] Present function address: " << hookinfo.p_present << std::endl;
	std::cout << "[flashgui] ResizeBuffers function address: " << hookinfo.p_resizebuffers << std::endl;

    while (true) {
        // Main loop for the DLL, can be used to update GUI or handle events
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to avoid busy-waiting
	}

	std::cout << "[flashgui] Exiting..." << std::endl;

    shutdown_console();

	FreeLibrary(module_handle); // Free the DLL module handle
	_endthreadex(NULL); // End the thread

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        {
            uintptr_t h_thread = _beginthreadex(nullptr, 0, &entry_function, hModule, 0, nullptr);
            if (h_thread) {
                CloseHandle((HANDLE)h_thread);
            }
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

