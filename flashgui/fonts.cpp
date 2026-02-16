#include "pch.h"
#include "fonts.h"

using namespace fgui;
using Microsoft::WRL::ComPtr;

void c_fonts::initialize(ComPtr<ID3D12Device> device) {

    // DirectWrite factory + system font collection
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwrite_factory);
    if (!m_dwrite_factory) throw std::runtime_error("Failed to create DirectWrite factory");
    m_dwrite_factory->GetSystemFontCollection(&m_system_fonts, FALSE);

    // create a shader-visible SRV heap to hold font atlas SRVs (one descriptor per font)
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = static_cast<UINT>(k_max_fonts);
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_font_srv_heap))))
        throw std::runtime_error("Failed to create font SRV heap");

    m_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_next_descriptor_index = 0;
}

std::vector<std::wstring> c_fonts::enumerate_families() const {
    std::vector<std::wstring> out;
    if (!m_system_fonts) return out;
    UINT32 count = m_system_fonts->GetFontFamilyCount();
    out.reserve(count);
    for (UINT32 i = 0; i < count; ++i) {
        ComPtr<IDWriteFontFamily> fam;
        if (SUCCEEDED(m_system_fonts->GetFontFamily(i, &fam))) {
            WCHAR name[256];
            UINT32 nameLen = 0;
            BOOL exists = FALSE;
			IDWriteLocalizedStrings* names = nullptr;
            fam->GetFamilyNames(&names);
            names->GetStringLength(0, &nameLen); // safe query; index 0 is one language
            names->GetString(0, name, std::min(256u, nameLen+1));
            out.emplace_back(name);
        }
    }
    return out;
}

font_handle c_fonts::allocate_handle_for_key(const font_key& key) {
    auto it = m_key_to_handle.find(key);
    if (it != m_key_to_handle.end()) return it->second;

    if (m_next_descriptor_index >= k_max_fonts)
        throw std::runtime_error("Exceeded font capacity");

    // allocate a descriptor index and use it as the font_handle
    font_handle h = static_cast<font_handle>(m_next_descriptor_index++);
    m_key_to_handle.emplace(key, h);
    m_atlases.emplace(h, font_atlas{});
    m_atlases[h].key = key;
    return h;
}

font_handle c_fonts::get_or_create_font(const std::wstring& family,
    DWRITE_FONT_WEIGHT weight,
    DWRITE_FONT_STYLE style,
    int size_px, bool* exists, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> cmd_queue, frame_resource& current_frame) {
    font_key key{ family, weight, style, size_px };
    font_handle fh = allocate_handle_for_key(key);

    // if we've already built atlas, done
    auto& atlas = m_atlases[fh];
    if (atlas.texture) {
		*exists = true;
        return fh;
    }

    // build atlas (may throw)
    if (!build_font_atlas(fh, atlas, device, cmd_queue, current_frame)) {
        // failed to build
        std::string msg = "Failed to build font atlas for " + std::string(key.family.begin(), key.family.end()) +
			" w=" + std::to_string(key.weight) + " s=" + std::to_string(key.style) + " sz=" + std::to_string(key.size_px);
		OutputDebugStringA(msg.c_str());
        std::cerr << msg;

        return 0;
    }
    return fh;
}

/*
PSEUDOCODE / PLAN (detailed):
- Problem: some glyphs visually overlap because the atlas packing uses the glyph's design advance (gm.advanceWidth * scale)
  which can be fractional and can be smaller than the glyph's raster width (overhangs / negative bearings), or rounding
  can cause adjacent glyphs to touch.
- Approach:
  1. Compute the glyph advance in physical pixels by scaling the design advance (keep as float for layout).
  2. Add a small pixel padding in the atlas packing (pad) to avoid sampling touching neighbors.
  3. Expose a per-glyph offset_x so renderer can place the glyph bitmap precisely relative to the pen.
  4. Use ClearType rendering mode for sharper horizontal edges on LCDs.
*/

bool c_fonts::build_font_atlas(font_handle fh, font_atlas& atlas, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> cmd_queue, frame_resource& current_frame) {
    // Find family -> font -> fontface
    UINT32 index = 0;
    BOOL exists = FALSE;
    if (FAILED(m_system_fonts->FindFamilyName(atlas.key.family.c_str(), &index, &exists)) || !exists) {
        // not found
        return false;
    }
    ComPtr<IDWriteFontFamily> family;
    m_system_fonts->GetFontFamily(index, &family);

    ComPtr<IDWriteFont> font;
    family->GetFirstMatchingFont(atlas.key.weight, DWRITE_FONT_STRETCH_NORMAL, atlas.key.style, &font);
    ComPtr<IDWriteFontFace> font_face;
    font->CreateFontFace(&font_face);

    // atlas params (simple, same packing approach you used)
    const int atlas_w = 512;
    const int atlas_h = 512;
    atlas.atlas_w = atlas_w;
    atlas.atlas_h = atlas_h;
    std::vector<uint8_t> atlas_data(atlas_w * atlas_h * 4, 0);

    DWRITE_FONT_METRICS metrics;
    font_face->GetMetrics(&metrics);

    // scale: here key.size_px is pixel height (choose convention)
    float scale = (float)atlas.key.size_px / (float)metrics.designUnitsPerEm;
    int ascender_px = int(metrics.ascent * scale);
    int baseline_offset = ascender_px;

    int cursor_x = 0, cursor_y = 0, line_h = 0;

    // small packing padding to avoid touching neighbors when sampling
    const int pack_pad = 1;

    // build glyphs for basic ASCII range (you can extend to glyph ranges you need)
    for (uint32_t cp = 0; cp < 127; ++cp) {
        UINT32 codepoint_arr[] = { cp };
        UINT16 glyph_index = 0;
        font_face->GetGlyphIndicesW(codepoint_arr, 1, &glyph_index);

        UINT16 glyph_indices[] = { glyph_index };
        DWRITE_GLYPH_METRICS gm;
        font_face->GetDesignGlyphMetrics(glyph_indices, 1, &gm, FALSE);

        // Use floating advance for layout (preserve fractional advances, better spacing/kerning)
        float adv_f = gm.advanceWidth * scale;
        int glyph_advance_px = std::max(1, int(std::ceil(adv_f)));

        // glyph run analysis to get raster bounds
        DWRITE_GLYPH_RUN glyph_run = {};
        glyph_run.fontFace = font_face.Get();
        glyph_run.fontEmSize = static_cast<FLOAT>(atlas.key.size_px);
        glyph_run.glyphCount = 1;
        glyph_run.glyphIndices = &glyph_index;

        ComPtr<IDWriteGlyphRunAnalysis> analysis;
        // Use ClearType natural rendering for crisper horizontal detail on LCDs
        HRESULT hr = m_dwrite_factory->CreateGlyphRunAnalysis(
            &glyph_run, 1.0f, nullptr, DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL,
            DWRITE_MEASURING_MODE_NATURAL, 0.0f, 1.0f, &analysis);

        RECT bounds{ 0,0,0,0 };
        if (SUCCEEDED(hr)) analysis->GetAlphaTextureBounds(DWRITE_TEXTURE_CLEARTYPE_3x1, &bounds);
        int w = bounds.right - bounds.left;
        int h = bounds.bottom - bounds.top;
        if (w <= 0) w = glyph_advance_px;
        if (w < glyph_advance_px) w = glyph_advance_px;
        if (h <= 0) h = ascender_px + int(metrics.descent * scale);

        // line packing
        if (cursor_x + w + pack_pad > atlas_w) { cursor_x = 0; cursor_y += line_h; line_h = 0; }
        if (cursor_y + h + pack_pad > atlas_h) {
            // atlas full; in production you should grow atlas or make larger atlas
            break;
        }

        if (SUCCEEDED(hr) && w > 0 && h > 0) {
            std::vector<BYTE> rowbuf(w * 3);
            for (int gy = 0; gy < h; ++gy) {
                RECT row_bounds = { bounds.left, bounds.top + gy, bounds.right, bounds.top + gy + 1 };
                UINT bufsize = static_cast<UINT>(rowbuf.size());
                if (FAILED(analysis->CreateAlphaTexture(DWRITE_TEXTURE_CLEARTYPE_3x1, &row_bounds, rowbuf.data(), bufsize)))
                    continue;
                for (int gx = 0; gx < w; ++gx) {
                    BYTE r = rowbuf[gx * 3 + 0];
                    BYTE g = rowbuf[gx * 3 + 1];
                    BYTE b = rowbuf[gx * 3 + 2];
                    int px = (cursor_x + gx);
                    int py = (cursor_y + gy);
                    size_t idx = (py * atlas_w + px) * 4;

                    // Store subpixel coverage into RGB and set alpha to 255 so shader can reconstruct properly.
                    // This preserves ClearType detail. If you prefer grayscale, set rgb=255 and alpha=avg.
                    atlas_data[idx + 0] = r;     // subpixel R coverage
                    atlas_data[idx + 1] = g;     // subpixel G coverage
                    atlas_data[idx + 2] = b;     // subpixel B coverage
                    atlas_data[idx + 3] = 255;   // full alpha (we carry coverage in RGB)
                }
            }
        }

        font_glyph_info gi{};
        // UV rect for the glyph in the atlas
        gi.u0 = float(cursor_x) / float(atlas_w);
        gi.v0 = float(cursor_y) / float(atlas_h);
        gi.u1 = float(cursor_x + w) / float(atlas_w);
        gi.v1 = float(cursor_y + h) / float(atlas_h);
        // use the floating design advance so render positions preserve sub-pixel spacing & kerning
        gi.advance = adv_f;
        // compute vertical offset (existing logic)
        gi.offset_y = baseline_offset - int(metrics.ascent * scale - bounds.top);
        // compute horizontal offset: distance from pen origin to bitmap origin (in pixels)
        // bounds.left is bitmap origin relative to glyph origin; gm.leftSideBearing is design units from glyph origin to left ink edge
        gi.offset_x = int(std::round(bounds.left - gm.leftSideBearing * scale));
        gi.metrics = gm;

        atlas.glyphs.emplace(cp, gi);

        // advance cursor with an extra pad between entries to avoid touching/bleeding
        cursor_x += w + pack_pad;
        line_h = std::max(line_h, h);
    }

    // create GPU texture
    D3D12_RESOURCE_DESC tex_desc = {};
    tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    tex_desc.Width = atlas_w;
    tex_desc.Height = atlas_h;
    tex_desc.DepthOrArraySize = 1;
    tex_desc.MipLevels = 1;
    tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
    if (FAILED(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &tex_desc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&atlas.texture))))
        throw std::runtime_error("Failed to create font texture");

    // upload (simple - reuse your existing upload flow)
    UINT64 uploadSize = GetRequiredIntermediateSize(atlas.texture.Get(), 0, 1);
    ComPtr<ID3D12Resource> upload;
    CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
    auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
    if (FAILED(device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload))))
        throw std::runtime_error("Failed to create upload buffer");

    D3D12_SUBRESOURCE_DATA sub{};
    sub.pData = atlas_data.data();
    sub.RowPitch = atlas_w * 4;
    sub.SlicePitch = sub.RowPitch * atlas_h;

    auto& cmd = current_frame.command_list;
    cmd->Reset(current_frame.command_allocator.Get(), nullptr);
    UpdateSubresources(cmd.Get(), atlas.texture.Get(), upload.Get(), 0, 0, 1, &sub);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(atlas.texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd->ResourceBarrier(1, &barrier);
    cmd->Close();

    ID3D12CommandList* lists[] = { cmd.Get() };
    cmd_queue->ExecuteCommandLists(1, lists);
    current_frame.signal(cmd_queue);
    current_frame.wait_for_gpu();

    // Use the font_handle value as the descriptor index (do NOT increment the descriptor counter here)
    uint32_t descriptor_index = static_cast<uint32_t>(fh);
    if (descriptor_index >= k_max_fonts) throw std::runtime_error("Font handle exceeds heap capacity");

    auto cpu = m_font_srv_heap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += SIZE_T(descriptor_index) * m_descriptor_size;

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    device->CreateShaderResourceView(atlas.texture.Get(), &srv_desc, cpu);

    auto gpu = m_font_srv_heap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += SIZE_T(descriptor_index) * m_descriptor_size;
    atlas.srv_gpu = gpu;

    return true;
}

const font_glyph_info* c_fonts::get_glyph_info(font_handle fh, uint32_t codepoint) const {
    auto it = m_atlases.find(fh);
    if (it == m_atlases.end()) return nullptr;
    auto git = it->second.glyphs.find(codepoint);
    if (git == it->second.glyphs.end()) return nullptr;
    return &git->second;
}

D3D12_GPU_DESCRIPTOR_HANDLE c_fonts::get_font_srv_gpu(font_handle fh) const {
    auto it = m_atlases.find(fh);
    if (it == m_atlases.end()) return {};
    return it->second.srv_gpu;
}