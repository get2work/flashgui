#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <directx-dxc/dxcapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace fgui {

	struct shader_options {
		std::wstring hlsl_version = L"2021"; // -HV
		std::vector<std::wstring> include_dirs; // -I
		std::vector<std::pair<std::wstring, std::wstring>> defines; // -DNAME=VALUE
		bool debug = false; // Enable debug information
		bool optimize = true; // Enable optimizations
	};

	enum class shader_type {
		vertex,
		pixel,
	};

	class c_shader_builder
	{
	public:
		c_shader_builder() {
			if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)))) {
				throw std::runtime_error("Failed to create DXC Compiler instance");
			}
			if (FAILED(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library)))) {
				throw std::runtime_error("Failed to create DXC Library instance");
			}
			if (FAILED(m_library->CreateIncludeHandler(&m_include_handler))) {
				throw std::runtime_error("Failed to create include handler");
			}
			if (FAILED(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&m_refl)))) {
				throw std::runtime_error("Failed to create DXC Container Reflection instance");
			}
		}
		~c_shader_builder() { shutdown(); };

		void initialize(ComPtr<ID3D12Device> device) {

			if (!supports_shader_model(device)) {
				throw std::runtime_error("Shader model 6.0 is not supported by the device");
			}

			// Compile vertex shader from string
			m_vertex_shader_blob = compile(m_vertex_shader_str, L"VSMain", L"vs_6_0");
			if (!m_vertex_shader_blob) {
				throw std::runtime_error("Failed to compile vertex shader");
			}

			// Compile pixel shader from string
			m_pixel_shader_blob = compile(m_pixel_shader_str, L"PSMain", L"ps_6_0");
			if (!m_pixel_shader_blob) {
				throw std::runtime_error("Failed to compile pixel shader");
			}
			std::cout << "[Shader Compiler] All shaders compiled successfully." << std::endl;
		}

		void append_common_args(const shader_options& options, std::vector<LPCWSTR>& args) {
			args.push_back(L"-HV");
			args.push_back(options.hlsl_version.c_str());

			if (options.debug) {
				args.push_back(L"-Zi"); // Enable debug information
				args.push_back(L"-Qembed_debug"); // Embed debug information in the shader
			}

			if (!options.optimize) {
				args.push_back(L"-Od"); // Disable optimizations
			}
			else {
				args.push_back(L"-O3"); // Enable optimizations
			}

			for (const auto& dir : options.include_dirs) {
				args.push_back(L"-I");
				args.push_back(dir.c_str());
			}

			for (const auto& define : options.defines) {
				std::wstring def = L"-D" + define.first;
				if (!define.second.empty()) {
					def += L"=" + define.second;
				}
				args.push_back(def.c_str());
			}
		}

		bool supports_shader_model(ComPtr<ID3D12Device> device) const {
			if (!device) {
				return false;
			}

			D3D12_FEATURE_DATA_SHADER_MODEL shader_model = {};
			shader_model.HighestShaderModel = D3D_SHADER_MODEL_6_0; // Check for shader model 6.0

			HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(shader_model));

			if (SUCCEEDED(hr)) {
				return shader_model.HighestShaderModel >= D3D_SHADER_MODEL_6_0;
			}

			return false;
		}

		ComPtr<IDxcBlob> compile(
			const std::string& source,
			const std::wstring& entry_point,
			const std::wstring& target_profile,
			const std::vector<std::wstring>& arguments = {}
		) {
			// Create blob from source string
			ComPtr<IDxcBlobEncoding> source_blob;

			if (FAILED(m_library->CreateBlobWithEncodingFromPinned(
				reinterpret_cast<const BYTE*>(source.data()),
				static_cast<UINT32>(source.size()),
				CP_UTF8,
				&source_blob))) {
				throw std::runtime_error("Failed to create blob from source string");
			}

			// Prepare arguments for compilation
			shader_options options;
			options.hlsl_version = L"2021"; // Default HLSL version
#ifdef _DEBUG
			options.debug = true; // Enable debug information for development
#endif
			options.optimize = true; // Enable optimizations by default
			options.include_dirs = { L"." }; // Include current directory by default
			//options.defines = { {L"EXAMPLE_DEFINE", L"1"} }; // Example define

			std::vector<LPCWSTR> args;
			args.push_back(L"-E"); // Entry point
			args.push_back(entry_point.c_str());
			args.push_back(L"-T"); // Target profile
			args.push_back(target_profile.c_str());
			append_common_args(options, args);

			// Convert arguments to LPCWSTR array
			std::vector<LPCWSTR> args_w(args.size() + 1);
			for (size_t i = 0; i < args.size(); ++i) {
				args_w[i] = args[i];
			}
			args_w[args.size()] = nullptr; // Null-terminate the array

			// Compile the shader
			ComPtr<IDxcOperationResult> result;
			if (FAILED(m_compiler->Compile(
				source_blob.Get(), // Source code as IDxcBlobEncoding
				L"shader.hlsl", // virtual file name for error messages
				entry_point.c_str(),	 // Entry point
				target_profile.c_str(), // Target profile
				args_w.data(), // Arguments as LPCWSTR array
				static_cast<UINT32>(args_w.size() - 1), // Exclude the null terminator
				nullptr, // No additional libraries
				0,
				m_include_handler.Get(),
				&result))) {
				throw std::runtime_error("Failed to compile shader");
			}
			// Check for compilation errors
			HRESULT hr;

			ComPtr<IDxcBlobEncoding> m_error_blob;
			if (FAILED(result->GetStatus(&hr)) || FAILED(hr)) {
				if (FAILED(result->GetErrorBuffer(&m_error_blob))) {
					throw std::runtime_error("Failed to get error buffer");
				}
				std::string error_message(static_cast<const char*>(m_error_blob->GetBufferPointer()), m_error_blob->GetBufferSize());
				throw std::runtime_error("Shader compilation failed: " + error_message);
			}

			// Get the compiled shader blob
			ComPtr<IDxcBlob> shader_blob;
			if (FAILED(result->GetResult(&shader_blob))) {
				throw std::runtime_error("Failed to get compiled shader blob");
			}
			if (!shader_blob) {
				throw std::runtime_error("Compiled shader blob is null");
			}
			return shader_blob;
		}

		ComPtr<IDxcBlob> get_shader_blob(shader_type type) const {
			switch (type) {
			case shader_type::vertex:
				return m_vertex_shader_blob;
			case shader_type::pixel:
				return m_pixel_shader_blob;
			default:
				return nullptr;
			}
		}

		void shutdown() {
			m_compiler.Reset();
			m_library.Reset();
			m_include_handler.Reset();
			m_refl.Reset();
			m_vertex_shader_blob.Reset();
			m_pixel_shader_blob.Reset();
		}

	private:
		ComPtr<IDxcCompiler> m_compiler;
		ComPtr<IDxcLibrary> m_library;
		ComPtr<IDxcIncludeHandler> m_include_handler;
		ComPtr<IDxcContainerReflection> m_refl;

		ComPtr<IDxcBlob> m_vertex_shader_blob;
		ComPtr<IDxcBlob> m_pixel_shader_blob;

		const std::string m_vertex_shader_str = R"(
		cbuffer TransformCB : register(b0)
		{
		float4x4 projection_matrix;
		};

		struct VS_INPUT
		{
		float2 quad_pos   : POSITION;   // [0,0] to [1,1]
		float2 inst_pos   : TEXCOORD1;  // start pos
		float2 inst_size  : TEXCOORD2;  // size (full extents)
		float  inst_rot   : TEXCOORD3;  // radians
		float  inst_stroke: TEXCOORD4;  // stroke
		float4 inst_clr   : TEXCOORD5;  // RGBA
		uint   inst_type  : TEXCOORD6;  // shape_type
		};

		struct VS_OUTPUT
		{
		float4 sv_position : SV_POSITION;
		float2 quad_pos    : TEXCOORD0;
		float2 inst_pos    : TEXCOORD1;
		float2 inst_size   : TEXCOORD2;
		float  inst_rot    : TEXCOORD3;
		float  inst_stroke : TEXCOORD4;
		float4 inst_clr    : TEXCOORD5;
		uint   inst_type   : TEXCOORD6;
		};

		VS_OUTPUT VSMain(VS_INPUT input)
		{
		VS_OUTPUT output;

		float2 local = input.quad_pos * input.inst_size;
		float2 world_pos = input.inst_pos + local;

		// Rotation, always about center (for quad/circle): unless line/box wants origin.
		if (input.inst_rot != 0.0f && input.inst_type >= 2) // circles rotate about center
		{
		float2 center = input.inst_pos + 0.5f * input.inst_size;
		float2 pos_rel = world_pos - center;
		float s = sin(input.inst_rot), c = cos(input.inst_rot);
		pos_rel = float2(c * pos_rel.x - s * pos_rel.y, s * pos_rel.x + c * pos_rel.y);
		world_pos = center + pos_rel;
		}

		float4 pos = float4(world_pos, 0.0f, 1.0f);
		output.sv_position = mul(projection_matrix, pos);

		output.quad_pos    = input.quad_pos;
		output.inst_pos    = input.inst_pos;
		output.inst_size   = input.inst_size;
		output.inst_rot    = input.inst_rot;
		output.inst_stroke = input.inst_stroke;
		output.inst_clr    = input.inst_clr;
		output.inst_type   = input.inst_type;

		return output;
		}


		)";

		const std::string m_pixel_shader_str = R"(
		struct PS_INPUT
		{
			float4 sv_position : SV_POSITION;
			float2 quad_pos    : TEXCOORD0; // [0,0] to [1,1]
			float2 inst_pos    : TEXCOORD1;
			float2 inst_size   : TEXCOORD2;
			float  inst_rot    : TEXCOORD3;
			float  inst_stroke : TEXCOORD4;
			float4 inst_clr    : TEXCOORD5;
			uint   inst_type   : TEXCOORD6;
		};

		// SDF for box starting at origin [0,0], size is full extents
		float sdBox(float2 p, float2 size)
		{
			float2 d = min(p, size - p);
			return min(d.x, d.y); // positive inside, zero at edge
		}

		float sdCircle(float2 p, float radius)
		{
			return length(p) - radius;
		}

		float sdLine(float2 p, float2 a, float2 b, float thickness)
		{
			float2 pa = p - a, ba = b - a;
			float h = saturate(dot(pa, ba) / dot(ba, ba));
			return length(pa - ba * h) - thickness * 0.5;
		}

		float4 PSMain(PS_INPUT input) : SV_TARGET
		{
			float2 local = input.quad_pos * input.inst_size;
			float dist = 0.0;
			float alpha = 1.0;

			if (input.inst_type == 2 || input.inst_type == 3) {
				// Circle/outline, centered
				float2 center = 0.5f * input.inst_size;
				float2 p = local - center;
				if (input.inst_rot != 0.0f) {
					float s = sin(input.inst_rot), c = cos(input.inst_rot);
					p = float2(c * p.x - s * p.y, s * p.x + c * p.y);
				}
				dist = sdCircle(p, center.x);
				alpha = (input.inst_type == 2)
					? smoothstep(0.0, fwidth(dist), -dist)
					: smoothstep(input.inst_stroke * 0.5, input.inst_stroke * 0.5 + fwidth(dist), abs(dist));
			}
			else if (input.inst_type == 0 || input.inst_type == 1) {
				// Box/outline, not centered
				float2 p = local;
				dist = sdBox(p, input.inst_size);
				alpha = (input.inst_type == 0)
					? smoothstep(0.0, fwidth(dist), dist)
					: smoothstep(input.inst_stroke * 0.5, input.inst_stroke * 0.5 + fwidth(dist), abs(dist));
			}
			else if (input.inst_type == 4) {
				// Line: from (0,0) to (size.x, size.y)
				float2 p = local;
				dist = sdLine(p, float2(0.0, 0.0), input.inst_size, input.inst_stroke);
				alpha = smoothstep(0.0, fwidth(dist), -dist);
			}

			float4 outColor = input.inst_clr;
			outColor.a *= alpha;
			return outColor;
			}

		)";
	};
}