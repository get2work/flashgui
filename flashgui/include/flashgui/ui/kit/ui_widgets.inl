inline void ui_label(const std::string& label, float64 x, float64 y, float64 h, float64 max_w) {
    float64 ix = ui_icon_take(x, y, h);
    max_w = max(max_w - (ix - x), 0.0);
    x = ix;
    float64 lw = ui_gfx_text_size(g_ui_font, label).x;
    if (lw <= max_w) {
        ui_text_at(label, x, y, h, g_ui_theme.text, g_ui_font);
    } else {
        ui_text_at(ui_fit_text(label, max_w), x, y, h, g_ui_theme.text, g_ui_font);
        lw = max_w;
        // A ui_tip after the row replaces this with its own text.
        if (g_ui_active == 0 && ui_hovered(x, y, max_w, h))
            g_ui_tip.text = label;
    }
    ui_row_mark(x, y, h, x + lw, x + max_w);
}

inline float64 ui_label_line(const std::string& label, float64 x, float64 y, float64 w) {
    float64 lh = ui_snap(g_ui_m.row_h * g_ui_style.text_line);
    ui_label(label, x, y, lh, w);
    return y + lh;
}

inline void ui_tip(const std::string& text) {
    float64 r = g_ui_m.tip_r;
    float64 cx = ui_snap(g_ui_row.label_end + g_ui_m.pad * 0.5 + r);
    float64 cy = ui_snap(g_ui_row.y + g_ui_row.h * 0.5);
    bool fits = cx + r <= g_ui_row.limit;
    float64 right = fits ? cx + r : g_ui_row.label_end;
    bool hot = g_ui_active == 0 && ui_hovered(g_ui_row.x, g_ui_row.y, right - g_ui_row.x, g_ui_row.h);
    if (fits) {
        color c = hot ? g_ui_theme.text : g_ui_theme.muted;
        if (hot)
            ui_gfx_disc(vec2(cx, cy), r, g_ui_theme.hover);
        ui_gfx_ring(vec2(cx, cy), r, hot ? g_ui_theme.dim : g_ui_theme.muted, 1.0);
        ui_text_center("?", cx - r, cy - r, 2.0 * r, 2.0 * r, c, g_ui_font_small);
    }
    if (hot)
        g_ui_tip.text = text;
}

inline bool ui_badge(const std::string& text, color c) {
    vec2 ts = ui_gfx_text_size(g_ui_font_small, text);
    float64 h = ui_snap(ts.y + g_ui_style.badge_pad_y);
    float64 w = ui_snap(ts.x + h * g_ui_style.badge_pad_x);
    float64 x = ui_snap(g_ui_row.label_end + g_ui_m.pad * 0.5);
    ui_need(ui_content_w() + x + w - g_ui_row.limit);
    if (x + w > g_ui_row.limit)
        return false;
    float64 y = ui_snap(g_ui_row.y + (g_ui_row.h - h) * 0.5);
    ui_gfx_fill(vec2(x, y), vec2(w, h), ui_fade(c, g_ui_theme.badge_fill_a), h * 0.5);
    color edge = ui_fade(c, g_ui_theme.badge_edge_a);
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), edge, 1.0, h * 0.5 - 0.5);
    ui_text_center(text, x, y, w, h, c, g_ui_font_small);
    g_ui_row.label_end = x + w;
    return true;
}

inline void ui_subheader(const std::string& text) {
    float64 h = ui_snap(g_ui_m.row_h * g_ui_style.text_line);
    float64 y = ui_row_begin(h);
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_content_w();
    float64 tw = ui_gfx_text_size(g_ui_font_small, text).x;
    ui_text_at(text, x, y, h, g_ui_theme.muted, g_ui_font_small);
    float64 lx = ui_snap(x + tw + g_ui_m.pad * 0.5);
    if (lx < x + w)
        ui_gfx_fill(vec2(lx, ui_snap(y + h * 0.5)), vec2(x + w - lx, 1.0), g_ui_theme.border, 0.0);
    ui_need(tw + g_ui_m.pad);
}

inline void ui_status_colored(const std::string& text, color c) {
    float64 h = ui_snap(g_ui_m.row_h * g_ui_style.text_line);
    float64 y = ui_row_begin(h);
    float64 x0 = ui_snap(ui_content_x());
    float64 x = ui_icon_take(x0, y, h);
    float64 w = max(ui_content_w() - (x - x0), 0.0);
    uint64 at = text.find(" | ");
    bool more = at != std::string::npos;
    std::string first = more ? text.substr(0, at) : text;
    float64 fw = ui_gfx_text_size(g_ui_font, first).x;
    bool cut = fw > w;
    ui_text_at(cut ? ui_fit_text(first, w) : first, x, y, h, c, g_ui_font);
    ui_row_mark(x, y, h, x + min(fw, w), x + w);
    if ((more || cut) && g_ui_active == 0 && ui_hovered(x, y, min(fw, w), h))
        g_ui_tip.text = text;
}

inline void ui_status(const std::string& text) {
    ui_status_colored(text, g_ui_theme.dim);
}

inline void ui_heading(const std::string& text) {
    float64 h = ui_snap(g_ui_m.row_h);
    float64 y = ui_row_begin(h);
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_content_w();
    ui_text_at(ui_fit_text_in(g_ui_font_bold, text, w), x, y, h, g_ui_theme.text, g_ui_font_bold);
    ui_row_mark(x, y, h, x + min(ui_gfx_text_size(g_ui_font_bold, text).x, w), x + w);
}

inline void ui_separator() {
    float64 y = ui_snap(g_ui_cursor + g_ui_m.pad * 0.5);
    ui_gfx_fill(vec2(ui_content_x(), y), vec2(ui_content_w(), 1.0), g_ui_theme.border, 0.0);
    g_ui_cursor = g_ui_cursor + g_ui_m.pad + 1.0;
}

inline void ui_text_colored(const std::string& text, color c) {
    uint64 slot = g_ui_wrap_n;
    g_ui_wrap_n = g_ui_wrap_n + 1;
    if (g_ui_wraps.size() <= slot)
        g_ui_wraps.resize(slot + 1);
    float64 w = ui_content_w();
    if (g_ui_wraps[slot].text != text || g_ui_wraps[slot].w != w || g_ui_wraps[slot].font != g_ui_font) {
        g_ui_wraps[slot].text = text;
        g_ui_wraps[slot].w = w;
        g_ui_wraps[slot].font = g_ui_font;
        ui_wrap_lines(text, w, g_ui_font, g_ui_wraps[slot].lines);
    }
    for (const std::string& line : g_ui_wraps[slot].lines) {
        ui_gfx_text(line, vec2(ui_content_x(), g_ui_cursor + g_ui_m.line_gap), c, g_ui_font);
        g_ui_cursor = g_ui_cursor + g_ui_font_h + g_ui_m.line_gap * 2.0;
    }
}

inline void ui_hint(const std::string& text) {
    ui_text_colored(text, g_ui_theme.muted);
}

inline void ui_progress_bar(float64 x, float64 y, float64 w, float64 h, float64 frac) {
    float64 f = ui_clamp(frac, 0.0, 1.0);
    ui_gfx_fill(vec2(x, y), vec2(w, h), g_ui_theme.bg2, h * 0.5);
    if (f > 0.0)
        ui_gfx_fill(vec2(x, y), vec2(ui_snap(max(w * f, h)), h), ui_col_accent(), h * 0.5);
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), g_ui_theme.border, 1.0, h * 0.5);
}

inline void ui_progress(const std::string& text, float64 frac) {
    ui_status(text);
    float64 h = ui_snap(max(g_ui_m.pad * g_ui_style.progress_h, g_ui_style.progress_min));
    float64 y = ui_row_begin(h + ui_snap(g_ui_m.pad * 0.5));
    ui_progress_bar(ui_snap(ui_content_x()), ui_snap(y), ui_snap(ui_content_w()), h, frac);
}

inline bool ui_toggle_note(const std::string& label, const std::string& note, bool& v) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    int32 id = ui_id();
    float64 bs = ui_snap(g_ui_m.box);
    float64 bx = x + w - bs;
    bool hot = ui_hovered(x, y, w, rh);
    bool changed = ui_button_behavior(id, hot);
    if (changed)
        v = !v;
    float64 nw = note.size() != 0 ? ui_snap(ui_gfx_text_size(g_ui_font, note).x) + g_ui_m.pad : 0.0;
    ui_need(ui_icon_w() + ui_gfx_text_size(g_ui_font, label).x + g_ui_m.pad + nw + bs);
    ui_row_hover(x, y, w, rh, ui_lit(hot, id));
    ui_label(label, x, y, rh, bx - g_ui_m.pad - nw - x);
    if (nw > 0.0)
        ui_text_at(note, bx - nw, y, rh, g_ui_theme.dim, g_ui_font);
    ui_draw_check(bx, ui_snap(y + (rh - bs) * 0.5), bs, v, ui_lit(hot, id));
    return changed;
}

inline bool ui_toggle(const std::string& label, bool& v) {
    return ui_toggle_note(label, "", v);
}

inline int32 ui_toggles(const std::vector<std::string>& labels, std::vector<bool>& values, const std::vector<std::string>& tips) {
    uint64 n = labels.size();
    values.resize(n);
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    if (n == 0)
        return -1;
    float64 pad = g_ui_m.pad;
    float64 gap = ui_snap(pad * g_ui_style.pair_gap);
    float64 nf = static_cast<float64>(n);
    float64 cw = ui_snap((w - gap * (nf - 1.0)) / nf);
    float64 bs = ui_snap(g_ui_m.box);
    ui_need(nf * (ui_max_text_w(labels) + pad + bs) + gap * (nf - 1.0));
    int32 hit = -1;
    for (uint64 i = 0; i < n; i++) {
        float64 cx = x + static_cast<float64>(i) * (cw + gap);
        float64 cell = i + 1 == n ? x + w - cx : cw;
        float64 bx = cx + cell - bs;
        int32 id = ui_id();
        bool hot = ui_hovered(cx, y, cell, rh);
        if (ui_button_behavior(id, hot)) {
            values[i] = !values[i];
            hit = static_cast<int32>(i);
        }
        bool lit = ui_lit(hot, id);
        ui_row_hover(cx, y, cell, rh, lit);
        ui_label(labels[i], cx, y, rh, bx - pad - cx);
        ui_draw_check(bx, ui_snap(y + (rh - bs) * 0.5), bs, values[i], lit);
        if (i < tips.size() && tips[i].size() != 0)
            ui_tip(tips[i]);
    }
    return hit;
}

inline float64 ui_on_grid(float64 v, float64 lo, float64 hi, float64 step) {
    float64 nv = ui_clamp(v, lo, hi);
    if (step > 0.0)
        nv = ui_clamp(lo + static_cast<float64>(static_cast<int64>((nv - lo) / step + 0.5)) * step, lo, hi);
    return nv;
}

inline int32 ui_step_decimals(float64 step) {
    if (step <= 0.0)
        return 2;
    float64 s = step;
    for (int32 d = 0; d < 3; d++) {
        if (std::abs(s - static_cast<float64>(static_cast<int64>(s + 0.5))) < 1e-6 * max(s, 1.0))
            return d;
        s = s * 10.0;
    }
    return 3;
}

inline std::string ui_format_value(float64 v, float64 step) {
    int32 d = ui_step_decimals(step);
    if (d == 0)
        return std::format("{:.0f}", v);
    if (d == 1)
        return std::format("{:.1f}", v);
    if (d == 2)
        return std::format("{:.2f}", v);
    return std::format("{:.3f}", v);
}

inline bool ui_slider_track(float64 x, float64 cy, float64 w, float64& v, float64 lo, float64 hi, float64 step, color fill) {
    int32 id = ui_id();
    float64 r = g_ui_m.knob_r;
    float64 hit_h = g_ui_m.frame_h;
    bool hot = ui_hovered(x - r, cy - hit_h * 0.5, w + 2.0 * r, hit_h);
    ui_take_press(id, hot);
    bool dragging = g_ui_active == id && (g_ui_down || g_ui_released);
    float64 nv = v;
    if (dragging) {
        nv = lo + ui_clamp((g_ui_mouse.x - x) / max(w, 1.0), 0.0, 1.0) * (hi - lo);
    } else if (hot && g_ui_active == 0 && g_ui_scroll != 0.0) {
        nv = v + (g_ui_scroll > 0.0 ? step : -step);
        g_ui_scroll = 0.0;
    }
    nv = ui_on_grid(nv, lo, hi, step);
    bool changed = nv != v;
    v = nv;

    float64 th = max(ui_snap(g_ui_m.track_h), g_ui_style.bar_min);
    float64 ty = ui_snap(cy - th * 0.5);
    bool lit = dragging || ui_lit(hot, id);
    ui_gfx_fill(vec2(x, ty), vec2(w, th), lit ? g_ui_theme.border_hi : g_ui_theme.border, th * 0.5);
    float64 t = hi > lo ? (v - lo) / (hi - lo) : 0.0;
    float64 fx = ui_snap(x + w * t);
    // Fill: a darker shade of fill brightening into the knob. Too short for the rounded gradient, a plain bar.
    if (fx - x >= th) {
        if (g_ui_fill_grad.size() != 2)
            g_ui_fill_grad.resize(2);
        g_ui_fill_grad[0] = fill.lerp(g_ui_theme.bg2, g_ui_theme.fill_shade);
        g_ui_fill_grad[1] = fill;
        ui_gfx_gradient(vec2(x, ty), vec2(fx - x, th), g_ui_fill_grad, th * 0.5, true);
    } else if (fx > x + 0.5) {
        ui_gfx_fill(vec2(x, ty), vec2(fx - x, th), fill, th * 0.5);
    }
    // Knob: a dark rim so it reads over the fill, a soft halo of fill while dragged.
    vec2 kc = vec2(fx, ty + th * 0.5);
    float64 kr = lit ? r + g_ui_style.knob_grow : r;
    if (dragging)
        ui_gfx_disc(kc, kr + g_ui_style.knob_halo, ui_fade(fill, g_ui_theme.knob_halo_a));
    ui_gfx_disc(kc, kr + g_ui_style.knob_rim, g_ui_theme.bg);
    ui_gfx_disc(kc, kr, lit ? g_ui_theme.text : g_ui_theme.knob);
    return changed;
}

inline bool ui_slider(const std::string& label, float64& v, float64 lo, float64 hi, float64 step) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 lh = ui_snap(g_ui_m.row_h);
    float64 tl = ui_snap(g_ui_m.row_h * g_ui_style.slider_line);
    float64 y = ui_row_begin(lh + tl);
    float64 pad = g_ui_m.pad;
    float64 vw = ui_snap(g_ui_m.value_w);
    float64 vx = x + w - vw;
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (lh - fh) * 0.5);
    float64 r = g_ui_m.knob_r;
    ui_need(max(ui_icon_w() + ui_gfx_text_size(g_ui_font, label).x + pad + vw, g_ui_m.track_min_w + 2.0 * r));
    ui_label(label, x, y, lh, vx - pad - x);
    float64 cy = y + lh + tl * g_ui_style.slider_track_y;
    bool changed = ui_slider_track(x + r, cy, max(w - 2.0 * r, 1.0), v, lo, hi, step, ui_col_accent());
    return ui_value_field(label, v, lo, hi, step, vx, fy, vw, fh, true) || changed;
}

inline bool ui_number(const std::string& label, float64& v, float64 lo, float64 hi, float64 step) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (rh - fh) * 0.5);
    float64 nw = ui_snap(g_ui_m.number_w);
    float64 nx = x + w - nw;
    float64 bw = fh;
    int32 minus_id = ui_id();
    int32 plus_id = ui_id();
    bool minus_hot = ui_hovered(nx, fy, bw, fh);
    bool plus_hot = ui_hovered(nx + nw - bw, fy, bw, fh);
    bool box_hot = ui_hovered(nx, fy, nw, fh);
    float64 nv = v;
    if (ui_button_behavior(minus_id, minus_hot))
        nv = nv - step;
    if (ui_button_behavior(plus_id, plus_hot))
        nv = nv + step;
    if (box_hot && g_ui_active == 0 && g_ui_scroll != 0.0) {
        nv = nv + (g_ui_scroll > 0.0 ? step : -step);
        g_ui_scroll = 0.0;
    }
    nv = ui_on_grid(nv, lo, hi, step);
    bool changed = nv != v;
    v = nv;

    ui_need(ui_gfx_text_size(g_ui_font, label).x + g_ui_m.pad + nw);
    ui_label(label, x, y, rh, nx - g_ui_m.pad - x);
    ui_draw_box(nx, fy, nw, fh, g_ui_theme.bg2, box_hot ? g_ui_theme.border_hi : g_ui_theme.border);
    if (ui_lit(minus_hot, minus_id))
        ui_gfx_fill(vec2(nx + 1.0, fy + 1.0), vec2(bw - 1.0, fh - 2.0), g_ui_theme.hover, 0.0);
    if (ui_lit(plus_hot, plus_id))
        ui_gfx_fill(vec2(nx + nw - bw, fy + 1.0), vec2(bw - 1.0, fh - 2.0), g_ui_theme.hover, 0.0);
    ui_gfx_fill(vec2(nx + bw, fy), vec2(1.0, fh), g_ui_theme.border, 0.0);
    ui_gfx_fill(vec2(nx + nw - bw - 1.0, fy), vec2(1.0, fh), g_ui_theme.border, 0.0);
    float64 s = ui_snap(fh * g_ui_style.stepper_glyph);
    float64 cy = fy + ui_snap(fh * 0.5);
    float64 mx = nx + ui_snap(bw * 0.5);
    float64 px = nx + nw - bw + ui_snap(bw * 0.5);
    ui_gfx_fill(vec2(mx - s, cy - 0.5), vec2(2.0 * s, 1.0), v > lo ? g_ui_theme.text : g_ui_theme.muted, 0.0);
    color pc = v < hi ? g_ui_theme.text : g_ui_theme.muted;
    ui_gfx_fill(vec2(px - s, cy - 0.5), vec2(2.0 * s, 1.0), pc, 0.0);
    ui_gfx_fill(vec2(px - 0.5, cy - s), vec2(1.0, 2.0 * s), pc, 0.0);
    return ui_value_field(label, v, lo, hi, step, nx + bw + 1.0, fy, nw - 2.0 * bw - 2.0, fh, false) || changed;
}

inline bool ui_button(const std::string& label) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (rh - fh) * 0.5);
    int32 id = ui_id();
    bool hot = ui_hovered(x, fy, w, fh);
    bool clicked = ui_button_behavior(id, hot);
    bool held = g_ui_active == id && g_ui_down;
    bool lit = ui_lit(hot, id);
    color bg = held ? g_ui_theme.pressed : (lit ? g_ui_theme.hover : g_ui_theme.bg2);
    ui_draw_box(x, fy, w, fh, bg, lit ? ui_col_accent_hi() : ui_col_accent());
    float64 tw = ui_gfx_text_size(g_ui_font, label).x;
    ui_need(tw + 2.0 * g_ui_m.pad);
    ui_text_center(label, x, fy, w, fh, g_ui_theme.text, g_ui_font);
    ui_row_mark(x, y, rh, x + (w + tw) * 0.5, x + w);
    return clicked;
}

inline int32 ui_buttons(const std::vector<std::string>& labels) {
    uint64 n = labels.size();
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    if (n == 0)
        return -1;
    float64 gap = ui_snap(g_ui_m.pad * 0.5);
    float64 bw = ui_snap((w - gap * static_cast<float64>(n - 1)) / static_cast<float64>(n));
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (rh - fh) * 0.5);
    ui_need(static_cast<float64>(n) * (ui_max_text_w(labels) + 2.0 * g_ui_m.pad) + gap * static_cast<float64>(n - 1));
    int32 hit = -1;
    for (uint64 i = 0; i < n; i++) {
        float64 bx = x + static_cast<float64>(i) * (bw + gap);
        float64 cw = i + 1 == n ? x + w - bx : bw;
        if (ui_small_button(labels[i], bx, fy, cw, fh, true))
            hit = static_cast<int32>(i);
    }
    return hit;
}

inline bool ui_small_button(const std::string& label, float64 x, float64 y, float64 w, float64 h, bool enabled) {
    int32 id = ui_id();
    bool hot = enabled && ui_hovered(x, y, w, h);
    bool clicked = ui_button_behavior(id, hot);
    bool held = g_ui_active == id && g_ui_down;
    bool lit = ui_lit(hot, id);
    color bg = held ? g_ui_theme.pressed : (lit ? g_ui_theme.hover : g_ui_theme.bg2);
    ui_draw_box(x, y, w, h, bg, enabled ? (lit ? ui_col_accent_hi() : ui_col_accent()) : g_ui_theme.border);
    ui_text_center(label, x, y, w, h, enabled ? g_ui_theme.text : g_ui_theme.muted, g_ui_font);
    return clicked;
}

inline bool ui_text_button(const std::string& text, const std::string& button, bool enabled) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 pad = ui_snap(g_ui_m.pad);
    float64 bw = ui_snap(ui_gfx_text_size(g_ui_font, button).x + 2.0 * pad);
    float64 bx = x + w - bw;
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (rh - fh) * 0.5);
    bool clicked = ui_small_button(button, bx, fy, bw, fh, enabled);
    // The text is often a long entity key, cut with a tooltip by design: only a short stub of it is required.
    ui_need(ui_icon_w() + min(ui_gfx_text_size(g_ui_font, text).x, g_ui_m.stub_w) + pad + bw);
    ui_label(text, x, y, rh, bx - pad - x);
    return clicked;
}

inline void ui_text_box_wrap(ui_text_box_t& tb, const std::string& text, float64 tw) {
    int64 now = ui_sys_ms();
    if (tw != tb.want_w) {
        tb.want_w = tw;
        tb.want_at = now;
    }
    if (tb.text == text && tb.font == g_ui_font && (tb.w == tw || now - tb.want_at < UI_TEXT_REWRAP_MS))
        return;
    if (tb.text != text)
        tb.top = 0;
    tb.text = text;
    tb.w = tw;
    tb.font = g_ui_font;
    tb.lines.clear();
    uint64 at = 0;
    while (at < text.size()) {
        uint64 nl = text.find("\n", at);
        uint64 end = nl != std::string::npos ? nl : text.size();
        ui_wrap_line(text.substr(at, end - at), tw, g_ui_font, tb.lines);
        at = end + 1;
    }
}

inline int32 ui_text_box_scroll(int32 top, int32 max_top, bool over) {
    bool up = g_ui_scroll > 0.0;
    if (!over || g_ui_scroll == 0.0 || g_ui_active != 0 || (up ? top <= 0 : top >= max_top))
        return top;
    g_ui_scroll = 0.0;
    top = top + (up ? -UI_TEXT_SCROLL_LINES : UI_TEXT_SCROLL_LINES);
    return top < 0 ? 0 : (top > max_top ? max_top : top);
}

inline void ui_text_box(const std::string& id, const std::string& text, const std::string& hint, int32 rows) {
    int32 shown = rows > 1 ? rows : 1;
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 pad = ui_snap(g_ui_m.pad);
    float64 ip = ui_snap(pad * 0.5);
    float64 lh = ui_snap(g_ui_font_h + g_ui_m.line_gap * 2.0);
    float64 h = lh * static_cast<float64>(shown) + 2.0 * ip;
    float64 y = ui_snap(ui_row_begin(h + ip));
    if (g_ui_text_boxes.count(id) == 0) {
        ui_text_box_t fresh;
        g_ui_text_boxes[id] = fresh;
    }
    ui_text_box_t& tb = g_ui_text_boxes[id];
    // Wrapped clear of the scrollbar's gutter, so the bar showing up never rewraps it.
    ui_text_box_wrap(tb, text, max(w - ip - pad, 1.0));
    int32 n = static_cast<int32>(tb.lines.size());
    int32 max_top = n > shown ? n - shown : 0;
    int32 top = tb.top < 0 ? 0 : (tb.top > max_top ? max_top : tb.top);
    top = ui_text_box_scroll(top, max_top, ui_hovered(x, y, w, h));
    float64 bar_x = x + w - pad;
    float64 th = max(h * static_cast<float64>(shown) / static_cast<float64>(n > 0 ? n : 1), lh);
    float64 ty = max_top > 0 ? y + (h - th) * static_cast<float64>(top) / static_cast<float64>(max_top) : y;
    int32 bar_id = ui_id();
    bool bar_hot = max_top > 0 && ui_hovered(bar_x, y, pad, h);
    // A press off the thumb centres it there, then drags as usual.
    if (ui_take_press(bar_id, bar_hot))
        g_ui_drag_dy = g_ui_mouse.y >= ty && g_ui_mouse.y < ty + th ? g_ui_mouse.y - ty : th * 0.5;
    bool dragging = g_ui_active == bar_id && g_ui_down && max_top > 0;
    if (dragging) {
        float64 t = ui_clamp((g_ui_mouse.y - g_ui_drag_dy - y) / max(h - th, 1.0), 0.0, 1.0);
        top = static_cast<int32>(t * static_cast<float64>(max_top) + 0.5);
    }
    tb.top = top;

    ui_draw_box(x, y, w, h, g_ui_theme.bg2, g_ui_theme.border);
    // Clips do not intersect on their own: cut the box to the visible group (or body) by hand. The text stops at the
    // scrollbar's gutter, which a wrap kept through a resize (ui_text_box_wrap) can reach.
    float64 vx = g_ui_in_group ? g_ui_clip_x : g_ui_body.x;
    float64 vy = g_ui_in_group ? g_ui_clip_y : g_ui_body.y;
    float64 vr = g_ui_in_group ? g_ui_clip_x + g_ui_clip_w : g_ui_body.x + g_ui_body.w;
    float64 vb = g_ui_in_group ? g_ui_clip_y + g_ui_clip_h : g_ui_body.y + g_ui_body.h;
    float64 cx = max(x + 1.0, vx);
    float64 cy = max(y + 1.0, vy);
    ui_gfx_clip_push(vec2(cx, cy), vec2(max(min(bar_x, vr) - cx, 0.0), max(min(y + h - 1.0, vb) - cy, 0.0)));
    float64 ly = y + ip + ui_snap((lh - g_ui_font_h) * 0.5);
    if (n == 0)
        ui_gfx_text(hint, vec2(x + ip, ly), g_ui_theme.muted, g_ui_font);
    for (int32 s = 0; s < shown && top + s < n; s++)
        ui_gfx_text(tb.lines[static_cast<uint64>(top + s)], vec2(x + ip, ly + static_cast<float64>(s) * lh),
            g_ui_theme.text, g_ui_font);
    ui_gfx_clip_pop();
    if (max_top > 0) {
        float64 bw = max(ui_snap(g_ui_m.scroll_w), g_ui_style.bar_min);
        ui_gfx_fill(vec2(ui_snap(bar_x + (pad - bw) * 0.5), y + 1.0), vec2(bw, h - 2.0), g_ui_theme.border, bw * 0.5);
        ty = y + (h - th) * static_cast<float64>(top) / static_cast<float64>(max_top);
        bool lit = dragging || (bar_hot && g_ui_active == 0);
        ui_scroll_thumb(bar_x, pad, ty + 1.0, th - 2.0, lit, ui_fade(ui_col_accent(), g_ui_theme.thumb_idle_a),
            ui_col_accent());
    }
    ui_need(g_ui_m.stub_w + pad);
    ui_row_mark(x, y, h, x, x);
}
