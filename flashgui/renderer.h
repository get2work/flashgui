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
#include <unordered_map>

#include "dxgicontext.h"
#include "procmanager.h"
#include "vec2.h"

#include "fonts.h"

namespace fgui {
	using Microsoft::WRL::ComPtr;

	enum class render_mode {
		external_window, // standalone windowed mode
		internal, // using swapchain and command queue
		external_overlay
	};

	class c_renderer {
	public:
		c_renderer(D3D_FEATURE_LEVEL feature_lvl, UINT buffer_count) :
			m_dx(std::make_unique<s_dxgicontext>(feature_lvl, buffer_count)) {}

		~c_renderer() = default;

		void initialize(IDXGISwapChain3* swapchain = nullptr, ID3D12CommandQueue* cmd_queue = nullptr, UINT sync_interval = 1, UINT flags = 0);

		void begin_frame();
		void end_frame();
		void post_present();

		void wait_for_gpu();

		void release_resources();
		void create_resources(bool create_heap_and_buffers = true);

		//draw functions
		//shape_instance* add_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width = 0.f, float rotation = 0.f);
		//shape_instance* add_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width = 1.f, float rotation = 0.f);
		//shape_instance* add_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width = 1.f);
		//shape_instance* add_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 0.f);
		//shape_instance* add_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 1.f);
		
		//void remove_shape(shape_instance* instance);
		void draw_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width = 0.f, float rotation = 0.f);
		void draw_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width = 1.f, float rotation = 0.f);
		void draw_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width = 1.f);
		void draw_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 0.f);
		void draw_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 1.f);

		void draw_text(const std::string& text, vec2i pos, font_handle font, DirectX::XMFLOAT4 clr);
		void draw_triangle(vec2i p1, vec2i p2, vec2i p3, DirectX::XMFLOAT4 clr);

		font_handle get_or_create_font(const std::wstring& family,
			DWRITE_FONT_WEIGHT weight,
			DWRITE_FONT_STYLE style,
			int size_px);

		std::vector<std::wstring> get_font_families() const;

		int get_fps() const;
	private:

		enum shape_type : int {
			quad = 0,
			quad_outline = 1,
			circle = 2,
			circle_outline = 3,
			line = 4,
			text_quad = 5
		};

		//immediate instances, cleared every frame, used for text and other shapes that need to be updated every frame
		//indexed by font handle
		std::vector<std::vector<shape_instance>> im_instances;

		vec2i m_cursor_pos; // Current cursor position	

		uint32_t m_frame_count = 0; // Total frame count this second
		int m_fps = 0; // FPS value
		std::chrono::steady_clock::time_point m_last_fps_update; // Last time FPS was updated

		std::unique_ptr<s_dxgicontext> m_dx; // DXGI context containing factory, adapter, device, command queue, and swapchain

		render_mode m_mode = render_mode::external_window; // Current rendering mode

	};
}
