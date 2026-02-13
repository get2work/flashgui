#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <wrl/client.h>

#include "srv_allocator.hpp"
#include "procmanager.h"

#include "shader_loader.hpp"
#include "root_sig_builder.hpp"
#include "pso_builder.hpp"

using Microsoft::WRL::ComPtr;

namespace fgui {

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

		// Pipeline + Geometry components
		ComPtr<ID3D12RootSignature> root_sig; // Root signature for the pipeline
		ComPtr<ID3D12PipelineState> pso_triangle; // Pipeline state object for triangles

		std::unique_ptr<c_srv_allocator> srv; // Shader resource view allocator
		std::unique_ptr<c_shader_loader> shaders; // Shader loader

		ComPtr<ID3D12DescriptorHeap> rtv_heap; // Render target view heap

		uint32_t rtv_descriptor_size = 0; // Size of render target view descriptor

		std::vector<ComPtr<ID3D12Resource>> back_buffers; // Vector of backbuffers
		uint32_t buffer_count = 4; // Default buffer count for swapchain

		ComPtr<ID3D12Resource> m_quad_vertex_buffer;
		ComPtr<ID3D12Resource> m_quad_index_buffer;
		D3D12_VERTEX_BUFFER_VIEW m_quad_vbv = {};
		D3D12_INDEX_BUFFER_VIEW  m_quad_ibv = {};

		DXGI_FORMAT dxgiformat = DXGI_FORMAT_R8G8B8A8_UNORM;
		UINT sample_count = 1; // Desired MSAA level (e.g., 2, 4, 8)
		UINT num_quality_levels = 0;
		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1; // Default feature level

		UINT sync_interval = 0; // V-sync off by default
		UINT swapchain_flags = 0; // No special flags by default

		bool hooked = false;

		void create_device_and_swapchain();
		void create_rtv_heap();
		void create_srv_heap();
		void release_resources();
		void create_resources();
		void create_backbuffers();
		void resize_backbuffers(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM) const;

		void create_quad_buffers();
		
		void create_pipeline();
		void initialize_hooked();
		void initialize_standalone(const uint32_t& target_buf_count);

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
	};
}