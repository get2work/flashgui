#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <d3dcommon.h>
#include <wrl/client.h>
#include "procmanager.h"

#include "fonts.h"
#include "shader_loader.hpp"
#include "root_sig_builder.hpp"
#include "pso_builder.hpp"
#include "frame_resource.hpp"

using Microsoft::WRL::ComPtr;

namespace fgui {

	struct shape_instance {
		DirectX::XMFLOAT2 pos; //start position, center position for circles
		DirectX::XMFLOAT2 size; // size (quad), radius (circle), endpos (line)
		float rotation; //rotation in radians (used for quads)
		float stroke_width; //line width for lines, >0 for outline on filled shapes
		DirectX::XMFLOAT4 clr; //rgba float colors (0f-1f)
		uint32_t shape_type; //0=quad, 1=quad outline, 2=circle, 3=circle outline, 4=line
		DirectX::XMFLOAT4  uv;          // u0,v0,u1,v1 for text/other textured

		bool visible = true; // Whether the shape should be rendered

		shape_instance(vec2i _pos, vec2i _size, DirectX::XMFLOAT4 _clr, float _rotation = 0.f, float _stroke_width = 0.f, uint32_t _shape_type = 0, DirectX::XMFLOAT4 _uv = { 0.f, 0.f, 1.f, 1.f })
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

	// Vertex structure: float2 position
	static const float quad_vertices[4][2] = {
		{ 0.0f, 0.0f }, // bottom-left
		{ 1.0f, 0.0f }, // bottom-right
		{ 1.0f, 1.0f }, // top-right
		{ 0.0f, 1.0f }  // top-left
	};

	static const uint16_t quad_indices[6] = {
		0, 1, 2, // first triangle
		0, 2, 3  // second triangle
	};

	struct s_dxgicontext {
		// Core DXGI components
		ComPtr<IDXGIFactory7> dxgi_factory;
		ComPtr<IDXGIAdapter1> adapter;
		ComPtr<ID3D12Device> device;
		ComPtr<ID3D12CommandQueue> cmd_queue;
		ComPtr<IDXGISwapChain3> swapchain;

		D3D12_VIEWPORT viewport = {}; // viewport for rendering
		D3D12_RECT scissor_rect = {}; // scissor rectangle for rendering

		uint32_t frame_index = 0; // current frame index

		// constant buffer for transformations, aligned to 256 bytes
		struct alignas(256) transform_cb {
			DirectX::XMMATRIX projection_matrix;
		} transform_cb{};

		std::vector<frame_resource> frame_resources; // frame resources for command allocators and command lists

		// pipeline + geometry components
		ComPtr<ID3D12RootSignature> root_sig; // root signature for the pipeline
		ComPtr<ID3D12PipelineState> pso_triangle; // pipeline state object for triangles

		std::unique_ptr<c_shader_loader> shaders; // shader loader

		ComPtr<ID3D12DescriptorHeap> rtv_heap; // render target view heap

		uint32_t rtv_descriptor_size = 0; // size of render target view descriptor

		std::vector<ComPtr<ID3D12Resource>> back_buffers; // Vector of backbuffers
		uint32_t buffer_count = 4; // default buffer count for swapchain


		DXGI_FORMAT dxgiformat = DXGI_FORMAT_R8G8B8A8_UNORM;
		UINT sample_count = 1; // desired multi-sampling level (e.g., 2, 4, 8)
		UINT num_quality_levels = 0;
		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1; // default feature level

		UINT sync_interval = 0; // v-sync off by default
		UINT swapchain_flags = 0; // no special flags by default

		std::unique_ptr<c_fonts> fonts;

		bool hooked = false;

		void create_device_and_swapchain();
		void create_rtv_heap();
		void wait_for_gpu();
		void release_resources();
		void create_resources();
		void begin_frame();
		void end_frame(std::vector<std::vector<shape_instance>>& shapes);
		void create_backbuffers();
		void resize_backbuffers(UINT width, UINT height, DXGI_FORMAT format) const;
		
		void create_pipeline();
		void initialize(IDXGISwapChain3* swapchain = nullptr, ID3D12CommandQueue* cmd_queue = nullptr, UINT sync_interval = 1, UINT flags = 0);
		void initialize_hooked();
		void initialize_standalone(const uint32_t& target_buf_count);

		frame_resource& get_current_frame_resource() {
			return frame_resources[frame_index];
		}

		ComPtr<ID3D12Resource> get_back_buffer(uint32_t index) const {
			if (index >= back_buffers.size()) return nullptr;
			return back_buffers[index].Get();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE get_rtv_handle(uint32_t index) const {
			if (!rtv_heap || index >= buffer_count)
				return {};

			D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
			handle.ptr += static_cast<SIZE_T>(index) * rtv_descriptor_size;

			return handle;
		}

		s_dxgicontext(D3D_FEATURE_LEVEL feature_lvl, UINT buffer_count) : buffer_count(buffer_count), feature_level(feature_lvl) {
			frame_resources.resize(buffer_count);
		}
	};
}