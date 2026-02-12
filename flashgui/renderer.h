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
#include "vec2.h"

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

		bool visible = true; // Whether the shape should be rendered

		shape_instance(vec2i _pos, vec2i _size, DirectX::XMFLOAT4 _clr, float _rotation = 0.f, float _stroke_width = 0.f, uint32_t _shape_type = 0, DirectX::XMFLOAT4 _uv = {0.f, 0.f, 1.f, 1.f})
			: pos(_pos),
			  size(_size),
			  rotation(_rotation),
			  stroke_width(_stroke_width),
			  clr(_clr),
			  shape_type(_shape_type),
			uv(_uv) {
		}
		shape_instance() : pos(0, 0), size(0, 0), rotation(0.f), stroke_width(0.f), clr(1.f, 1.f, 1.f, 1.f), shape_type(0), uv(0.f, 0.f, 1.f, 1.f) {
		}
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
		shape_instance* add_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width = 0.f, float rotation = 0.f);
		shape_instance* add_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width = 1.f, float rotation = 0.f);
		shape_instance* add_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width = 1.f);
		shape_instance* add_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 0.f);
		shape_instance* add_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 1.f);
		
		void remove_shape(shape_instance* instance);
		void draw_quad(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float outline_width = 0.f, float rotation = 0.f);
		void draw_quad_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float width = 1.f, float rotation = 0.f);
		void draw_line(vec2i start, vec2i end, DirectX::XMFLOAT4 clr, float width = 1.f);
		void draw_circle(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 0.f);
		void draw_circle_outline(vec2i pos, vec2i size, DirectX::XMFLOAT4 clr, float angle = 0.f, float outline_wdith = 1.f);

		void draw_text(const std::string& text, vec2i pos, float scale, DirectX::XMFLOAT4 clr);

		int get_fps() const;
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

		//immediate instances, cleared every frame, used for text and other shapes that need to be updated every frame
		std::vector<shape_instance> im_instances;

		uint32_t m_frame_index = 0; // Current frame index

		vec2i m_cursor_pos; // Current cursor position	

		uint32_t m_frame_count = 0; // Total frame count this second
		int m_fps = 0; // FPS value
		std::chrono::steady_clock::time_point m_last_fps_update; // Last time FPS was updated

		D3D12_VIEWPORT m_viewport = {}; // Viewport for rendering
		D3D12_RECT m_scissor_rect = {}; // Scissor rectangle for rendering

		// Constant buffer for transformations, aligned ttop o 256 bytes
		struct alignas(256) transform_cb {
			DirectX::XMMATRIX projection_matrix;
		} m_transform_cb{};

		std::vector<frame_resource> m_frame_resources; // Frame resources for command allocators and command lists
		s_dxgicontext m_dx; // DXGI context containing factory, adapter, device, command queue, and swapchain
		render_mode m_mode = render_mode::hooked; // Current rendering mode

	};
}
