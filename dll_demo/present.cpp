#include "pch.h"
#include "hooks.h"
#include "flashgui.h"

HRESULT __fastcall hk::present(IDXGISwapChain3* p_swapchain, UINT sync_interval, UINT flags) {
	static bool first_present = true;
	// Call the original Present function
	static auto original_present = reinterpret_cast<fgui::hk::fn_present>(fgui::hk::get_info(fgui::process->get_pid(), fgui::process->get_instance()).p_present);

	ID3D12CommandQueue* d3d12_command_queue = (ID3D12CommandQueue*)(((void**)p_swapchain)[fgui::hk::hookinfo.command_queue_offset]);

	if (first_present) {
		first_present = false;
		fgui::render->initialize(p_swapchain, d3d12_command_queue, sync_interval, flags);
	}

	if (!fgui::render)

	fgui::render->begin_frame();

	fgui::render->draw_quad({ 100, 100 }, { 200, 200 }, { 1.f, 0.f, 0.f, 1.f }, 0.f, 0.f);

	fgui::render->end_frame();

	HRESULT hr = original_present(p_swapchain, sync_interval, flags);

	fgui::render->post_present(hr);

	return hr;
}

HRESULT __fastcall hk::resize_buffers(IDXGISwapChain3* p_this, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags) {
	// Call the original ResizeBuffers function
	static auto original_resize_buffers = reinterpret_cast<fgui::hk::fn_resize_buffers>(fgui::hk::get_info(fgui::process->get_pid(), fgui::process->get_instance()).p_resizebuffers);
	
	fgui::render->release_resources();
	
	HRESULT hr = original_resize_buffers(p_this, buffer_count, width, height, new_format, swapchain_flags);
	
	if (SUCCEEDED(hr)) {
		fgui::process->resize_complete();
		fgui::render->create_resources();
	}

	return hr;
}