#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"  // https://github.com/nothings/stb

static std::vector<uint8_t> ReadWholeFile(const std::string& path)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) throw std::runtime_error("Failed to open: " + path);

    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    if (size < 0) { std::fclose(f); throw std::runtime_error("ftell failed"); }

    std::rewind(f);
    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (size > 0) {
        if (std::fread(data.data(), 1, static_cast<size_t>(size), f) != static_cast<size_t>(size)) {
            std::fclose(f); throw std::runtime_error("fread failed");
        }
    }
    std::fclose(f);
    return data;
}

static uint32_t CalcChecksum(const std::vector<uint8_t>& data)
{
    uint32_t sum = 0;
    for (uint8_t b : data) sum += b;
    return sum;
}

static void EmitArrayChunked(FILE* out, const char* baseName, const std::vector<uint8_t>& data)
{
    const size_t kMaxChunkKB = 90;  // Conservative for most IDEs
    const size_t kMaxChunkSize = kMaxChunkKB * 1024;

    size_t offset = 0;
    size_t chunkIdx = 0;

    while (offset < data.size()) {
        size_t chunkSize = std::min(kMaxChunkSize, data.size() - offset);
        std::string chunkName = std::string(baseName) + "_c" + std::to_string(chunkIdx++);

        std::fprintf(out, "// %s: bytes [%zu-%zu] (%zu KB)\n",
            chunkName.c_str(), offset, offset + chunkSize - 1, chunkSize / 1024);
        std::fprintf(out, "extern const uint8_t %s[%zu];\n", chunkName.c_str(), chunkSize);

        // Write chunk file
        std::string chunkPath = std::string(chunkName) + ".cpp";
        FILE* chunkFile = std::fopen(chunkPath.c_str(), "wb");
        if (!chunkFile) throw std::runtime_error("Cannot create: " + chunkPath);

        std::fprintf(chunkFile,
            R"(#include <cstdint>

const uint8_t %s[%zu] = {
)", chunkName.c_str(), chunkSize);

        const size_t kPerLine = 16;
        for (size_t i = 0; i < chunkSize; ++i) {
            if (i % kPerLine == 0) std::fprintf(chunkFile, "\n    ");
            std::fprintf(chunkFile, "0x%02X%s", data[offset + i],
                (/*(i + 1) % kPerLine == 0 || */i + 1 == chunkSize) ? "" : ", ");
        }
        std::fprintf(chunkFile, "\n};\n");
        std::fclose(chunkFile);

        offset += chunkSize;
        std::printf("Wrote %s (%zu KB)\n", chunkPath.c_str(), chunkSize / 1024);
    }
    std::fprintf(out, "\n");
}

struct GlyphInfo {
    uint16_t codepoint;
    uint16_t atlasX, atlasY, width, height;
    float    u0, v0, u1, v1, advance, bearingX, bearingY;
};

static void EmitGlyphTable(FILE* out, const char* varName,
    const std::vector<GlyphInfo>& glyphs, int atlasW, int atlasH)
{
    std::fprintf(out,
        R"(struct GlyphInfo {
    uint16_t codepoint;
    uint16_t atlasX, atlasY, width, height;
    float u0, v0, u1, v1, advance, bearingX, bearingY;
};
)");

    std::fprintf(out, "\nconst GlyphInfo %s[%zu] = {\n", varName, glyphs.size());
    for (const auto& g : glyphs) {
        std::fprintf(out,
            "    {%4u,%4u,%4u,%4u,%4u, %7.5f,%7.5f,%7.5f,%7.5f,%7.3f,%7.3f,%7.3f},\n",
            g.codepoint, g.atlasX, g.atlasY, g.width, g.height,
            g.u0, g.v0, g.u1, g.v1, g.advance, g.bearingX, g.bearingY);
    }
    std::fprintf(out, "};\n");
    std::fprintf(out, "const size_t %sCount = %zu;\n", varName + 2, glyphs.size());
    std::fprintf(out, "const int %sAtlasW = %d, %sAtlasH = %d;\n\n",
        varName + 2, atlasW, varName + 2, atlasH);
}

// RLE: [run_length, value] pairs (run 1-255)
static std::vector<uint8_t> CompressRLE(const std::vector<uint8_t>& raw)
{
    std::vector<uint8_t> compressed;
    size_t i = 0;
    while (i < raw.size()) {
        uint8_t val = raw[i];
        size_t run = 1;
        while (i + run < raw.size() && raw[i + run] == val && run < 255) ++run;
        compressed.push_back(static_cast<uint8_t>(run));
        compressed.push_back(val);
        i += run;
    }
    return compressed;
}

static std::vector<GlyphInfo> PrecomputeGlyphs(const std::vector<uint8_t>& fontData,
    float pixelHeight,
    std::vector<uint8_t>& outAtlasRLE)
{
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontData.data(), 0))
        throw std::runtime_error("stbtt_InitFont failed");

    float scale = stbtt_ScaleForPixelHeight(&font, pixelHeight);
    std::vector<GlyphInfo> glyphs;

    const int kFirstCP = 32, kLastCP = 255;
    const int kAtlasW = 1024, kAtlasH = 1024;
    std::vector<uint8_t> atlas(kAtlasW * kAtlasH, 0);

    int x = 0, y = 0, maxH = 0;

    for (int cp = kFirstCP; cp <= kLastCP; ++cp) {
        int ix = stbtt_FindGlyphIndex(&font, cp);
        if (ix == 0) continue;

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font, cp, scale, scale, &x0, &y0, &x1, &y1);
        int w = x1 - x0, h = y1 - y0;
        if (w <= 0 || h <= 0) continue;

        if (x + w > kAtlasW) { x = 0; y += maxH; maxH = 0; }
        if (y + h > kAtlasH) break;  // Overflow

        stbtt_MakeCodepointBitmap(&font, atlas.data() + (y + y0) * kAtlasW + (x + x0),
            w, h, kAtlasW, scale, scale, cp);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, cp, &advance, &lsb);

        GlyphInfo gi{
            uint16_t(cp), uint16_t(x + x0), uint16_t(y + y0),
            uint16_t(w), uint16_t(h),
            float(x + x0) / kAtlasW, float(y + y0) / kAtlasH,
            float(x + x0 + w) / kAtlasW, float(y + y0 + h) / kAtlasH,
            float(advance) * scale, float(lsb) * scale, float(-y0) * scale
        };
        glyphs.push_back(gi);

        x += int(gi.advance);
        maxH = std::max(maxH, h);
    }

    outAtlasRLE = CompressRLE(atlas);
    std::sort(glyphs.begin(), glyphs.end(),
        [](const GlyphInfo& a, const GlyphInfo& b) { return a.codepoint < b.codepoint; });
    return glyphs;
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::fprintf(stderr, "Usage: %s REGULAR.ttf BOLD.ttf OUTPUT.cpp\n", argv[0]);
        return 1;
    }

    try {
        auto regData = ReadWholeFile(argv[1]);
        auto boldData = ReadWholeFile(argv[2]);

        FILE* out = std::fopen(argv[3], "wb");
        if (!out) throw std::runtime_error("Cannot open output");

        std::fprintf(out,
            R"(#include <cstdint>
#include <cstddef>

// AUTOGENERATED FONT DATA - NO RUNTIME TTF PARSING
// DejaVu Sans Regular + Bold: glyphs, atlases (RLE), checksums
)");

        // TTF bytes (chunked)
        EmitArrayChunked(out, "g_DejaVuSans", regData);
        std::fprintf(out, "const size_t g_DejaVuSansSize = %zu;\n", regData.size());
        std::fprintf(out, "const uint32_t g_DejaVuSansChecksum = 0x%08X;\n\n",
            CalcChecksum(regData));

        EmitArrayChunked(out, "g_DejaVuSansBold", boldData);
        std::fprintf(out, "const size_t g_DejaVuSansBoldSize = %zu;\n", boldData.size());
        std::fprintf(out, "const uint32_t g_DejaVuSansBoldChecksum = 0x%08X;\n\n",
            CalcChecksum(boldData));

        // Regular glyphs + atlas
        std::vector<uint8_t> regAtlasRLE;
        auto regGlyphs = PrecomputeGlyphs(regData, 48.0f, regAtlasRLE);
        EmitGlyphTable(out, "g_DejaVuSansGlyphs", regGlyphs, 1024, 1024);
        EmitArrayChunked(out, "g_DejaVuSansAtlasRLE", regAtlasRLE);
        std::fprintf(out, "const size_t g_DejaVuSansAtlasRLESize = %zu;\n\n", regAtlasRLE.size());

        // Bold glyphs + atlas
        std::vector<uint8_t> boldAtlasRLE;
        auto boldGlyphs = PrecomputeGlyphs(boldData, 48.0f, boldAtlasRLE);
        EmitGlyphTable(out, "g_DejaVuSansBoldGlyphs", boldGlyphs, 1024, 1024);
        EmitArrayChunked(out, "g_DejaVuSansBoldAtlasRLE", boldAtlasRLE);
        std::fprintf(out, "const size_t g_DejaVuSansBoldAtlasRLESize = %zu;\n\n", boldAtlasRLE.size());

        std::fclose(out);

        std::printf("\n Generated %s + %zu chunk files\n", argv[3],
            (regData.size() + boldData.size() + regAtlasRLE.size() + boldAtlasRLE.size()) / 90000 + 1);
        std::printf("  Glyphs: %zu reg / %zu bold\n", regGlyphs.size(), boldGlyphs.size());

    }
    catch (const std::exception& e) {
        std::fprintf(stderr, "ERROR: %s\n", e.what());
        return 1;
    }
    return 0;
}
