#include "pch.h"
#include "hooks.h"
#include "menu.h"

fgui::font_handle verdanab_24;

HRESULT __fastcall hooks::present(IDXGISwapChain3* p_swapchain, UINT sync_interval, UINT flags) {
	static bool first_present = true;

	ID3D12CommandQueue* d3d12_command_queue = (ID3D12CommandQueue*)(((void**)p_swapchain)[fgui::hk::hookinfo.command_queue_offset]);

	if (first_present) {
		first_present = false;
		fgui::render->initialize(p_swapchain, d3d12_command_queue, sync_interval, flags);
		verdanab_24 = fgui::render->get_or_create_font(L"Verdana", DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, 12);
	}

	fgui::render->begin_frame();

	menu::window.draw();

	fgui::render->end_frame();

	HRESULT hr = fgui::hk::o_present(p_swapchain, sync_interval, flags);

	fgui::render->post_present();

	return hr;
}

HRESULT __fastcall hooks::resize_buffers(IDXGISwapChain3* p_this, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags) {
	
	//fgui::render->wait_for_gpu();
	
	fgui::render->release_resources();

	fgui::process->window.set_size(width, height);

	HRESULT hr = fgui::hk::o_resize_buffers(p_this, buffer_count, width, height, new_format, swapchain_flags);

	if (SUCCEEDED(hr)) {
		fgui::process->resize_complete();
		fgui::render->create_resources();
	}

	return hr;
}