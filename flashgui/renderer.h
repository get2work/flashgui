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
#include "frame_resource.hpp"

#include "fonts.h"

namespace fgui {
	using Microsoft::WRL::ComPtr;

	enum class render_mode {
		standalone, // standalone windowed mode
		hooked // using swapchain and command queue
	};

	struct shape_instance {
		DirectX::XMFLOAT2 pos; //start position, center position for circles
		DirectX::XMFLOAT2 size; // size (quad), radius (circle), endpos (line)
		float rotation; //rotation in radians (used for quads)
		float stroke_width; //line width for lines, >0 for outline on filled shapes
		DirectX::XMFLOAT4 clr; //rgba float colors (0f-1f)
		uint32_t shape_type; //0=quad, 1=quad outline, 2=circle, 3=circle outline, 4=line
		DirectX::XMFLOAT4  uv;          // u0,v0,u1,v1 for text/other textured
	};

	struct font_glyph_info {
		float u0, v0, u1, v1; // UV rect in atlas
		float advance;        // in pixels
	};

	class c_renderer {
	public:
		c_renderer(D3D_FEATURE_LEVEL feature_lvl, UINT buffer_count) : m_dx() {
			m_dx.buffer_count = buffer_count;
			m_dx.feature_level = feature_lvl;
		}
		~c_renderer() = default;

		void initialize(IDXGISwapChain3* swapchain = nullptr, ID3D12CommandQueue* cmd_queue = nullptr);

		void resize_frame();
		void begin_frame();
		void end_frame();

		//draw functions
		void add_quad(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float outline_width = 0.f, float rotation = 0.f);
		void add_quad_outline(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float width = 1.f, float rotation = 0.f);
		void add_line(DirectX::XMFLOAT2 start, DirectX::XMFLOAT2 end, DirectX::XMFLOAT4 clr, float width = 1.f);
		void add_circle(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 0.f);
		void add_circle_outline(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 1.f);
		void draw_text(const std::string& text, DirectX::XMFLOAT2 pos, float scale, DirectX::XMFLOAT4 clr);

		int get_fps();
	private:

		void initialize_fonts();

		std::unordered_map<uint32_t, font_glyph_info> m_font_glyphs;
		Microsoft::WRL::ComPtr<ID3D12Resource>        m_font_texture;
		D3D12_GPU_DESCRIPTOR_HANDLE                   m_font_srv{};

		enum shape_type : int {
			quad = 0,
			quad_outline = 1,
			circle = 2,
			circle_outline = 3,
			line = 4,
			text_quad = 5
		};

		//shape instances for quad
		std::vector<shape_instance> instances;

		//circle instances
		std::vector<shape_instance> instances_circle;

		uint32_t m_frame_index = 0; // Current frame index

		uint32_t m_frame_count = 0; // Total frame count this second
		int m_fps = 0; // FPS value

		D3D12_VIEWPORT m_viewport = {}; // Viewport for rendering
		D3D12_RECT m_scissor_rect = {}; // Scissor rectangle for rendering

		// Constant buffer for transformations, aligned to 256 bytes
		struct alignas(256) transform_cb {
			DirectX::XMMATRIX projection_matrix;
		} m_transform_cb{};

		std::vector<frame_resource> m_frame_resources; // Frame resources for command allocators and command lists
		s_dxgicontext m_dx; // DXGI context containing factory, adapter, device, command queue, and swapchain
		render_mode m_mode = render_mode::hooked; // Current rendering mode

	};
}
