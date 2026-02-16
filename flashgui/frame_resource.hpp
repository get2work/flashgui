#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

namespace fgui {
	struct frame_resource {
		ComPtr<ID3D12CommandAllocator> command_allocator;
		ComPtr<ID3D12GraphicsCommandList> command_list;
		ComPtr<ID3D12Fence> fence;
		UINT64 fence_value = 0;
		HANDLE fence_event = nullptr;

		// Dynamic upload heap for GUI or batching
		ComPtr<ID3D12Resource> upload_heap;
		UINT8* upload_ptr = nullptr;

		// In frame_resource
		size_t upload_size = 0;
		size_t upload_cursor = 0;

		D3D12_GPU_VIRTUAL_ADDRESS upload_gpu_base = 0;

		static inline size_t align_up(size_t v, size_t a) { return (v + (a - 1)) & ~(a - 1); }

		void reset_upload_cursor() { upload_cursor = 0; }

		// Push a constant buffer (returns GPU VA), 256B-aligned
		D3D12_GPU_VIRTUAL_ADDRESS push_cb(const void* data, size_t size_bytes) {

			const size_t off = align_up(upload_cursor, 256);

			if (off + size_bytes > upload_size)
				throw std::runtime_error("Upload heap out of space for CB");

			memcpy(upload_ptr + off, data, size_bytes);

			upload_cursor = off + align_up(size_bytes, 256);
			return upload_gpu_base + off;
		}

		D3D12_GPU_VIRTUAL_ADDRESS push_bytes(const void* data, size_t size_bytes, size_t alignment = 16) {
			const size_t off = align_up(upload_cursor, alignment);

			if (off + size_bytes > upload_size)
				throw std::runtime_error("Upload heap out of space");

			memcpy(upload_ptr + off, data, size_bytes);
			upload_cursor = off + size_bytes;

			return upload_gpu_base + off;
		}

		// Initialize the frame resource, creating command allocators, command lists, fences, and upload heaps.
		// The heap size is the size of the upload heap in bytes.
		void initialize(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type, size_t heap_size) {

			if (FAILED(device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator)))) {
				throw std::runtime_error("Failed to create command allocator");
			}

			if (FAILED(device->CreateCommandList(0, type, command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)))) {
				throw std::runtime_error("Failed to create command list");
			}

			command_list->Close(); // Command lists are created in the recording state. Close it for now.

			if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
				throw std::runtime_error("Failed to create fence");
			}

			create_upload_heap(device, heap_size);

			fence_value = 0;
			fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (!fence_event) {
				throw std::runtime_error("Failed to create fence event");
			}
		}

		void create_upload_heap(ComPtr<ID3D12Device> device, size_t size) {
			D3D12_HEAP_PROPERTIES heap_props = {};
			heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Width = size;
			desc.Height = 1;
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1;
			desc.SampleDesc.Count = 1;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			if (FAILED(device->CreateCommittedResource(
				&heap_props,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&upload_heap)))) {
				throw std::runtime_error("Failed to create upload heap");
			}

			upload_heap->Map(0, nullptr, reinterpret_cast<void**>(&upload_ptr));

			if (!upload_ptr) {
				throw std::runtime_error("Failed to map upload heap");
			}

			upload_size = size;
			upload_gpu_base = upload_heap->GetGPUVirtualAddress();
		}

		void release() {
			if (upload_heap && upload_ptr) {
				upload_heap->Unmap(0, nullptr);
				upload_heap.Reset();
				upload_ptr = nullptr;
			}

			command_allocator.Reset();
			command_list.Reset();
			
			upload_heap.Reset();
			upload_ptr = nullptr;

			fence.Reset();
			fence_value = 0;

			if (fence_event) {
				CloseHandle(fence_event);
				fence_event = nullptr;
			}
		}

		void reset(ComPtr<ID3D12PipelineState> pso) const {
			command_allocator->Reset();
			command_list->Reset(command_allocator.Get(), pso.Get());
		}

		void wait_for_gpu() const {
			if (fence && fence->GetCompletedValue() < fence_value) {
				fence->SetEventOnCompletion(fence_value, fence_event);
				WaitForSingleObject(fence_event, INFINITE);
			}
		}

		void signal(ComPtr<ID3D12CommandQueue> command_queue) {
			fence_value++;
			if (FAILED(command_queue->Signal(fence.Get(), fence_value))) {
				throw std::runtime_error("Failed to signal command queue");
			}
		}
	};
}