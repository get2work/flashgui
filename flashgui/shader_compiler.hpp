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
				// Projection matrix that maps world / screen positions into clip space
				float4x4 projection_matrix;
			};

			// Input from the IA / vertex buffer / instancing data
			struct VS_INPUT
			{
				float2 quad_pos   : POSITION;   // [0,0] to [1,1] local coordinate on the unit quad
				float2 inst_pos   : TEXCOORD1;  // start position of the shape in world / screen space
				float2 inst_size  : TEXCOORD2;  // full extents (width, height) for the instance
				float  inst_rot   : TEXCOORD3;  // rotation in radians; used for quads when inst_type >= 2
				float  inst_stroke: TEXCOORD4;  // stroke width for outlines and lines
				float4 inst_clr   : TEXCOORD5;  // RGBA color for the instance (fill/tint)
				uint   inst_type  : TEXCOORD6;  // shape type (0=box, 1=box outline, 2=circle, 3=circle outline, 4=line, 5=textured quad)
				float4 inst_uv    : TEXCOORD7; // texture UV rectangle (u0,v0,u1,v1) for textured quads
			};

			// Output sent to the rasterizer and pixel shader
			struct VS_OUTPUT
			{
				float4 sv_position : SV_POSITION; // clip space position to be interpolated
				float2 quad_pos    : TEXCOORD0;   // [0,0] to [1,1] across the quad (for SDF / UV in PS)
				float2 inst_pos    : TEXCOORD1;   // instance start position (screen origin)
				float2 inst_size   : TEXCOORD2;   // full extents of the instance (for SDF / size)
				float  inst_rot    : TEXCOORD3;   // radians of rotation, passed through
				float  inst_stroke : TEXCOORD4;   // stroke width, for outlines / lines
				float4 inst_clr    : TEXCOORD5;   // RGBA tint, passed to pixel shader
				uint   inst_type   : TEXCOORD6;   // shape type, used in pixel shader branches
				float4 inst_uv     : TEXCOORD7;   // UV rectangle for text / texture sampling
			};

			VS_OUTPUT VSMain(VS_INPUT input)
			{
				VS_OUTPUT output;

				// Convert [0,1]x[0,1] quad_pos to local pixel offset within the instance
				float2 local = input.quad_pos * input.inst_size;

				// Compute world / screen position of this corner
				float2 world_pos = input.inst_pos + local;

				// Optional in-place rotation about the instance center for shapes like quads
				// Only applied when explicitly requested and for types that should rotate (e.g., not raw lines)
				if (input.inst_rot != 0.0f && input.inst_type >= 2) // circles, boxes, etc.
				{
					// Center of the instance in screen space
					float2 center = input.inst_pos + 0.5f * input.inst_size;

					// Vector from center to current point
					float2 pos_rel = world_pos - center;

					// Compute sin / cos of the rotation
					float s = sin(input.inst_rot), c = cos(input.inst_rot);

					// Apply 2D rotation matrix to pos_rel
					pos_rel = float2(c * pos_rel.x - s * pos_rel.y,
									 s * pos_rel.x + c * pos_rel.y);

					// Transform back into world space with center restored
					world_pos = center + pos_rel;
				}

				// Promote to 4D homogeneous clip space position; z=0, w=1
				float4 pos = float4(world_pos, 0.0f, 1.0f);

				// Transform by projection to get final screen clip position
				output.sv_position = mul(projection_matrix, pos);

				// Pass all per-instance data to the pixel shader for SDF / text / etc.
				output.quad_pos    = input.quad_pos;    // [0,0] to [1,1]
				output.inst_pos    = input.inst_pos;    // instance origin in screen space
				output.inst_size   = input.inst_size;   // full width/height
				output.inst_rot    = input.inst_rot;    // radians
				output.inst_stroke = input.inst_stroke; // stroke / outline width
				output.inst_clr    = input.inst_clr;    // RGBA tint
				output.inst_type   = input.inst_type;   // shape type selector
				output.inst_uv     = input.inst_uv;     // UV rect for text / textures

				return output;
			}
		)";

		const std::string m_pixel_shader_str = R"(
			struct PS_INPUT
			{
				float4 sv_position : SV_POSITION; // clip-space pixel position from the vertex shader
				float2 quad_pos    : TEXCOORD0; // local coordinates of the pixel within the unit quad (0,0 to 1,1)
				float2 inst_pos    : TEXCOORD1; // start position of the shape instance
				float2 inst_size   : TEXCOORD2; // size of the shape instance (full extents for quads, radius for circles, endpos for lines)
				float  inst_rot    : TEXCOORD3; // rotation in radians for quads, ignored for circles/lines
				float  inst_stroke : TEXCOORD4; // stroke width for lines, >0 for outline on filled shapes
				float4 inst_clr    : TEXCOORD5; // RGBA color for the instance, used for both fill and stroke (alpha can be used to fade out)
				uint   inst_type   : TEXCOORD6; // shape type
				float4 inst_uv     : TEXCOORD7; // UV coordinates for text/texture rendering (u0,v0,u1,v1) (min_x, min_y, max_x, max_y)
			};

			// Texture and sampler for text rendering
			Texture2D font_tex : register(t0);
			SamplerState font_samp : register(s0);

			// Signed Distance Function for an axis-aligned box
			// Top left corner at the origin and size defined by 'size'. 
			float sd_box(float2 p, float2 size)
			{
				float2 d = min(p, size - p);
				return min(d.x, d.y); // positive inside, zero at edge, negative outside
			}
		
			// Signed Distance Function for a circle centered at the origin with given radius
			float sd_circle(float2 p, float radius)
			{
				return length(p) - radius;
			}

			// Signed Distance Function for a line segment from start to end, with thickness.
			// p is the pixel position in world space.
			float sd_line(float2 p, float2 start, float2 end, float thickness)
			{
				// vector from start to pixel, and from start to end
				float2 pa = p - start, ba = end - start; 
			
				// squared length of the line segment
				float ba_dot = dot(ba, ba); 

				// project pa onto ba, clamping to the line segment
				// This gives us the closest point on the line segment to p
				float h = ba_dot > 0.0 ? saturate(dot(pa, ba) / ba_dot) : 0.0;
			
				// Distance from p to the closest point on the line segment
				// -minus half the thickness = signed distance for a line with thickness.
				return length(pa - ba * h) - thickness * 0.5;
			}

			float4 PSMain(PS_INPUT input) : SV_TARGET
			{
				float4 out_color = input.inst_clr;
				// Local pixel position within the shape instance, in pixels. 
				// For quads, this goes from (0,0) to (size.x, size.y). 
				// For circles, this goes from (0,0) to (radius,radius).
				// For lines, this is the pixel position relative to the start point.
				float2 local = input.quad_pos * input.inst_size;

				// Textured quad
				if (input.inst_type == 5) 
				{

					// Sample the font texture using inst_uv and quad_pos to get the glyph alpha
					// inst_uv contains the UV rect for the glyph in the atlas: (u0,v0,u1,v1)
					// quad_pos goes from (0,0) to (1,1) across the quad, so we can lerp between u0 and u1, v0 and v1
					float2 uv = float2(lerp(input.inst_uv.x, input.inst_uv.z, input.quad_pos.x),
										 lerp(input.inst_uv.y, input.inst_uv.w, input.quad_pos.y) );
				
					// Sample the font texture to get the glyph alpha
					float4 texel = font_tex.Sample(font_samp, uv);

					// Multiply the sampled alpha by the instance color alpha to get final alpha
					float alpha = texel.a * input.inst_clr.a;
 
					// Tint the glyph using inst_clr.rgb
					float3 rgb = input.inst_clr.rgb * texel.a;
                
					// Output the final color with the glyph alpha applied
					out_color = float4(rgb, alpha);
				}
				else if (input.inst_type == 2) {
					// Circle/outline, centered
					// For circles, local is the pixel position relative to the top-left of the bounding box. We want it relative to the center for the SDF.
					float2 center = 0.5f * input.inst_size;
				
					// SDF for circle centered at origin, radius = center.x
					float2 p = local - center;
				
					// For filled circle, alpha is smoothstep based on distance to edge. For outline, we will do a band based on stroke width.
					float dist = sd_circle(p, center.x);

					// For filled circles, we want alpha to be 1 inside the circle and fade out at the edge. For outlines, we will handle that in the next case.
					float alpha = smoothstep(0.0, fwidth(dist), -dist);
					out_color.a = alpha;
				}
				else if (input.inst_type == 3) {
					//circle outline

					// we want to create a band for the outline based on the stroke width.
					float2 center = 0.5f * input.inst_size;
					float2 p = local - center; // pixel position relative to circle center
					float radius = center.x;
					float thickness = input.inst_stroke;

					// Calculate SDF for outer edge (radius) and inner edge (radius - thickness)
					float dist_outer = sd_circle(p, radius);
					float dist_inner = sd_circle(p, radius - thickness);
					float aa = fwidth(dist_outer);

					// Use smoothstep to create a band for the outline.
					// The alpha will be 1 between the inner and outer edges, and fade out outside of that.
					float alpha_outer = smoothstep(0.0, aa, -dist_outer);
					float alpha_inner = smoothstep(0.0, aa, -dist_inner);
					// final alpha is the difference between the outer and inner alpha, creates a ring for the outline.
					float alpha = alpha_outer - alpha_inner;
					out_color.a *= alpha;	
				}
				else if (input.inst_type == 0) {
					// Box/outline, not centered
					float2 p = local;
					// For filled box, alpha is smoothstep based on distance to edge
					// For outline, we will do a band based on stroke width in the next case. 
					float dist = sd_box(p, input.inst_size);
					float alpha = (input.inst_type == 0)
						? smoothstep(0.0, fwidth(dist), dist)
						: smoothstep(input.inst_stroke * 0.5, input.inst_stroke * 0.5 + fwidth(dist), abs(dist));
					out_color.a *= alpha;
				}
				else if (input.inst_type == 1) { 
					// Quad outline
					float2 p = local;
					// create a band for the outline based on the stroke width. 
					// The SDF gives us the distance to the edge, so we can use that to create a band.
			
					float2 half_size = input.inst_size * 0.5;
					// distance from the center, minus half the size gives us a sdf to the edge
					float2 d = abs(p - half_size) - half_size;
					// length of the positive part of d gives us the dist to the edge.
					float dist = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
					// create a band for the outline based on the stroke width
					float edge = input.inst_stroke * 0.5;
					// alpha is 1 in the band around the edge, and fades out outside of that. 
					// We abs(dist) to create a band on both sides of the edge for the outline.
					float alpha = smoothstep(edge, edge + fwidth(dist), abs(dist));
					alpha = 1.0 - alpha; // Only the shell is visible
					out_color.a *= alpha;
				}
				else if (input.inst_type == 4) {
					// Line: input.inst_pos to (size.x, size.y)
					// use line SDF to create a band based on stroke width.
					float dist = sd_line(input.sv_position.xy, input.inst_pos, input.inst_size, input.inst_stroke);
					float alpha = smoothstep(0.0, fwidth(dist), -dist);
					out_color.a *= alpha;
				}

				return out_color;
			}
		)";
	};
}