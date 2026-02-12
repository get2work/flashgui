#pragma once

#include "pch.h"

namespace hk {
	extern HRESULT __fastcall present(IDXGISwapChain3* p_this, UINT sync_interval, UINT flags);
	extern HRESULT __fastcall resize_buffers(IDXGISwapChain3* p_this, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags);
}