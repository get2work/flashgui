#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <directx-dxc/dxcapi.h>
#include <iostream>
#include <stdexcept>
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

namespace fgui {
    class c_rootsig_builder {
    public:
        c_rootsig_builder(ComPtr<ID3D12Device> device) : m_device(device) {}

        c_rootsig_builder& add_descriptor_table(const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges, D3D12_SHADER_VISIBILITY vis) {
            m_ranges_storage.push_back(ranges);
            D3D12_ROOT_PARAMETER1 p{};
            p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            p.ShaderVisibility = vis;
            p.DescriptorTable.NumDescriptorRanges = (UINT)m_ranges_storage.back().size();
            p.DescriptorTable.pDescriptorRanges = m_ranges_storage.back().data();
            m_params.push_back(p);
            return *this;
        }

        c_rootsig_builder& add_root_srv(UINT reg, D3D12_SHADER_VISIBILITY vis) {
            D3D12_ROOT_PARAMETER1 p{};
            p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            p.ShaderVisibility = vis;
            p.Descriptor.ShaderRegister = reg;
            p.Descriptor.RegisterSpace = 0;
            p.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
            m_params.push_back(p);
            return *this;
        }

        c_rootsig_builder& add_root_uav(UINT reg, D3D12_SHADER_VISIBILITY vis) {
            D3D12_ROOT_PARAMETER1 p{};
            p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            p.ShaderVisibility = vis;
            p.Descriptor.ShaderRegister = reg;
            p.Descriptor.RegisterSpace = 0;
            p.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
            m_params.push_back(p);
            return *this;
        }

        c_rootsig_builder& add_root_cbv(UINT reg, D3D12_SHADER_VISIBILITY vis) {
            D3D12_ROOT_PARAMETER1 p{};
            p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            p.ShaderVisibility = vis;
            p.Descriptor.ShaderRegister = reg;
            p.Descriptor.RegisterSpace = 0;
            p.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
            m_params.push_back(p);
            return *this;
        }

        c_rootsig_builder& add_static_sampler(const D3D12_STATIC_SAMPLER_DESC& s) {
            m_samplers.push_back(s); return *this;
        }

        c_rootsig_builder& set_flags(D3D12_ROOT_SIGNATURE_FLAGS f) { m_flags = f; return *this; }

        c_rootsig_builder& add_constants(UINT reg, UINT num32BitValues, D3D12_SHADER_VISIBILITY vis) {
            D3D12_ROOT_PARAMETER1 p{};
            p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            p.ShaderVisibility = vis;
            p.Constants.ShaderRegister = reg;
            p.Constants.RegisterSpace = 0;
            p.Constants.Num32BitValues = num32BitValues;
            m_params.push_back(p);
            return *this;
        }

        ComPtr<ID3D12RootSignature> build(const char* debugName = nullptr) {
            if (!m_device) {
                throw std::runtime_error("Invalid D3D12 device");
            }
            D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_sig_desc = {};
            root_sig_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            root_sig_desc.Desc_1_1.NumParameters = (UINT)m_params.size();
            root_sig_desc.Desc_1_1.pParameters = m_params.data();
            root_sig_desc.Desc_1_1.NumStaticSamplers = (UINT)m_samplers.size();
            root_sig_desc.Desc_1_1.pStaticSamplers = m_samplers.data();
            root_sig_desc.Desc_1_1.Flags = m_flags;
            ComPtr<ID3DBlob> sig_blob;
            ComPtr<ID3DBlob> error_blob;

            HRESULT hr = D3DX12SerializeVersionedRootSignature(
                &root_sig_desc,
                D3D_ROOT_SIGNATURE_VERSION_1_1,
                &sig_blob,
                &error_blob
            );

            if (FAILED(hr)) {
                if (error_blob) {
                    std::cerr << "[astral-render] Root Signature Serialization Error: "
                        << static_cast<const char*>(error_blob->GetBufferPointer()) << std::endl;
                }
                throw std::runtime_error("Failed to serialize root signature");
            }
            ComPtr<ID3D12RootSignature> root_signature;
            hr = m_device->CreateRootSignature(0, sig_blob->GetBufferPointer(), sig_blob->GetBufferSize(),
                IID_PPV_ARGS(&root_signature));
            if (FAILED(hr)) {
                throw std::runtime_error("Failed to create root signature");
            }

            // Set debug name if provided
            if (debugName) {
                root_signature->SetName(std::wstring(debugName, debugName + strlen(debugName)).c_str());
            }
            return root_signature;
        }
    private:
        ComPtr<ID3D12Device> m_device{};
        std::vector<D3D12_ROOT_PARAMETER1> m_params;
        std::vector<std::vector<D3D12_DESCRIPTOR_RANGE1>> m_ranges_storage;
        std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplers;
        D3D12_ROOT_SIGNATURE_FLAGS m_flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    };
}