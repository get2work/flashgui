# FlashGUI

Small DirectX12 overlay renderer + tiny GUI primitives.  
Includes:
- renderer core (DirectX12): quad instancing, SDF shapes, text rendering via DirectWrite glyph atlases
- standalone demo (`exe_demo`) and injector demo (`dll_demo`)
- precompiled shaders (`quad_vs.cso` / `quad_ps.cso`) embedded as byte arrays and loaded at runtime â no runtime shader compiler required
- font system powered by DirectWrite: system fonts are enumerated automatically and glyph atlases are built on demand

Prerequisites
- Windows 10/11 with Windows 10 SDK (D3D12/DXGI/DirectWrite headers & libs)
- Visual Studio 2022 (x64 workload)
- git
- vcpkg (recommended: manifest mode is supported via `vcpkg.json` in repo root)
- A modern GPU with D3D12 support

Quick build (recommended)
1. Clone repository:
   - git clone --recurse-submodules <repo_url>
2. Install dependencies via vcpkg (manifest present):
   - Open Developer PowerShell for VS 2022 and from repo root:
     - bootstrap vcpkg if you havenât already: `.\vcpkg\bootstrap-vcpkg.bat`
     - Install triplet (manifest will be used): `.\vcpkg\vcpkg install --triplet x64-windows`
   - Or configure vcpkg integration for Visual Studio:
     - `.\vcpkg\vcpkg integrate install`
3. Open the solution in Visual Studio 2022:
   - Set Solution Platform to `x64`
   - Build: use __Build Solution__
4. Run:
   - Standalone demo: set `exe_demo` as startup project and use __Start Debugging__ or run the produced EXE.
   - Injector demo: `dll_demo` builds a DLL that is suitable for manual injection into a D3D12 target process (use with caution).

Font system
- Fonts are obtained via DirectWrite (`IDWriteFactory` + `GetSystemFontCollection`), which enumerates all fonts installed on the system.
- Call `get_font_families()` on the renderer to retrieve a list of available family names.
- Request a font with `get_or_create_font(family, weight, style, size_px)`. A glyph atlas (D3D12 texture) is built on demand and cached for the lifetime of the renderer.
- No external font files or offline baking step is required.

Shader system
- Vertex and pixel shaders are precompiled to DXGI shader object (`.cso`) format and checked into the repository (`flashgui/shaders/quad_vs.cso`, `flashgui/shaders/quad_ps.cso`).
- `shader_loader.hpp` embeds the compiled bytecode as a `static const uint8_t[]` array and copies it into a `ID3DBlob` at initialisation â no DXC runtime dependency.
- To recompile shaders after editing the HLSL sources, compile `vertex.hlsl` / `pixel.hlsl` with DXC (or `fxc`) targeting `vs_6_0` / `ps_6_0` and replace the corresponding `.cso` files. Then regenerate the byte-array in `shader_loader.hpp`.

Usage notes
- Standalone flow (exe_demo):
  - The demo creates a small window and renders shapes + text using the renderer API.
  - Console output is enabled for quick diagnostics.
- Hooked flow (dll_demo):
  - The DLL finds swapchain/command queue offsets and hooks Present/ResizeBuffers.
  - Injection into other processes can trigger anti-cheat / security detections â only use on trusted targets.

Important implementation notes & troubleshooting
- Shaders:
  - Shader bytecode is loaded from the embedded byte arrays in `shader_loader.hpp`. There is no runtime DXC dependency; if you see `D3DCreateBlob` failures, ensure `d3dcompiler.lib` is linked (it is only used to create the blob wrapper).
  - If you see rendering artifacts after editing shaders, verify that the `.cso` files and the `shader_loader.hpp` byte arrays are in sync with the latest HLSL sources.
- Text rendering gotchas:
  - The vertex shader must pass per-vertex `quad_pos` in [0,1]^2 (unit quad coordinates) to the pixel shader.
  - The pixel shader reconstructs UV like: `uv = lerp(inst_uv.xy, inst_uv.zw, quad_pos)`.
  - Use a point sampler or inset UV by half-texel to avoid bilinear bleed between packed glyphs. The project uses a point/static sampler by default to avoid bleed.
  - If sampled glyph alpha is zero for all uv, verify that the atlas SRV was created and that the font atlas upload succeeded (check console logs).
- Descriptor heaps & frame resources:
  - The project uses a small SRV allocator (`srv_allocator.hpp`) with transient entries per-frame; ensure buffer_count is configured to match swapchain.
  - When resizing: frame resources must be signaled and waited for before resizing swapchain resources.

Debugging tips
- Enable console output (demo apps already do) and read errors printed to stderr/stdout.
- If text doesnât show:
  - Verify the font SRV heap is set on the command list (`ID3D12GraphicsCommandList::SetDescriptorHeaps`) and the correct root descriptor table is bound before sampling.
  - Check PSO sampler bindings and that the static sampler is registered at the same shader register (s0).
  - Confirm the requested font family name is available via `get_font_families()`.
- For device removed / presentation failures, check `GetDeviceRemovedReason()` and log the HRESULT.

Project layout (high level)
- `flashgui/` â renderer, precompiled shaders, helpers, and overlay UI
- `flashgui/shaders/` â HLSL sources (`vertex.hlsl`, `pixel.hlsl`) and precompiled bytecode (`quad_vs.cso`, `quad_ps.cso`)
- `exe_demo/` â standalone demo app
- `dll_demo/` â injector-style demo (DLL main + entry thread)
- `vcpkg.json` â dependency manifest for vcpkg

Contributing
- Feel free to open issues or PRs. Keep changes small and focused.
- If you modify shader HLSL sources, recompile to `.cso` and update the byte arrays in `shader_loader.hpp`.

License
- See repository root for LICENSE file. If none, ask the project owner before reuse.

Contact / help
- For quick help: build in Debug, run the demo, and paste console output/errors into an issue.
