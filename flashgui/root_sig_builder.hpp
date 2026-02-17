#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <directx/d3dx12.h>

using Microsoft::WRL::ComPtr;

namespace fgui {
    // Root signature builder that serializes to D3D_ROOT_SIGNATURE_VERSION_1 for maximum compatibility
    class c_rootsig_builder {
    public:
        explicit c_rootsig_builder(ComPtr<ID3D12Device> device) : m_device(device) {
            if (!m_device) {
                throw std::runtime_error("c_rootsig_builder: device is null");
            }
        }

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
            m_samplers.push_back(s);
            return *this;
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

        // Build a generic root signature from previously added parameters.
        // Serializes as D3D_ROOT_SIGNATURE_VERSION_1 for compatibility with hooked game devices.
        ComPtr<ID3D12RootSignature> build(const char* debugName = nullptr) {
            if (!m_device) {
                throw std::runtime_error("Invalid D3D12 device");
            }

            // Convert stored D3D12_ROOT_PARAMETER1 + D3D12_DESCRIPTOR_RANGE1 into D3D12_ROOT_PARAMETER + D3D12_DESCRIPTOR_RANGE
            std::vector<D3D12_ROOT_PARAMETER> out_params;
            std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> out_ranges_storage;
            out_params.reserve(m_params.size());
            out_ranges_storage.reserve(m_ranges_storage.size());

            size_t range_index = 0;
            for (const auto& p1 : m_params) {
                D3D12_ROOT_PARAMETER p0{};
                p0.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)p1.ParameterType;
                p0.ShaderVisibility = p1.ShaderVisibility;

                if (p1.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
                    // Map the next entry from m_ranges_storage into a D3D12_DESCRIPTOR_RANGE array
                    if (range_index < m_ranges_storage.size()) {
                        const auto& ranges1 = m_ranges_storage[range_index++];
                        out_ranges_storage.emplace_back();
                        auto& target_ranges = out_ranges_storage.back();
                        target_ranges.reserve(ranges1.size());
                        for (const auto& r1 : ranges1) {
                            D3D12_DESCRIPTOR_RANGE r0{};
                            r0.RangeType = (D3D12_DESCRIPTOR_RANGE_TYPE)r1.RangeType;
                            r0.NumDescriptors = r1.NumDescriptors;
                            r0.BaseShaderRegister = r1.BaseShaderRegister;
                            r0.RegisterSpace = r1.RegisterSpace;
                            // Ensure explicit 0 offset from table start (compat requirement)
                            r0.OffsetInDescriptorsFromTableStart = static_cast<UINT>(r1.OffsetInDescriptorsFromTableStart);
                            target_ranges.push_back(r0);
                        }
                        p0.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(target_ranges.size());
                        p0.DescriptorTable.pDescriptorRanges = target_ranges.data();
                    } else {
                        p0.DescriptorTable.NumDescriptorRanges = 0;
                        p0.DescriptorTable.pDescriptorRanges = nullptr;
                    }
                } else if (p1.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS) {
                    p0.Constants.ShaderRegister = p1.Constants.ShaderRegister;
                    p0.Constants.RegisterSpace = p1.Constants.RegisterSpace;
                    p0.Constants.Num32BitValues = p1.Constants.Num32BitValues;
                } else {
                    p0.Descriptor.ShaderRegister = p1.Descriptor.ShaderRegister;
                    p0.Descriptor.RegisterSpace = p1.Descriptor.RegisterSpace;
                }

                out_params.push_back(p0);
            }

            D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
            root_sig_desc.NumParameters = static_cast<UINT>(out_params.size());
            root_sig_desc.pParameters = out_params.data();
            root_sig_desc.NumStaticSamplers = static_cast<UINT>(m_samplers.size());
            root_sig_desc.pStaticSamplers = m_samplers.empty() ? nullptr : m_samplers.data();
            root_sig_desc.Flags = m_flags;

            ComPtr<ID3DBlob> sig_blob;
            ComPtr<ID3DBlob> error_blob;
            HRESULT hr = D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1, &sig_blob, &error_blob);
            if (FAILED(hr)) {
                if (error_blob) {
                    const char* msg = static_cast<const char*>(error_blob->GetBufferPointer());
                    OutputDebugStringA("[flashgui] D3D12SerializeRootSignature failed: ");
                    OutputDebugStringA(msg);
                    std::cerr << "[flashgui] D3D12SerializeRootSignature failed: " << msg << std::endl;
                } else {
                    OutputDebugStringA("[flashgui] D3D12SerializeRootSignature failed (no error blob)\n");
                    std::cerr << "[flashgui] D3D12SerializeRootSignature failed (no error blob)" << std::endl;
                }
                throw std::runtime_error("Failed to serialize root signature");
            }

            ComPtr<ID3D12RootSignature> root_signature;
            hr = m_device->CreateRootSignature(0, sig_blob->GetBufferPointer(), sig_blob->GetBufferSize(),
                IID_PPV_ARGS(&root_signature));
            if (FAILED(hr)) {
                std::string msg = "[flashgui] CreateRootSignature failed, HRESULT: " + std::to_string(hr) + "\n";
                OutputDebugStringA(msg.c_str());
                std::cerr << msg;
                throw std::runtime_error("Failed to create root signature");
            }

            if (debugName && root_signature) {
                root_signature->SetName(std::wstring(debugName, debugName + strlen(debugName)).c_str());
            }

            return root_signature;
        }
           
        
        // safe configuration
        ComPtr<ID3D12RootSignature> build_safe() {
            if (!m_device) {
                throw std::runtime_error("Invalid D3D12 device");
            }

            D3D12_DESCRIPTOR_RANGE srv_range{};
            srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            srv_range.NumDescriptors = 256;
            srv_range.BaseShaderRegister = 0; // t0
            srv_range.RegisterSpace = 0;
            srv_range.OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER params[2] = {};

            params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            params[0].Constants.ShaderRegister = 0; // b0
            params[0].Constants.RegisterSpace = 0;
            params[0].Constants.Num32BitValues = 16;
            params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[1].DescriptorTable.NumDescriptorRanges = 1;
            params[1].DescriptorTable.pDescriptorRanges = &srv_range;
            params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_STATIC_SAMPLER_DESC sampler{};
            sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            sampler.MaxLOD = D3D12_FLOAT32_MAX;
            sampler.ShaderRegister = 0; // s0
            sampler.RegisterSpace = 0;
            sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_ROOT_SIGNATURE_DESC desc{};
            desc.NumParameters = 2;
            desc.pParameters = params;
            desc.NumStaticSamplers = 1;
            desc.pStaticSamplers = &sampler;
            desc.Flags =
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

            ComPtr<ID3DBlob> sig;
            ComPtr<ID3DBlob> error;
            HRESULT hr = D3D12SerializeRootSignature(
                &desc,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &sig,
                &error
            );
            if (FAILED(hr)) {
                if (error) {
                    OutputDebugStringA((char*)error->GetBufferPointer());
                }
                throw std::runtime_error("D3D12SerializeRootSignature(build_safe) failed");
            }

            ComPtr<ID3D12RootSignature> root_signature;
            hr = m_device->CreateRootSignature(
                0,
                sig->GetBufferPointer(),
                sig->GetBufferSize(),
                IID_PPV_ARGS(&root_signature)
            );
            if (FAILED(hr)) {
                std::string msg = "[flashgui] CreateRootSignature(build_safe) failed, HRESULT: " + std::to_string(hr) + "\n";
                OutputDebugStringA(msg.c_str());
                throw std::runtime_error("CreateRootSignature(build_safe) failed");
            }

            return root_signature;
        }


    private:
        ComPtr<ID3D12Device> m_device;
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