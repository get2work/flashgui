#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>
#include <vector>
#include <Windows.h>
#include <string>
#include <stdexcept>
#include <wrl.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace fgui {

	struct srv_entry {
		ComPtr<ID3D12Resource> resource = nullptr; // Resource associated with the shader resource view
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{}; // Description of the shader resource view
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle{}; // GPU descriptor handle for the shader resource view
		uint32_t index = 0; // Index of the entry in the descriptor heap

		bool transient = false; // Indicates if the entry is transient (temporary) or persistent
	};

	class c_srv_allocator {
	public:
		void initialize(ComPtr<ID3D12Device> device, uint32_t num_descriptors, uint32_t buffer_count) {
			if (!device) {
				throw std::runtime_error("Device cannot be null");
			}

			m_transient_indices.resize(buffer_count); // Resize transient indices to accommodate the number of frames

			m_device = device;
			m_descriptor_max = num_descriptors;
			m_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
			heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Shader resource view heap
			heap_desc.NumDescriptors = m_descriptor_max; // Set the maximum number of descriptors
			heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Make it visible to shaders

			HRESULT hr = m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_heap));
			if (FAILED(hr)) {
				throw std::runtime_error("Failed to create descriptor heap, HRESULT: " + std::to_string(hr));
			}

			m_base_cpu = m_heap->GetCPUDescriptorHandleForHeapStart();
			m_base_gpu = m_heap->GetGPUDescriptorHandleForHeapStart();
		}

		void reset() {
			m_cursor = 0; // Reset the cursor to the start
			m_free_indices.clear(); // Clear the free indices
			m_transient_indices.clear(); // Clear transient indices
			m_entries.clear(); // Clear all entries
		}

		D3D12_GPU_DESCRIPTOR_HANDLE allocate(ComPtr<ID3D12Resource> resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, bool transient, uint32_t frame_index)
		{
			uint32_t index = 0;
			if (!m_free_indices.empty()) {
				index = m_free_indices.back(); // Reuse an existing free index
				m_free_indices.pop_back();
			}
			else {
				if (m_cursor >= m_descriptor_max)
					throw std::runtime_error("No more descriptors available in the heap");
				index = m_cursor++;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_base_cpu, index, m_descriptor_size);
			D3D12_GPU_DESCRIPTOR_HANDLE gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_base_gpu, index, m_descriptor_size);

			m_device->CreateShaderResourceView(resource.Get(), &desc, cpu); // Create the shader resource view

			srv_entry entry;
			entry.resource = resource; // Store the resource
			entry.srv_desc = desc; // Store the shader resource view description
			entry.transient = transient; // Set if the entry is transient
			entry.index = index; // Set the index of the entry
			entry.gpu_handle = gpu; // Set the GPU descriptor handle

			// Ensure the entries vector is large enough to hold the new entry
			if (index >= m_entries.size()) {
				m_entries.resize(size_t(index + 1)); // Resize the entries vector if necessary
			}

			m_entries[index] = entry; // Store the entry at the allocated index

			if (transient) {
				m_transient_indices[frame_index].push_back(index); // Store the index in the transient indices for the current frame
			}

			return gpu; // Return the GPU descriptor handle
		}

		void free_if_backbuffer(ComPtr<ID3D12Resource> resource) {
			for (auto& entry : m_entries) {
				if (entry.resource && entry.resource == resource && !entry.transient) {
					free(entry.index);
					break;
				}
			}
		}

		D3D12_GPU_DESCRIPTOR_HANDLE allocate_backbuffer_srv(ComPtr<ID3D12Resource> resource, DXGI_FORMAT format) {
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = format;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Texture2D.MipLevels = 1;

			return allocate(resource, desc, false, 0);
		}

		void free(uint32_t index) {
			if (index >= m_descriptor_max) {
				throw std::runtime_error("Index out of bounds for descriptor heap");
			}
			m_free_indices.push_back(index); // Add the index to the free indices
			m_entries[index] = {}; // Clear the entry at this index
		}

		void begin_frame(uint32_t frame_index) {
			if (frame_index >= m_transient_indices.size()) {
				throw std::runtime_error("Frame index out of bounds for transient indices");
			}

			for (uint32_t index : m_transient_indices[frame_index]) {
				m_free_indices.push_back(index); // Add the index back to free indices
			}
			m_transient_indices[frame_index].clear(); // Clear the transient indices for the current frame
		}

		std::vector<srv_entry> m_entries; // Vector to hold shader resource view entries

		uint32_t m_cursor = 0; // Current cursor position in the shader resource view entries

		uint32_t m_descriptor_size = 0; // Size of each descriptor in the heap
		uint32_t m_descriptor_count = 0; // Total number of descriptors allocated
		uint32_t m_descriptor_max = 0; // Maximum number of descriptors allowed

		ComPtr<ID3D12Device> m_device = nullptr; // Pointer to the D3D12 device
		ComPtr<ID3D12DescriptorHeap> m_heap = nullptr; // Shader resource view heap

		D3D12_CPU_DESCRIPTOR_HANDLE m_base_cpu{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_base_gpu{};

		std::vector<uint32_t> m_free_indices; // Vector to hold free indices for reuse
		std::vector<std::vector<uint32_t>> m_transient_indices; // Vector to hold transient indices for temporary resources
	};
}