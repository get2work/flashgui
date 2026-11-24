inline int32 ui_id() {
    int32 id = g_ui_next_id;
    g_ui_next_id = g_ui_next_id + 1;
    return id;
}

inline bool ui_mouse_in(float64 x, float64 y, float64 w, float64 h) {
    return g_ui_mouse.x >= x && g_ui_mouse.x < x + w && g_ui_mouse.y >= y && g_ui_mouse.y < y + h;
}

inline bool ui_hovered(float64 x, float64 y, float64 w, float64 h) {
    if (g_ui_pop.live && ui_mouse_in(g_ui_pop.x, g_ui_pop.y, g_ui_pop.w, g_ui_pop.h))
        return false;
    if (g_ui_pk.live && !g_ui_pk.drawing && ui_mouse_in(g_ui_pk.x, g_ui_pk.y, g_ui_pk.w, g_ui_pk.h))
        return false;
    if (g_ui_body.live && !ui_mouse_in(g_ui_body.x, g_ui_body.y, g_ui_body.w, g_ui_body.h))
        return false;
    if (g_ui_in_group && !ui_mouse_in(g_ui_clip_x, g_ui_clip_y, g_ui_clip_w, g_ui_clip_h))
        return false;
    return ui_mouse_in(x, y, w, h);
}

inline bool ui_hovered_in(float64 rx, float64 ry, float64 rw, float64 rh, float64 x, float64 y, float64 w, float64 h) {
    return ui_mouse_in(rx, ry, rw, rh) && ui_hovered(x, y, w, h);
}

inline bool ui_lit(bool hot, int32 id) {
    return hot && (g_ui_active == 0 || g_ui_active == id);
}

inline bool ui_take_press(int32 id, bool hot) {
    if (!hot || !g_ui_pressed)
        return false;
    g_ui_pressed = false;
    g_ui_active = id;
    return true;
}

inline bool ui_button_behavior(int32 id, bool hot) {
    ui_take_press(id, hot);
    return g_ui_active == id && g_ui_released && hot;
}

inline void ui_reset_press(bool down, bool rdown) {
    g_ui_prev_down = down;
    g_ui_prev_rdown = rdown;
    g_ui_active = down ? UI_ID_NOWHERE : 0;
    g_ui_pop.press = -1;
    // The window was not drawn meanwhile: what its popup or picker was open on may have changed.
    g_ui_pop.label = "";
    g_ui_pk.press = false;
    g_ui_pk.open = false;
}

inline float64 ui_clamp(float64 v, float64 lo, float64 hi) {
    return min(max(v, lo), hi);
}

inline float64 ui_snap(float64 v) {
    return static_cast<float64>(static_cast<int64>(v + 0.5));
}

inline float64 ui_content_x() { return g_ui_area_x; }

inline float64 ui_content_w() { return g_ui_area_w; }

inline void ui_area_body() {
    g_ui_area_x = g_ui_body.x;
    g_ui_area_w = g_ui_body.w;
}

inline void ui_need(float64 w) {
    g_ui_need_w = max(g_ui_need_w, w);
}

inline float64 ui_row_begin(float64 h) {
    float64 y = g_ui_cursor;
    g_ui_cursor = y + h;
    return y;
}

inline void ui_row_mark(float64 x, float64 y, float64 h, float64 label_end, float64 limit) {
    g_ui_row.x = x;
    g_ui_row.y = y;
    g_ui_row.h = h;
    g_ui_row.label_end = label_end;
    g_ui_row.limit = limit;
}

inline float64 ui_max_text_w(const std::vector<std::string>& texts) {
    float64 w = 0.0;
    for (const std::string& t : texts)
        w = max(w, ui_gfx_text_size(g_ui_font, t).x);
    return w;
}

inline void ui_text_at(const std::string& text, float64 x, float64 row_y, float64 row_h, color c, int64 font) {
    vec2 ts = ui_gfx_text_size(font, text);
    ui_gfx_text(text, vec2(x, row_y + (row_h - ts.y) * 0.5), c, font);
}

inline void ui_text_center(const std::string& text, float64 x, float64 y, float64 w, float64 h, color c, int64 font) {
    vec2 ts = ui_gfx_text_size(font, text);
    ui_gfx_text(text, vec2(ui_snap(x + (w - ts.x) * 0.5), y + (h - ts.y) * 0.5), c, font);
}

inline std::string ui_fit_text_with(int64 font, const std::string& text, float64 max_w, const std::string& mark) {
    if (ui_gfx_text_size(font, text).x <= max_w)
        return text;
    float64 dots = ui_gfx_text_size(font, mark).x;
    int32 fit = ui_gfx_text_fit(font, max(max_w - dots, 0.0), text);
    return text.substr(0, fit > 0 ? static_cast<uint64>(fit) : 0) + mark;
}

inline std::string ui_fit_text_in(int64 font, const std::string& text, float64 max_w) {
    return ui_fit_text_with(font, text, max_w, "..");
}

inline std::string ui_fit_text(const std::string& text, float64 max_w) {
    return ui_fit_text_in(g_ui_font, text, max_w);
}

inline void ui_wrap_lines(const std::string& text, float64 w, int64 font, std::vector<std::string>& out) {
    out.clear();
    std::string rest = text;
    while (rest.size() != 0) {
        uint64 brk = rest.find("\n");
        uint64 bar = rest.find(" | ");
        uint64 cut = brk < bar ? brk : bar;
        uint64 skip = brk < bar ? 1 : 3;
        std::string seg = cut != std::string::npos ? rest.substr(0, cut) : rest;
        rest = cut != std::string::npos ? rest.substr(cut + skip) : "";
        if (seg.size() != 0)
            ui_wrap_line(seg, w, font, out);
    }
}

inline void ui_wrap_line(const std::string& line, float64 w, int64 font, std::vector<std::string>& out) {
    if (line.size() == 0) {
        out.push_back("");
        return;
    }
    std::string seg = line;
    while (seg.size() != 0) {
        int32 fit = ui_gfx_text_fit(font, w, seg);
        uint64 n = fit > 0 ? static_cast<uint64>(fit) : 1;
        if (n < seg.size()) {
            uint64 sp = seg.substr(0, n + 1).rfind(" ");
            if (sp != std::string::npos && sp > 0)
                n = sp;
        }
        out.push_back(seg.substr(0, n));
        while (n < seg.size() && seg.substr(n, 1) == " ")
            n = n + 1;
        seg = seg.substr(n);
    }
}

inline void ui_draw_box(float64 x, float64 y, float64 w, float64 h, color bg, color border) {
    ui_gfx_fill(vec2(x, y), vec2(w, h), bg, g_ui_m.round);
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), border, 1.0, g_ui_m.round);
}

inline void ui_draw_field(float64 x, float64 y, float64 w, float64 h, bool lit, bool focus) {
    color border = focus ? ui_col_accent() : (lit ? g_ui_theme.border_hi : g_ui_theme.border);
    ui_draw_box(x, y, w, h, lit ? g_ui_theme.hover : g_ui_theme.bg2, border);
}

inline void ui_draw_check(float64 x, float64 y, float64 s, bool on, bool lit) {
    if (on) {
        color a = ui_col_accent();
        float64 o = g_ui_style.check_ring;
        if (lit)
            ui_gfx_frame(vec2(x - o, y - o), vec2(s + 2.0 * o, s + 2.0 * o), ui_fade(a, g_ui_theme.check_ring_a), 1.0,
                g_ui_m.round + o);
        ui_gfx_fill(vec2(x, y), vec2(s, s), a, g_ui_m.round);
        float64 t = max(ui_stroke_w() * g_ui_style.check_stroke, g_ui_style.check_stroke_min);
        const std::vector<float64>& m = g_ui_style.check_mark;
        vec2 p0 = vec2(x + s * m[0], y + s * m[1]);
        vec2 p1 = vec2(x + s * m[2], y + s * m[3]);
        vec2 p2 = vec2(x + s * m[4], y + s * m[5]);
        ui_gfx_line(p0, p1, g_ui_theme.bg, t);
        ui_gfx_line(p1, p2, g_ui_theme.bg, t);
        // Two butted lines leave a notch at the elbow.
        ui_gfx_disc(p1, t * 0.5, g_ui_theme.bg);
        return;
    }
    ui_draw_box(x, y, s, s, lit ? g_ui_theme.hover : g_ui_theme.bg2, lit ? g_ui_theme.border_hi : g_ui_theme.border);
}

inline void ui_row_hover(float64 x, float64 y, float64 w, float64 h, bool lit) {
    if (!lit)
        return;
    float64 e = ui_snap(g_ui_m.pad * 0.5);
    ui_gfx_fill(vec2(x - e, y), vec2(w + 2.0 * e, h), ui_fade(g_ui_theme.hover, g_ui_theme.row_hover_a), g_ui_m.round);
}

inline void ui_draw_chevron(float64 cx, float64 cy, float64 s, bool up, color c) {
    float64 hs = s * 0.5;
    if (up)
        ui_gfx_triangle(vec2(cx - s, cy + hs), vec2(cx + s, cy + hs), vec2(cx, cy - hs), c);
    else
        ui_gfx_triangle(vec2(cx - s, cy - hs), vec2(cx + s, cy - hs), vec2(cx, cy + hs), c);
}

inline void ui_draw_cross(float64 cx, float64 cy, float64 s, color c, float64 t) {
    ui_gfx_line(vec2(cx - s, cy - s), vec2(cx + s, cy + s), c, t);
    ui_gfx_line(vec2(cx - s, cy + s), vec2(cx + s, cy - s), c, t);
}

inline void ui_glyph_quad(float64 x0, float64 y0, float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3, color c) {
    std::vector<float32> pts = { static_cast<float32>(x0), static_cast<float32>(y0), static_cast<float32>(x1),
        static_cast<float32>(y1), static_cast<float32>(x2), static_cast<float32>(y2), static_cast<float32>(x3),
        static_cast<float32>(y3) };
    ui_gfx_polygon(pts, c);
}

inline void ui_draw_glyph(int32 g, float64 cx, float64 cy, float64 s, color c) {
    float64 t = max(ui_stroke_w(), 1.0);
    if (g == UI_GLYPH_MAP) {
        // Folded in three: the outer panels solid, the middle one shaded, a thin gap at each fold.
        float64 pw = s * 2.0 / 3.0;
        float64 gap = max(s * 0.08, 0.6);
        float64 lo = s * 0.72;
        float64 x0 = cx - s;
        float64 x1 = cx - s + pw;
        float64 x2 = cx + s - pw;
        float64 x3 = cx + s;
        ui_glyph_quad(x0, cy - lo, x1 - gap, cy - s, x1 - gap, cy + lo, x0, cy + s, c);
        ui_glyph_quad(x1 + gap, cy - s, x2 - gap, cy - lo, x2 - gap, cy + s, x1 + gap, cy + lo, ui_fade(c, 0.55));
        ui_glyph_quad(x2 + gap, cy - lo, x3, cy - s, x3, cy + lo, x2 + gap, cy + s, c);
    } else if (g == UI_GLYPH_BACK) {
        vec2 tip = vec2(cx - s * 0.7, cy);
        ui_gfx_line(vec2(cx + s * 0.75, cy), tip, c, t);
        ui_gfx_line(tip, vec2(cx - s * 0.05, cy - s * 0.65), c, t);
        ui_gfx_line(tip, vec2(cx - s * 0.05, cy + s * 0.65), c, t);
        ui_gfx_disc(tip, t * 0.5, c);
    } else if (g == UI_GLYPH_PLUS || g == UI_GLYPH_MINUS) {
        ui_gfx_line(vec2(cx - s * 0.7, cy), vec2(cx + s * 0.7, cy), c, t);
        if (g == UI_GLYPH_PLUS)
            ui_gfx_line(vec2(cx, cy - s * 0.7), vec2(cx, cy + s * 0.7), c, t);
    } else if (g == UI_GLYPH_TARGET) {
        float64 r = s * 0.55;
        ui_gfx_ring(vec2(cx, cy), r, c, t);
        ui_gfx_line(vec2(cx - s, cy), vec2(cx - r, cy), c, t);
        ui_gfx_line(vec2(cx + r, cy), vec2(cx + s, cy), c, t);
        ui_gfx_line(vec2(cx, cy - s), vec2(cx, cy - r), c, t);
        ui_gfx_line(vec2(cx, cy + r), vec2(cx, cy + s), c, t);
        ui_gfx_disc(vec2(cx, cy), max(t, s * 0.16), c);
    } else if (g == UI_GLYPH_PANEL) {
        // A window with its side panel on the right.
        float64 h = s * 0.8;
        ui_gfx_frame(vec2(cx - s + 0.5, cy - h + 0.5), vec2(2.0 * s - 1.0, 2.0 * h - 1.0), c, t, 1.5);
        float64 px = ui_snap(cx + s * 0.25);
        ui_gfx_fill(vec2(px, cy - h), vec2(cx + s - px, 2.0 * h), ui_fade(c, 0.55), 0.0);
    } else if (g == UI_GLYPH_CLOSE) {
        ui_draw_cross(cx, cy, s * 0.6, c, t);
    }
}

inline float64 ui_border_round() {
    return ui_snap(g_ui_m.round);
}

inline void ui_border_fill(float64 x, float64 y, float64 w, float64 h, color c, int32 corners) {
    ui_gfx_fill_corners(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), c, ui_border_round(), corners);
}

inline void ui_gradient_border(float64 x, float64 y, float64 w, float64 h) {
    float64 r = ui_border_round();
    float64 cw = min(r + 1.0, ui_snap(w * 0.5));
    vec2 op = vec2(x + 0.5, y + 0.5);
    vec2 os = vec2(w - 1.0, h - 1.0);
    ui_gfx_clip_push(vec2(x, y), vec2(cw, h));
    ui_gfx_frame(op, os, g_ui_theme.gradient[0], 1.0, r);
    ui_gfx_clip_pop();
    ui_gfx_clip_push(vec2(x + w - cw, y), vec2(cw, h));
    ui_gfx_frame(op, os, g_ui_theme.gradient[g_ui_theme.gradient.size() - 1], 1.0, r);
    ui_gfx_clip_pop();
    ui_gfx_gradient(vec2(x + cw, y), vec2(w - 2.0 * cw, 1.0), g_ui_theme.gradient, 0.0, true);
    ui_gfx_gradient(vec2(x + cw, y + h - 1.0), vec2(w - 2.0 * cw, 1.0), g_ui_theme.gradient, 0.0, true);
}

inline void ui_scroll_thumb(float64 gx, float64 gw, float64 ty, float64 th, bool lit, color idle, color hot) {
    float64 bw = max(ui_snap(g_ui_m.scroll_w), g_ui_style.bar_min);
    float64 tw = lit ? bw + g_ui_style.thumb_grow : bw;
    ui_gfx_fill(vec2(ui_snap(gx + (gw - tw) * 0.5), ui_snap(ty)), vec2(tw, ui_snap(th)), lit ? hot : idle, tw * 0.5);
}

inline void ui_icon_next(const std::string& key) {
    g_ui_icon.on = true;
    g_ui_icon.key = key;
    g_ui_icon.path = "";
}

inline void ui_icon_next_path(const std::string& path) {
    g_ui_icon.on = true;
    g_ui_icon.key = "";
    g_ui_icon.path = path;
}

inline float64 ui_item_icon(float64 h) {
    return ui_snap(h * g_ui_style.item_icon);
}

inline float64 ui_icon_w() {
    return g_ui_icon.on ? ui_snap(g_ui_m.icon) + ui_snap(g_ui_m.pad * 0.5) : 0.0;
}

inline bool ui_icon_any(const std::string& icon, float64 x, float64 y, float64 size) {
    if (icon.size() == 0)
        return false;
    if (icon.find(".png") != std::string::npos)
        return ui_gfx_icon_path(icon, x, y, size);
    return ui_gfx_icon_key(icon, x, y, size);
}

inline float64 ui_icon_take(float64 x, float64 y, float64 h) {
    if (!g_ui_icon.on)
        return x;
    float64 s = ui_snap(g_ui_m.icon);
    float64 w = ui_icon_w();
    g_ui_icon.on = false;
    if (g_ui_icon.path.size() != 0)
        ui_gfx_icon_path(g_ui_icon.path, x, ui_snap(y + (h - s) * 0.5), s);
    else if (g_ui_icon.key.size() != 0)
        ui_gfx_icon_key(g_ui_icon.key, x, ui_snap(y + (h - s) * 0.5), s);
    return x + w;
}
