#pragma once
#include <stdint.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "frame_resource.hpp"
using Microsoft::WRL::ComPtr;

namespace fgui {

    // font_handle is used in instance packing (high 16 bits). Make it explicit and sized.
    using font_handle = uint16_t;
    static constexpr uint32_t k_max_fonts = 256; // tune as needed; must be <= 65535 if you pack into 16 bits

    struct font_glyph_info {
        float u0, v0, u1, v1; // UV rect in atlas
        float advance;        // in pixels (floating to allow sub-pixel placement)
        int offset_x;         // horizontal offset (pixels) from pen position to bitmap origin
        int offset_y;         // vertical offset (pixels) from baseline to bitmap origin
        DWRITE_GLYPH_METRICS metrics; // Glyph metrics from DirectWrite
    };

    struct font_key {
        std::wstring family;
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
        int size_px = 16;

        bool operator==(const font_key& o) const noexcept {
            return family == o.family && weight == o.weight && style == o.style && size_px == o.size_px;
        }
    };

    struct font_key_hash {
        size_t operator()(font_key const& k) const noexcept {
            std::hash<std::wstring> sh;
            size_t h = sh(k.family);
            h ^= (size_t)k.weight + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= (size_t)k.style  + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= (size_t)k.size_px + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            return h;
        }
    };

    struct font_atlas {
        font_key key;
        std::unordered_map<uint32_t, font_glyph_info> glyphs;
        ComPtr<ID3D12Resource> texture;
        D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu{}; // in the global heap
        int atlas_w = 0;
        int atlas_h = 0;
    };

    class c_fonts
    {
    public:
        c_fonts() = default;

        void initialize(ComPtr<ID3D12Device> device);

        std::vector<std::wstring> enumerate_families() const;

        // returns non-zero font_handle on success, 0 on failure
        font_handle get_or_create_font(const std::wstring& family,
                                      DWRITE_FONT_WEIGHT weight,
                                      DWRITE_FONT_STYLE style,
                                      int size_px, bool* exists, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> cmd_queue, frame_resource& current_frame);

        const font_glyph_info* get_glyph_info(font_handle fh, uint32_t codepoint) const;
        ComPtr<ID3D12DescriptorHeap> get_font_srv_heap() const { return m_font_srv_heap; }
        D3D12_GPU_DESCRIPTOR_HANDLE get_font_srv_gpu(font_handle fh) const;

    private:
        bool build_font_atlas(font_handle fh, font_atlas& atlas, ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> cmd_queue, frame_resource& current_frame);
        font_handle allocate_handle_for_key(const font_key& key);

        ComPtr<IDWriteFactory> m_dwrite_factory;
        ComPtr<IDWriteFontCollection> m_system_fonts;

        std::unordered_map<font_key, font_handle, font_key_hash> m_key_to_handle;
        std::unordered_map<font_handle, font_atlas> m_atlases;

        ComPtr<ID3D12DescriptorHeap> m_font_srv_heap;
        uint32_t m_descriptor_size = 0;

        // descriptor allocator index (also used as the handle value)
        uint32_t m_next_descriptor_index = 0;
    };

} // namespace fgui