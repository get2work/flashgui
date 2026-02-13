#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

//#include "shader_loader.hpp"

using Microsoft::WRL::ComPtr;

namespace fgui {
	// Builder for creating D3D12 pipeline state objects
	// This class allows setting various pipeline states such as shaders, input layout, rasterizer state, etc.
	// It uses the builder pattern to allow chaining of method calls for configuration.
	class c_pso_builder
	{
	public:
		c_pso_builder(ComPtr<ID3D12Device> device)
			: m_device(device)
		{
			if (!m_device) {
				throw std::runtime_error("Invalid D3D12 device");
			}
			// Set default states
			m_rasterizer_desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			m_blend_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			m_depth_stencil_desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			m_depth_stencil_desc.DepthEnable = FALSE; // Disable depth by default
			m_primitive_topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			m_sample_desc.Count = 1;
			m_sample_desc.Quality = 0;
			m_sample_mask = UINT_MAX;
			m_num_render_targets = 1;
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
				m_rtv_formats[i] = DXGI_FORMAT_UNKNOWN;
			}
			m_rtv_formats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Default RTV format
			m_dsv_format = DXGI_FORMAT_UNKNOWN;
			m_input_elements = {};
			m_input_layout = {};
		}
		c_pso_builder& set_vertex_shader(const ComPtr<ID3DBlob>& shader_blob) {
			m_vertex_shader = shader_blob;
			return *this;
		}
		c_pso_builder& set_pixel_shader(const ComPtr<ID3DBlob>& shader_blob) {
			m_pixel_shader = shader_blob;
			return *this;
		}
		c_pso_builder& set_input_layout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& input_elements) {

			m_input_elements = input_elements;
			m_input_layout.pInputElementDescs = m_input_elements.data();
			m_input_layout.NumElements = static_cast<UINT>(m_input_elements.size());
			return *this;
		}
		c_pso_builder& set_rasterizer_state(const D3D12_RASTERIZER_DESC& rasterizer_desc) {
			m_rasterizer_desc = rasterizer_desc;
			return *this;
		}
		c_pso_builder& set_blend_state(const D3D12_BLEND_DESC& blend_desc) {
			m_blend_desc = blend_desc;
			return *this;
		}
		c_pso_builder& set_depth_stencil_state(const D3D12_DEPTH_STENCIL_DESC& depth_stencil_desc) {
			m_depth_stencil_desc = depth_stencil_desc;
			return *this;
		}
		c_pso_builder& set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type) {
			m_primitive_topology = topology_type;
			return *this;
		}

		c_pso_builder& set_sample_desc(const DXGI_SAMPLE_DESC& sample_desc) {
			m_sample_desc = sample_desc;
			return *this;
		}

		c_pso_builder& set_sample_mask(UINT sample_mask) {
			m_sample_mask = sample_mask;
			return *this;
		}

		c_pso_builder& set_num_render_targets(UINT num_render_targets) {
			m_num_render_targets = num_render_targets;
			return *this;
		}

		c_pso_builder& set_rtv_format(UINT index, DXGI_FORMAT format) {
			if (index < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT) {
				m_rtv_formats[index] = format;
			}
			return *this;
		}
		c_pso_builder& set_dsv_format(DXGI_FORMAT format) {
			m_dsv_format = format;
			return *this;
		}

		c_pso_builder& set_root_signature(const ComPtr<ID3D12RootSignature>& root_signature) {
			m_root_signature = root_signature;
			return *this;
		}

		c_pso_builder& disable_depth() {
			m_depth_stencil_desc.DepthEnable = FALSE;
			m_depth_stencil_desc.StencilEnable = FALSE;
			m_dsv_format = DXGI_FORMAT_UNKNOWN;
			return *this;
		}

		c_pso_builder& enable_depth(DXGI_FORMAT dsv_format) {
			m_depth_stencil_desc.DepthEnable = TRUE;
			m_depth_stencil_desc.StencilEnable = FALSE;
			m_dsv_format = dsv_format;
			return *this;
		}

		c_pso_builder& enable_alpha_blending(bool premultiplied = false) {
			m_blend_desc.RenderTarget[0].BlendEnable = TRUE;
			m_blend_desc.RenderTarget[0].SrcBlend = premultiplied ? D3D12_BLEND_ONE : D3D12_BLEND_SRC_ALPHA;
			m_blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			m_blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			m_blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			m_blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			m_blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			m_blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			return *this;
		}

		c_pso_builder& set_rtv_formats(const std::vector<DXGI_FORMAT>& formats) {
			m_num_render_targets = static_cast<UINT>(formats.size());
			for (UINT i = 0; i < m_num_render_targets && i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
				m_rtv_formats[i] = formats[i];
			}
			return *this;
		}

		c_pso_builder& set_debug_name(const std::string& name) {
			// Optional: Set a debug name for the pipeline state object
			m_debug_name = name; return *this;
		}

		c_pso_builder& allow_empty_input_layout(bool v = true) {
			m_allow_empty_input_layout = v; return *this;
		}

		c_pso_builder& set_cull_none() {
			m_rasterizer_desc.CullMode = D3D12_CULL_MODE_NONE; return *this;
		}

		c_pso_builder& set_wireframe(bool on = true) {
			m_rasterizer_desc.FillMode = on ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID; return *this;
		}

		c_pso_builder& set_node_mask(UINT mask) {
			m_node_mask = mask; return *this;
		}
		c_pso_builder& set_flags(D3D12_PIPELINE_STATE_FLAGS flags) {
			m_flags = flags; return *this;
		}

		c_pso_builder& set_srgb(bool enable) {
			for (UINT i = 0; i < m_num_render_targets; ++i) {
				if (m_rtv_formats[i] == DXGI_FORMAT_R8G8B8A8_UNORM || m_rtv_formats[i] == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) {
					m_rtv_formats[i] = enable ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
				}
			}
			return *this;
		}

		ComPtr<ID3D12PipelineState> build() {
			if (!m_device) {
				throw std::runtime_error("Invalid D3D12 device");
			}
			if (!m_vertex_shader || !m_pixel_shader) {
				throw std::runtime_error("Vertex and pixel shaders must be set");
			}

			if (!m_root_signature) {
				throw std::runtime_error("Root signature must be set");
			}

			//validate required fields
			if (m_num_render_targets == 0) {
				throw std::runtime_error("At least one render target must be set");
			}

			if (!m_allow_empty_input_layout && (m_input_layout.pInputElementDescs == nullptr || m_input_layout.NumElements == 0)) {
				throw std::runtime_error("Input layout must be set and cannot be empty");
			}

			D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
			pso_desc.pRootSignature = m_root_signature.Get();
			pso_desc.VS = { m_vertex_shader->GetBufferPointer(), m_vertex_shader->GetBufferSize() };
			pso_desc.PS = { m_pixel_shader->GetBufferPointer(), m_pixel_shader->GetBufferSize() };
			pso_desc.BlendState = m_blend_desc;
			pso_desc.RasterizerState = m_rasterizer_desc;
			pso_desc.DepthStencilState = m_depth_stencil_desc;
			pso_desc.InputLayout = m_input_layout;
			pso_desc.PrimitiveTopologyType = m_primitive_topology;
			pso_desc.SampleDesc = m_sample_desc;
			pso_desc.SampleMask = m_sample_mask;
			pso_desc.NumRenderTargets = m_num_render_targets;

			for (UINT i = 0; i < m_num_render_targets; ++i) {

				if (m_rtv_formats[i] == DXGI_FORMAT_UNKNOWN) {
					throw std::runtime_error("Render target format must be set for all render targets");
				}

				pso_desc.RTVFormats[i] = m_rtv_formats[i];
			}
			pso_desc.DSVFormat = m_dsv_format;
			pso_desc.NodeMask = m_node_mask;
			pso_desc.Flags = m_flags;

			ComPtr<ID3D12PipelineState> pipeline_state;
			if (FAILED(m_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)))) {
				throw std::runtime_error("Failed to create graphics pipeline state");
			}

			if (!m_debug_name.empty()) {
				pipeline_state->SetName(std::wstring(m_debug_name.begin(), m_debug_name.end()).c_str());
			}

			return pipeline_state;
		}

	private:
		ComPtr<ID3D12Device> m_device = nullptr;
		ComPtr<ID3DBlob> m_vertex_shader;
		ComPtr<ID3DBlob> m_pixel_shader;
		ComPtr<ID3D12RootSignature> m_root_signature;
		D3D12_RASTERIZER_DESC m_rasterizer_desc;
		D3D12_BLEND_DESC m_blend_desc;
		D3D12_DEPTH_STENCIL_DESC m_depth_stencil_desc;
		D3D12_INPUT_LAYOUT_DESC m_input_layout;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitive_topology;
		DXGI_SAMPLE_DESC m_sample_desc;
		UINT m_sample_mask;
		UINT m_num_render_targets;
		DXGI_FORMAT m_rtv_formats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
		DXGI_FORMAT m_dsv_format;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_elements;
		std::string m_debug_name;
		bool m_allow_empty_input_layout = false;
		UINT m_node_mask = 0;
		D3D12_PIPELINE_STATE_FLAGS m_flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	};
}