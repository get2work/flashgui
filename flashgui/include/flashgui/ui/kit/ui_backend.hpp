#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <sstream>

namespace fgui::ui {

const int32 UI_CORNER_TL = 1;
const int32 UI_CORNER_TR = 2;
const int32 UI_CORNER_BL = 4;
const int32 UI_CORNER_BR = 8;
const uint8 UI_KEY_DOWN = 1;
const uint8 UI_KEY_TOGGLE = 2;
const color UI_GFX_NO_OUTLINE = color(0, 0, 0, 0);
const int32 UI_GFX_FX_OUTLINE = 1;

inline DirectX::XMFLOAT4 ui_gfx_color(color c) {
	constexpr float64 channel = 1.0 / 255.0;
	return {
		static_cast<float>(c.r * channel),
		static_cast<float>(c.g * channel),
		static_cast<float>(c.b * channel),
		static_cast<float>(c.a * channel)
	};
}

inline vec2i ui_gfx_point(vec2 p) {
	return vec2i(static_cast<int>(std::lround(p.x)), static_cast<int>(std::lround(p.y)));
}

inline void ui_gfx_triangle(vec2 a, vec2 b, vec2 c, color col);
inline void ui_gfx_line(vec2 a, vec2 b, color c, float64 thick);

inline void ui_gfx_fill_mask(vec2 pos, vec2 size, color c, float64 round, int32 corners) {
	if (!fgui::render || size.x <= 0.0 || size.y <= 0.0)
		return;
	if (corners == 0)
		corners = UI_CORNER_TL | UI_CORNER_TR | UI_CORNER_BL | UI_CORNER_BR;
	const float64 radius = std::clamp(round, 0.0, min(size.x, size.y) * 0.5);
	if (radius <= 0.0) {
		fgui::render->draw_quad(ui_gfx_point(pos), ui_gfx_point(size), ui_gfx_color(c));
		return;
	}
	std::vector<vec2> points;
	const float64 pi = 3.14159265358979323846;
	const int segments = 5;
	const auto add_corner = [&](float64 cx, float64 cy, float64 start, bool rounded, float64 square_x, float64 square_y) {
		if (!rounded) {
			points.emplace_back(square_x, square_y);
			return;
		}
		for (int i = 0; i <= segments; ++i) {
			const float64 angle = start + (pi * 0.5) * i / segments;
			points.emplace_back(cx + std::cos(angle) * radius, cy + std::sin(angle) * radius);
		}
	};
	add_corner(pos.x + radius, pos.y + radius, pi, (corners & UI_CORNER_TL) != 0, pos.x, pos.y);
	add_corner(pos.x + size.x - radius, pos.y + radius, -pi * 0.5, (corners & UI_CORNER_TR) != 0,
		pos.x + size.x, pos.y);
	add_corner(pos.x + size.x - radius, pos.y + size.y - radius, 0.0, (corners & UI_CORNER_BR) != 0,
		pos.x + size.x, pos.y + size.y);
	add_corner(pos.x + radius, pos.y + size.y - radius, pi * 0.5, (corners & UI_CORNER_BL) != 0,
		pos.x, pos.y + size.y);
	const vec2 center(pos.x + size.x * 0.5, pos.y + size.y * 0.5);
	for (size_t i = 0; i < points.size(); ++i)
		ui_gfx_triangle(center, points[i], points[(i + 1) % points.size()], c);
}

inline void ui_gfx_fill(vec2 pos, vec2 size, color c, float64 round) {
	ui_gfx_fill_mask(pos, size, c, round, UI_CORNER_TL | UI_CORNER_TR | UI_CORNER_BL | UI_CORNER_BR);
}

inline void ui_gfx_fill_corners(vec2 pos, vec2 size, color c, float64 round, int32 corners) {
	ui_gfx_fill_mask(pos, size, c, round, corners);
}

inline void ui_gfx_frame(vec2 pos, vec2 size, color c, float64 thick, float64 round) {
	if (!fgui::render || size.x <= 0.0 || size.y <= 0.0)
		return;
	const float64 radius = std::clamp(round, 0.0, min(size.x, size.y) * 0.5);
	if (radius <= 0.0) {
		fgui::render->draw_quad_outline(ui_gfx_point(pos), ui_gfx_point(size), ui_gfx_color(c), static_cast<float>(thick));
		return;
	}
	const float64 pi = 3.14159265358979323846;
	const vec2 centers[] = {
		{ pos.x + radius, pos.y + radius },
		{ pos.x + size.x - radius, pos.y + radius },
		{ pos.x + size.x - radius, pos.y + size.y - radius },
		{ pos.x + radius, pos.y + size.y - radius }
	};
	const float64 starts[] = { pi, -pi * 0.5, 0.0, pi * 0.5 };
	const int arc_segments = 5;
	for (int corner = 0; corner < 4; ++corner) {
		for (int i = 0; i < arc_segments; ++i) {
			const float64 a0 = starts[corner] + (pi * 0.5) * i / arc_segments;
			const float64 a1 = starts[corner] + (pi * 0.5) * (i + 1) / arc_segments;
			ui_gfx_line(vec2(centers[corner].x + std::cos(a0) * radius, centers[corner].y + std::sin(a0) * radius),
				vec2(centers[corner].x + std::cos(a1) * radius, centers[corner].y + std::sin(a1) * radius), c, thick);
		}
	}
	ui_gfx_line(vec2(pos.x + radius, pos.y), vec2(pos.x + size.x - radius, pos.y), c, thick);
	ui_gfx_line(vec2(pos.x + size.x, pos.y + radius), vec2(pos.x + size.x, pos.y + size.y - radius), c, thick);
	ui_gfx_line(vec2(pos.x + size.x - radius, pos.y + size.y), vec2(pos.x + radius, pos.y + size.y), c, thick);
	ui_gfx_line(vec2(pos.x, pos.y + size.y - radius), vec2(pos.x, pos.y + radius), c, thick);
}

inline void ui_gfx_fills(const std::vector<float32>& rects, color c) {
	for (size_t i = 0; i + 3 < rects.size(); i += 4)
		ui_gfx_fill(vec2(rects[i], rects[i + 1]), vec2(rects[i + 2], rects[i + 3]), c, 0.0);
}

inline void ui_gfx_fills_each(const std::vector<float32>& rects, const std::vector<color>& cols) {
	const size_t count = min(rects.size() / 4, cols.size());
	for (size_t i = 0; i < count; ++i) {
		const size_t at = i * 4;
		ui_gfx_fill(vec2(rects[at], rects[at + 1]), vec2(rects[at + 2], rects[at + 3]), cols[i], 0.0);
	}
}

inline color ui_gfx_gradient_at(const std::vector<color>& stops, float64 t) {
	if (stops.empty())
		return color(0, 0, 0, 0);
	if (stops.size() == 1)
		return stops.front();
	t = std::clamp(t, 0.0, 1.0);
	const float64 scaled = t * static_cast<float64>(stops.size() - 1);
	const size_t index = min(static_cast<size_t>(scaled), stops.size() - 2);
	return stops[index].lerp(stops[index + 1], scaled - static_cast<float64>(index));
}

inline void ui_gfx_gradient(vec2 pos, vec2 size, const std::vector<color>& stops, float64, bool horizontal) {
	if (stops.empty() || size.x <= 0.0 || size.y <= 0.0)
		return;
	const float64 length = horizontal ? size.x : size.y;
	const size_t segments = min<size_t>(max<size_t>(stops.size() * 8, 1), static_cast<size_t>(max(1.0, length)));
	for (size_t i = 0; i < segments; ++i) {
		const float64 t0 = static_cast<float64>(i) / static_cast<float64>(segments);
		const float64 t1 = static_cast<float64>(i + 1) / static_cast<float64>(segments);
		vec2 part_pos = pos;
		vec2 part_size = size;
		if (horizontal) {
			part_pos.x += size.x * t0;
			part_size.x = size.x * (t1 - t0) + 1.0;
		} else {
			part_pos.y += size.y * t0;
			part_size.y = size.y * (t1 - t0) + 1.0;
		}
		ui_gfx_fill(part_pos, part_size, ui_gfx_gradient_at(stops, (t0 + t1) * 0.5), 0.0);
	}
}

inline void ui_gfx_corners(vec2 pos, vec2 size, color tl, color tr, color bl, color br, float64) {
	constexpr size_t columns = 12;
	constexpr size_t rows = 8;
	for (size_t y = 0; y < rows; ++y) {
		const float64 y0 = static_cast<float64>(y) / rows;
		const float64 y1 = static_cast<float64>(y + 1) / rows;
		for (size_t x = 0; x < columns; ++x) {
			const float64 x0 = static_cast<float64>(x) / columns;
			const float64 x1 = static_cast<float64>(x + 1) / columns;
			const color top = tl.lerp(tr, (x0 + x1) * 0.5);
			const color bottom = bl.lerp(br, (x0 + x1) * 0.5);
			ui_gfx_fill(vec2(pos.x + size.x * x0, pos.y + size.y * y0),
				vec2(size.x * (x1 - x0) + 1.0, size.y * (y1 - y0) + 1.0),
				top.lerp(bottom, (y0 + y1) * 0.5), 0.0);
		}
	}
}

inline void ui_gfx_line(vec2 a, vec2 b, color c, float64 thick) {
	if (fgui::render)
		fgui::render->draw_line(ui_gfx_point(a), ui_gfx_point(b), ui_gfx_color(c), static_cast<float>(thick));
}

inline void ui_gfx_gradient_line(vec2 a, vec2 b, const std::vector<color>& stops, float64 thick) {
	if (stops.empty())
		return;
	const float64 dx = b.x - a.x;
	const float64 dy = b.y - a.y;
	const float64 length = std::sqrt(dx * dx + dy * dy);
	const size_t segments = min<size_t>(max<size_t>(stops.size() * 8, 1), static_cast<size_t>(max(1.0, length)));
	for (size_t i = 0; i < segments; ++i) {
		const float64 t0 = static_cast<float64>(i) / static_cast<float64>(segments);
		const float64 t1 = static_cast<float64>(i + 1) / static_cast<float64>(segments);
		ui_gfx_line(vec2(a.x + dx * t0, a.y + dy * t0), vec2(a.x + dx * t1, a.y + dy * t1),
			ui_gfx_gradient_at(stops, (t0 + t1) * 0.5), thick);
	}
}

inline void ui_gfx_disc(vec2 centre, float64 r, color c) {
	if (fgui::render) {
		const int diameter = max(1, static_cast<int>(std::lround(r * 2.0)));
		fgui::render->draw_circle(ui_gfx_point(vec2(centre.x - r, centre.y - r)), vec2i(diameter, diameter), ui_gfx_color(c));
	}
}

inline void ui_gfx_discs(const std::vector<float32>& xyr, color c) {
	for (size_t i = 0; i + 2 < xyr.size(); i += 3)
		ui_gfx_disc(vec2(xyr[i], xyr[i + 1]), xyr[i + 2], c);
}

inline void ui_gfx_discs_each(const std::vector<float32>& xyr, const std::vector<color>& cols) {
	const size_t count = min(xyr.size() / 3, cols.size());
	for (size_t i = 0; i < count; ++i)
		ui_gfx_disc(vec2(xyr[i * 3], xyr[i * 3 + 1]), xyr[i * 3 + 2], cols[i]);
}

inline void ui_gfx_ring(vec2 centre, float64 r, color c, float64 thick) {
	if (fgui::render) {
		const int diameter = max(1, static_cast<int>(std::lround(r * 2.0)));
		fgui::render->draw_circle_outline(ui_gfx_point(vec2(centre.x - r, centre.y - r)),
			vec2i(diameter, diameter), ui_gfx_color(c), 0.0f, static_cast<float>(thick));
	}
}

inline void ui_gfx_arc(vec2 centre, float64 r, float64 start_deg, float64 sweep_deg, color c, float64 thick) {
	const int segments = max(1, static_cast<int>(std::ceil(std::abs(sweep_deg) / 8.0)));
	const float64 radians = 3.14159265358979323846 / 180.0;
	for (int i = 0; i < segments; ++i) {
		const float64 a0 = (start_deg + sweep_deg * i / segments) * radians;
		const float64 a1 = (start_deg + sweep_deg * (i + 1) / segments) * radians;
		ui_gfx_line(vec2(centre.x + std::cos(a0) * r, centre.y + std::sin(a0) * r),
			vec2(centre.x + std::cos(a1) * r, centre.y + std::sin(a1) * r), c, thick);
	}
}

inline void ui_gfx_triangle(vec2 a, vec2 b, vec2 c, color col) {
	if (fgui::render)
		fgui::render->draw_triangle(ui_gfx_point(a), ui_gfx_point(b), ui_gfx_point(c), ui_gfx_color(col));
}

inline void ui_gfx_polygon(const std::vector<float32>& pts, color c) {
	if (pts.size() < 6)
		return;
	const vec2 first(pts[0], pts[1]);
	for (size_t i = 2; i + 3 < pts.size(); i += 2)
		ui_gfx_triangle(first, vec2(pts[i], pts[i + 1]), vec2(pts[i + 2], pts[i + 3]), c);
}

inline void ui_gfx_clip_push(vec2 pos, vec2 size) {
	if (fgui::render)
		fgui::render->push_clip_rect(ui_gfx_point(pos), ui_gfx_point(size));
}

inline void ui_gfx_clip_pop() {
	if (fgui::render)
		fgui::render->pop_clip_rect();
}

inline vec2 ui_gfx_view() {
	if (!fgui::process)
		return vec2();
	return vec2(fgui::process->window.width, fgui::process->window.height);
}

inline std::map<int64, float64> g_ui_font_heights;

inline int64 ui_gfx_font(const std::string&, int64 px) {
	if (!fgui::render)
		return 0;
	const int size = max(1, static_cast<int>(px));
	const auto font = fgui::render->get_font(L"Segoe UI", size);
	g_ui_font_heights[font] = size * 1.2;
	return font;
}

inline int64 ui_gfx_font_default() {
	return ui_gfx_font("", 18);
}

inline vec2 ui_gfx_text_size(int64 font, const std::string& text) {
	if (!fgui::render)
		return vec2();
	const auto handle = static_cast<fgui::font_handle>(font);
	const float width = fgui::render->measure_text_width(text, handle);
	const auto height = g_ui_font_heights.find(font);
	const float64 text_height = height == g_ui_font_heights.end() ? 18.0 : height->second;
	return vec2(width, text_height);
}

inline int32 ui_gfx_text_fit(int64 font, float64 width, const std::string& text) {
	if (width <= 0.0)
		return 0;
	if (!fgui::render)
		return 0;
	const auto handle = static_cast<fgui::font_handle>(font);
	size_t low = 0;
	size_t high = text.size();
	while (low < high) {
		const size_t middle = (low + high + 1) / 2;
		if (fgui::render->measure_text_width(text.substr(0, middle), handle) <= width)
			low = middle;
		else
			high = middle - 1;
	}
	return static_cast<int32>(low);
}

inline int64 ui_gfx_font_ranges(const std::string& path, int64 px, const std::vector<uint32>&) {
	return ui_gfx_font(path, px);
}

inline void ui_gfx_text(const std::string& text, vec2 pos, color c, int64 font) {
	if (fgui::render)
		fgui::render->draw_text(text, ui_gfx_point(pos), static_cast<fgui::font_handle>(font), ui_gfx_color(c));
}

inline void ui_gfx_text_outline(const std::string& text, vec2 pos, color c, int64 font, color edge, float64 thick) {
	const int offset = max(1, static_cast<int>(std::lround(thick)));
	for (int y = -offset; y <= offset; ++y) {
		for (int x = -offset; x <= offset; ++x) {
			if (x != 0 || y != 0)
				ui_gfx_text(text, vec2(pos.x + x, pos.y + y), edge, font);
		}
	}
	ui_gfx_text(text, pos, c, font);
}

using UiKeyBitsFn = std::function<bool(int64, uint8&)>;
using UiKeyReadyFn = std::function<bool()>;
using UiIconTexFn = std::function<int64(const std::string&)>;
using UiIconAspectFn = std::function<float64(const std::string&)>;
using UiIconSettledFn = std::function<bool(const std::string&)>;
using UiIconDrawTexFn = std::function<void(int64, float64, float64, float64, float64, color)>;
using UiIconDrawPathFn = std::function<bool(const std::string&, float64, float64, float64)>;
using UiIconDrawKeyFn = std::function<bool(const std::string&, float64, float64, float64)>;
using UiIconLookupFn = std::function<std::string(const std::string&)>;
using UiIconPathFn = std::function<std::string(const std::string&)>;

struct ui_icon_provider_t {
	UiIconTexFn tex_fn;
	UiIconAspectFn aspect_fn;
	UiIconSettledFn settled_fn;
	UiIconDrawTexFn draw_tex_fn;
	UiIconDrawPathFn draw_path_fn;
	UiIconDrawKeyFn draw_key_fn;
	UiIconLookupFn lookup_fn;
	UiIconPathFn path_fn;
};

inline UiKeyBitsFn g_ui_key_bits_fn;
inline UiKeyReadyFn g_ui_key_ready_fn;
inline ui_icon_provider_t g_ui_icon_provider;

inline void ui_set_key_reader(UiKeyBitsFn bits_fn, UiKeyReadyFn ready_fn) {
	g_ui_key_bits_fn = std::move(bits_fn);
	g_ui_key_ready_fn = std::move(ready_fn);
}

inline void ui_set_icon_provider(const ui_icon_provider_t& provider) {
	g_ui_icon_provider = provider;
}

inline void ui_gfx_image(int64 tex, vec2 pos, vec2 size, color tint) {
	if (fgui::render && tex > 0)
		fgui::render->draw_image(static_cast<fgui::image_handle>(tex), ui_gfx_point(pos), ui_gfx_point(size), ui_gfx_color(tint));
}

inline void ui_gfx_image_uv(int64 tex, vec2 pos, vec2 size, vec2 uv0, vec2 uv1, color tint) {
	if (fgui::render && tex > 0) {
		fgui::render->draw_image_uv(static_cast<fgui::image_handle>(tex), ui_gfx_point(pos), ui_gfx_point(size),
			DirectX::XMFLOAT4(static_cast<float>(uv0.x), static_cast<float>(uv0.y),
				static_cast<float>(uv1.x), static_cast<float>(uv1.y)), ui_gfx_color(tint));
	}
}

inline int64 ui_gfx_texture(const std::string& path, float64& aspect) {
	aspect = 1.0;
	if (!g_ui_icon_provider.tex_fn)
		return 0;
	const int64 tex = g_ui_icon_provider.tex_fn(path);
	if (tex != 0 && g_ui_icon_provider.aspect_fn)
		aspect = g_ui_icon_provider.aspect_fn(path);
	return tex;
}

inline bool g_ui_provider_has_texture(const std::string& path) {
	return g_ui_icon_provider.tex_fn && g_ui_icon_provider.tex_fn(path) != 0;
}

inline bool ui_gfx_texture_settled(const std::string& path) {
	return g_ui_icon_provider.settled_fn && g_ui_icon_provider.settled_fn(path);
}

inline void ui_gfx_texture_fit(int64 tex, float64 aspect, float64 x, float64 y, float64 size, color tint) {
	if (tex == 0)
		return;
	if (g_ui_icon_provider.draw_tex_fn) {
		g_ui_icon_provider.draw_tex_fn(tex, aspect, x, y, size, tint);
		return;
	}
	float64 width = size;
	float64 height = size * aspect;
	if (height > size) {
		height = size;
		width = size / aspect;
	}
	ui_gfx_image(tex, vec2(x + (size - width) * 0.5, y + (size - height) * 0.5), vec2(width, height), tint);
}

inline bool ui_gfx_icon_path(const std::string& path, float64 x, float64 y, float64 size) {
	return g_ui_icon_provider.draw_path_fn && g_ui_icon_provider.draw_path_fn(path, x, y, size);
}

inline bool ui_gfx_icon_key(const std::string& key, float64 x, float64 y, float64 size) {
	return g_ui_icon_provider.draw_key_fn && g_ui_icon_provider.draw_key_fn(key, x, y, size);
}

inline bool ui_gfx_icon_ready(const std::string& icon) {
	if (icon.empty() || !g_ui_icon_provider.tex_fn)
		return false;
	std::string path = icon;
	if (path.find(".png") == std::string::npos) {
		if (!g_ui_icon_provider.lookup_fn)
			return false;
		path = g_ui_icon_provider.lookup_fn(icon);
		if (path.empty())
			return false;
		if (path.find(".png") == std::string::npos) {
			if (!g_ui_icon_provider.path_fn)
				return false;
			path = g_ui_icon_provider.path_fn(path);
		}
	}
	return g_ui_provider_has_texture(path);
}

inline bool ui_gfx_icon_loading(const std::string& key) {
	if (!g_ui_icon_provider.lookup_fn)
		return false;
	std::string path = g_ui_icon_provider.lookup_fn(key);
	if (path.empty())
		return false;
	if (path.find(".png") == std::string::npos && g_ui_icon_provider.path_fn)
		path = g_ui_icon_provider.path_fn(path);
	return !ui_gfx_texture_settled(path);
}

inline vec2 ui_in_mouse() {
	if (!fgui::process)
		return vec2();
	return vec2(fgui::process->input.mouse_pos.x, fgui::process->input.mouse_pos.y);
}

inline bool ui_in_key_bits(int64 vk_code, uint8& bits) {
	if (g_ui_key_bits_fn && g_ui_key_bits_fn(vk_code, bits))
		return true;
	bits = (fgui::process && vk_code >= 0 && vk_code < 256 &&
		fgui::process->input.key_down[static_cast<size_t>(vk_code)]) ? UI_KEY_DOWN : 0;
	return true;
}

inline bool ui_in_key_reader() {
	return g_ui_key_ready_fn && g_ui_key_ready_fn();
}

inline bool ui_in_key_down(int64 vk_code) {
	return fgui::process && vk_code >= 0 && vk_code < 256 &&
		fgui::process->input.key_down[static_cast<size_t>(vk_code)];
}

inline std::vector<int32> ui_in_keys_down() {
	std::vector<int32> keys;
	if (!fgui::process)
		return keys;
	for (int32 key = 0; key < 256; ++key) {
		if (fgui::process->input.key_down[key])
			keys.push_back(key);
	}
	return keys;
}

inline std::string ui_in_key_name(int64 vk_code) {
	if (vk_code < 0 || vk_code > 255)
		return {};
	const UINT scan = MapVirtualKeyA(static_cast<UINT>(vk_code), MAPVK_VK_TO_VSC);
	char name[64]{};
	const LONG key_data = static_cast<LONG>(scan << 16);
	const int length = GetKeyNameTextA(key_data, name, static_cast<int>(sizeof(name)));
	return length > 0 ? std::string(name, static_cast<size_t>(length)) : std::string();
}

inline bool ui_in_key_bits_reader(int64 vk_code, uint8& bits) {
	return g_ui_key_bits_fn && g_ui_key_bits_fn(vk_code, bits);
}

inline bool ui_in_key_down_api(int64 vk_code) {
	return ui_in_key_down(vk_code);
}

inline bool ui_in_key_fired(int64 vk_code) {
	return fgui::process && vk_code >= 0 && vk_code < 256 &&
		fgui::process->input.key_pressed[static_cast<size_t>(vk_code)];
}

inline float64 ui_in_wheel() {
	if (!fgui::process)
		return 0.0;
	const float64 wheel = static_cast<float64>(fgui::process->input.scroll_delta);
	fgui::process->input.scroll_delta = 0;
	return wheel;
}

inline int64 ui_sys_ms() {
	return static_cast<int64>(GetTickCount64());
}

inline int64 ui_sys_perf_time() {
	LARGE_INTEGER value{};
	QueryPerformanceCounter(&value);
	return static_cast<int64>(value.QuadPart);
}

inline int64 ui_sys_perf_freq() {
	LARGE_INTEGER value{};
	QueryPerformanceFrequency(&value);
	return static_cast<int64>(value.QuadPart);
}

inline std::string ui_sys_clock_or(const std::string& fallback) {
	std::time_t now = std::time(nullptr);
	std::tm local{};
#if defined(_WIN32)
	if (localtime_s(&local, &now) != 0)
#else
	if (localtime_r(&now, &local) == nullptr)
#endif
		return fallback;
	std::ostringstream out;
	out << std::setfill('0') << std::setw(2) << local.tm_hour << ':'
		<< std::setw(2) << local.tm_min << ':' << std::setw(2) << local.tm_sec;
	return out.str();
}

inline bool ui_sys_file_write(const std::string& path, const std::string& text) {
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file)
		return false;
	file.write(text.data(), static_cast<std::streamsize>(text.size()));
	return file.good();
}

inline bool ui_sys_file_append(const std::string& path, const std::string& text) {
	std::ofstream file(path, std::ios::binary | std::ios::app);
	if (!file)
		return false;
	file.write(text.data(), static_cast<std::streamsize>(text.size()));
	return file.good();
}

inline std::string ui_sys_file_read(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return {};
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

inline bool ui_sys_file_exists(const std::string& path) {
	return std::filesystem::exists(path);
}

inline bool ui_sys_parse_int(const std::string& text, int64& out) {
	if (text.empty())
		return false;
	size_t parsed = 0;
	try {
		const long long value = std::stoll(text, &parsed, 10);
		if (parsed != text.size())
			return false;
		out = static_cast<int64>(value);
		return true;
	} catch (const std::invalid_argument&) {
		return false;
	} catch (const std::out_of_range&) {
		return false;
	}
}

inline bool ui_sys_parse_float(const std::string& text, float64& out) {
	if (text.empty())
		return false;
	size_t parsed = 0;
	try {
		const double value = std::stod(text, &parsed);
		if (parsed != text.size() || !std::isfinite(value))
			return false;
		out = value;
		return true;
	} catch (const std::invalid_argument&) {
		return false;
	} catch (const std::out_of_range&) {
		return false;
	}
}

inline void ui_sys_log(const std::string& line) {
	std::clog << line << std::endl;
}

inline void println(const std::string& line) {
	ui_sys_log(line);
}

}
