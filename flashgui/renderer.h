#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Windows.h>
#include <wrl.h>
#include <wrl/client.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <DirectXMath.h>

#include "dxgicontext.h"
#include "procmanager.h"
#include "frame_resource.hpp"

namespace fgui {
	using Microsoft::WRL::ComPtr;

	enum class render_mode {
		standalone, // standalone windowed mode
		hooked // using swapchain and command queue
	};

	class c_renderer {
	public:
		c_renderer(process_data procdata, D3D_FEATURE_LEVEL feature_lvl, UINT buffer_count)
			: m_dx(), m_proc(procdata) {
			m_dx.buffer_count = buffer_count;
			m_dx.feature_level = feature_lvl;
		}
		~c_renderer() = default;

		void initialize(IDXGISwapChain3* swapchain = nullptr, ID3D12CommandQueue* cmd_queue = nullptr);
		LRESULT window_proc(HWND h_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

		void resize_frame();
		void begin_frame();
		void end_frame();
	private:

		bool m_pending_resize = false; // Flag to indicate if a resize is pending
		uint32_t m_frame_index = 0; // Current frame index
		D3D12_VIEWPORT m_viewport = {}; // Viewport for rendering
		D3D12_RECT m_scissor_rect = {}; // Scissor rectangle for rendering

		// Constant buffer for transformations, aligned to 256 bytes
		struct alignas(256) transform_cb {
			DirectX::XMMATRIX projection_matrix;
		} m_transform_cb{};

		std::vector<frame_resource> m_frame_resources; // Frame resources for command allocators and command lists
		s_dxgicontext m_dx; // DXGI context containing factory, adapter, device, command queue, and swapchain
		render_mode m_mode = render_mode::hooked; // Current rendering mode
		process_data m_proc; // Information about the target process
	};
}
