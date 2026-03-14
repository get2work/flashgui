#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <d3dcompiler.h> // For D3DCreateBlob declaration

#include "shaders/quad_ps.h"
#include "shaders/quad_vs.h"

using Microsoft::WRL::ComPtr;

namespace fgui {
    class c_shader_loader {
    public:
        struct shader_bytecode {
            const uint8_t* data;
            size_t size;
        };

        c_shader_loader() = default;

        void initialize(ComPtr<ID3D12Device> device) {

            m_device = device;

            // load VS bytecode
            throw_if_failed(D3DCreateBlob(g_quad_vs_length, &m_vs_blob));
            memcpy(m_vs_blob->GetBufferPointer(), &g_quad_vs, g_quad_vs_length);

            // load PS bytecode
            throw_if_failed(D3DCreateBlob(g_quad_ps_length, &m_ps_blob));
            memcpy(m_ps_blob->GetBufferPointer(), &g_quad_ps, g_quad_ps_length);

            std::cout << "[Shader Loader] Bytecode loaded: VS=" << g_quad_vs_length
                << "B PS=" << g_quad_ps_length << "B\n";
        }

        D3D12_SHADER_BYTECODE get_vs() const {
            return { m_vs_blob->GetBufferPointer(), m_vs_blob->GetBufferSize() };
        }

        ComPtr<ID3DBlob> get_vs_blob() const {
            return m_vs_blob;
		}

        ComPtr<ID3DBlob> get_ps_blob() const {
            return m_ps_blob;
		}

        D3D12_SHADER_BYTECODE get_ps() const {
            return { m_ps_blob->GetBufferPointer(), m_ps_blob->GetBufferSize() };
        }

    private:

        inline void throw_if_failed(HRESULT hr) {
            if (FAILED(hr)) {
                // set breakpoint here
                char buf[128];
                sprintf_s(buf, "D3D12 Error 0x%08X", hr);
                MessageBoxA(nullptr, buf, "D3D12 Error", MB_OK | MB_ICONERROR);
                __debugbreak();  // breakpoint
            }
        }

        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3DBlob> m_vs_blob, m_ps_blob;
    };
}