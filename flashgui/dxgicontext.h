#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <wrl/client.h>

#include "srv_allocator.hpp"
#include "procmanager.h"

#include "shader_compiler.hpp"
#include "root_sig_builder.hpp"
#include "pso_builder.hpp"

using Microsoft::WRL::ComPtr;

namespace fgui {
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
		ComPtr<ID3D12PipelineState> pso_line; // Pipeline state object for lines	

		std::unique_ptr<c_srv_allocator> srv; // Shader resource view allocator
		std::unique_ptr<c_shader_builder> shaders; // Shader compiler and manager

		ComPtr<ID3D12DescriptorHeap> rtv_heap; // Render target view heap
		uint32_t rtv_descriptor_size = 0; // Size of render target view descriptor

		std::vector<ComPtr<ID3D12Resource>> back_buffers; // Vector of backbuffers
		uint32_t buffer_count = 3; // Default buffer count for swapchain

		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_1; // Default feature level

		void create_device_and_swapchain(const process_data& data);
		void create_rtv_heap();
		void create_srv_heap();
		void create_backbuffers();
		void resize_backbuffers(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
		
		void create_pipeline();
		void initialize_hooked();
		void initialize_standalone(const process_data& data, const uint32_t& target_buf_count);

		ComPtr<ID3D12Resource> get_back_buffer(uint32_t index) const {
			if (index >= back_buffers.size()) return nullptr;
			return back_buffers[index].Get();
		}

		D3D12_CPU_DESCRIPTOR_HANDLE get_rtv_handle(uint32_t index) const {
			if (!rtv_heap || index >= buffer_count)
				return {};

			D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
			handle.ptr += index * rtv_descriptor_size;

			return handle;
		}
	};
}