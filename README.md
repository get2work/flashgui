# FlashGUI

Small DirectX12 overlay renderer + tiny GUI primitives.  
Includes:
- renderer core (DirectX12): quad instancing, SDF shapes, text rendering via bitmap atlas
- standalone demo (`exe_demo`) and injector demo (`dll_demo`)
- font atlas generator (`font_converter`) using stb_truetype (produces embedded atlas + glyph table)
- shader compiler using DXC (DirectX Shader Compiler) and a minimal SRV allocator

Prerequisites
- Windows 10/11 with Windows 10 SDK (D3D12/DXGI headers & libs)
- Visual Studio 2022 (x64 workload)
- git
- vcpkg (recommended: manifest mode is supported via `vcpkg.json` in repo root)
- A modern GPU with D3D12 support

Quick build (recommended)
1. Clone repository:
   - git clone --recurse-submodules <repo_url>
2. Install dependencies via vcpkg (manifest present):
   - Open Developer PowerShell for VS 2022 and from repo root:
     - bootstrap vcpkg if you haven’t already: `.\vcpkg\bootstrap-vcpkg.bat`
     - Install triplet (manifest will be used): `.\vcpkg\vcpkg install --triplet x64-windows`
   - Or configure vcpkg integration for Visual Studio:
     - `.\vcpkg\vcpkg integrate install`
3. Open the solution in Visual Studio 2022:
   - Set Solution Platform to `x64`
   - Build: use __Build Solution__
4. Run:
   - Standalone demo: set `exe_demo` as startup project and use __Start Debugging__ or run the produced EXE.
   - Injector demo: `dll_demo` builds a DLL that is suitable for manual injection into a D3D12 target process (use with caution).

Font atlas / embedded font generation
- The project includes a `font_converter` tool to bake TTF -> atlas + glyph table (embedded C++).
- Example:
  - Build `font_converter` (x64).
  - Run: `font_converter\font_converter.exe DejaVuSans.ttf DejaVuSans-Bold.ttf output_font_data.cpp`
  - Add the generated output to the project and recompile.
- The runtime expects glyph UVs (u0,v0,u1,v1) normalized to atlas size and uses instanced quads to render text.

Usage notes
- Standalone flow (exe_demo):
  - The demo creates a small window and renders shapes + text using the renderer API.
  - Console output is enabled for quick diagnostics.
- Hooked flow (dll_demo):
  - The DLL finds swapchain/command queue offsets and hooks Present/ResizeBuffers.
  - Injection into other processes can trigger anti-cheat / security detections — only use on trusted targets.

Important implementation notes & troubleshooting
- Shader compilation:
  - Requires DXC (DirectX Shader Compiler) available at runtime (the project uses DXC via vcpkg/DirectX tooling). If shader compilation fails, ensure DXC is present and visible to the build/link environment.
  - If you see errors complaining about missing SDF helpers (e.g. `sdCircle`, `sdBox`, `sdLine`), ensure `shader_compiler.hpp` contains the SDF helper implementations. These functions are required by the pixel shader branches that draw SDF shapes.
- Text rendering gotchas:
  - The vertex shader must pass per-vertex `quad_pos` in [0,1]^2 (unit quad coordinates) to the pixel shader.
  - The pixel shader reconstructs UV like: `uv = lerp(inst_uv.xy, inst_uv.zw, quad_pos)`.
  - Use a point sampler or inset UV by half-texel to avoid bilinear bleed between packed glyphs. The project uses a point/static sampler by default to avoid bleed.
  - If sampled glyph alpha is zero for all uv, verify that the atlas SRV was created and that the font atlas upload succeeded (check console logs during `initialize_fonts()`).
- Descriptor heaps & frame resources:
  - The project uses a small SRV allocator (`srv_allocator.hpp`) with transient entries per-frame; ensure buffer_count is configured to match swapchain.
  - When resizing: frame resources must be signaled and waited for before resizing swapchain resources.

Debugging tips
- Enable console output (demo apps already do) and read errors printed to stderr/stdout.
- When debugging shader compilation errors, inspect the DXC error string printed to console — it includes file/line info from the virtual file `"shader.hlsl"`.
- If text doesn't show:
  - Verify `m_font_srv` is valid, descriptor heaps are set on the command list (`ID3D12GraphicsCommandList::SetDescriptorHeaps`), and the correct root descriptor table is bound before sampling.
  - Check PSO sampler bindings and that the static sampler is registered at the same shader register (s0).
- For device removed / presentation failures, check `GetDeviceRemovedReason()` and log the HRESULT.

Project layout (high level)
- `flashgui/` — renderer, shaders, helpers, and overlay UI
- `exe_demo/` — standalone demo app
- `dll_demo/` — injector-style demo (DLL main + entry thread)
- `font_converter/` — tool to bake TTF fonts into C++ source (uses stb_truetype)
- `vcpkg.json` — dependency manifest for vcpkg

Contributing
- Feel free to open issues or PRs. Keep changes small and focused.
- If you modify shader code, test on both debug and release DXC compile settings.

License
- See repository root for LICENSE file. If none, ask the project owner before reuse.

Contact / help
- For quick help: build in Debug, run the demo, and paste console output/errors into an issue.
- If you want, I can:
  - Add a small README section with exact vcpkg package names derived from the current manifest.
  - Create a troubleshooting snippet that prints nested exceptions (using std::throw_with_nested) and adds a top-level printer to the demo console.
