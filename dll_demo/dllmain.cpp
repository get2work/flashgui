#include "pch.h"
#include "hooks.h"
#include "menu.h"

static void init_console() {
    // ignore parent console, create new one
    AllocConsole();

    SetConsoleOutputCP(CP_UTF8);

    // reopen C stdio to the console
    FILE* f_in = nullptr;
    FILE* f_out = nullptr;
    FILE* f_err = nullptr;

    freopen_s(&f_in, "CONIN$", "r", stdin);
    freopen_s(&f_out, "CONOUT$", "w", stdout);
    freopen_s(&f_err, "CONOUT$", "w", stderr);

    // make C stdio unbuffered for immediate prints
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    // re-sync C++ iostreams with C stdio
    std::ios::sync_with_stdio(true);
    std::wcout.clear(); std::cout.clear(); std::wcerr.clear(); std::cerr.clear(); std::wcin.clear(); std::cin.clear();

    // replace with log manager
    std::cout << "[console] Initialized" << std::endl;
}

static void shutdown_console() {
    if (GetConsoleWindow()) {
        FreeConsole();
	}
}

static unsigned __stdcall entry_function(void* param) {
    DWORD pid = GetCurrentProcessId();
    HINSTANCE module_handle = (HINSTANCE)param;

    init_console();
    std::cout << "[flashgui] Analyzing process " << pid << std::endl;
    
    try {
		fgui::initialize(pid, module_handle);
    }
    catch (const std::runtime_error& e) {
		// catch any runtime errors and print them to the console
        std::cerr << "[flashgui] Error: " << e.what() << std::endl;
		std::cerr << "[flashgui] Failed to analyze process " << pid << std::endl;
        std::cerr << "[flashgui] Make sure you are running this on a process that uses DirectX 12 and has a valid swapchain." << std::endl;
        std::cout << "[flashgui] Exiting in 5 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        goto exit;
    }

    menu::setup_menu();

	std::cout << "[flashgui] Process " << pid << " analyzed successfully." << std::endl;
	std::cout << "[flashgui] Command queue offset: " <<    fgui::hk::hookinfo.command_queue_offset << std::endl;
	std::cout << "[flashgui] Present function address: " << fgui::hk::hookinfo.p_present << std::endl;
	std::cout << "[flashgui] ResizeBuffers function address: " << fgui::hk::hookinfo.p_resizebuffers << std::endl;

	MH_Initialize();
	MH_CreateHook(fgui::hk::hookinfo.p_present, &hooks::present, reinterpret_cast<LPVOID*>(&fgui::hk::o_present));
	MH_CreateHook(fgui::hk::hookinfo.p_resizebuffers, &hooks::resize_buffers, reinterpret_cast<LPVOID*>(&fgui::hk::o_resize_buffers));
	MH_EnableHook(MH_ALL_HOOKS);

    while (true) {
        // press END key to exit
        if (GetAsyncKeyState(VK_END) & 1) { 
            std::cout << "[flashgui] END key pressed, exiting..." << std::endl;
            break;
        }

        menu::update();

       // main loop for the DLL, can be used to update GUI or handle events
       // std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();

    shutdown_console();

	std::cout << "[flashgui] Exiting..." << std::endl;
    goto exit;

exit:
    shutdown_console();

	FreeLibrary(module_handle);
	_endthreadex(NULL);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE h_module,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(h_module);
        {
            uintptr_t h_thread = _beginthreadex(nullptr, 0, &entry_function, h_module, 0, nullptr);
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