inline bool ui_float_drag(float64& x, float64& y, float64 w, float64 h, bool interactive) {
    return ui_float_drag_at(0, x, y, w, h, interactive);
}

inline bool ui_float_drag_at(int32 slot, float64& x, float64& y, float64 w, float64 h, bool interactive) {
    while (g_ui_floats.size() < static_cast<uint64>(UI_FLOAT_SLOTS)) {
        ui_float_rect_t blank;
        g_ui_floats.push_back(blank);
    }
    int32 s = slot < 0 ? 0 : (slot >= UI_FLOAT_SLOTS ? UI_FLOAT_SLOTS - 1 : slot);
    ui_float_rect_t& fl = g_ui_floats[static_cast<uint64>(s)];
    fl.live = interactive;
    bool dragging = interactive && g_ui_active == UI_ID_FLOAT - s && g_ui_down;
    if (dragging) {
        vec2 view = ui_gfx_view();
        x = ui_snap(ui_clamp(g_ui_mouse.x - g_ui_drag_dx, 0.0, max(view.x - w, 0.0)));
        y = ui_snap(ui_clamp(g_ui_mouse.y - g_ui_drag_dy, 0.0, max(view.y - h, 0.0)));
    }
    fl.x = x;
    fl.y = y;
    fl.w = w;
    fl.h = h;
    return dragging;
}

inline void ui_hud_panel(float64 x, float64 y, float64 w, float64 h) {
    ui_hud_panel_faded(x, y, w, h, 1.0);
}

inline void ui_hud_panel_faded(float64 x, float64 y, float64 w, float64 h, float64 f) {
    // Drawn with the menu closed too, so the gradient cannot wait for ui_begin to follow the accent.
    ui_sync_accent();
    float64 r = ui_snap(g_ui_m.round);
    ui_gfx_fill(vec2(x, y), vec2(w, h), ui_fade(g_ui_theme.panel, f), r);
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), ui_fade(g_ui_theme.panel_edge, f), 1.0, r);
    if (f >= 1.0) {
        ui_gfx_gradient(vec2(x + r, y), vec2(max(w - 2.0 * r, 0.0), 1.0), g_ui_theme.gradient, 0.0, true);
        return;
    }
    g_ui_hud_grad.clear();
    for (const color& c : g_ui_theme.gradient)
        g_ui_hud_grad.push_back(ui_fade(c, f));
    ui_gfx_gradient(vec2(x + r, y), vec2(max(w - 2.0 * r, 0.0), 1.0), g_ui_hud_grad, 0.0, true);
}

inline void ui_wm_pt(float64 x, float64 y) {
    g_ui_wm_s.pts.push_back(static_cast<float32>(x));
    g_ui_wm_s.pts.push_back(static_cast<float32>(y));
}

inline void ui_wm_octagon(float64 x, float64 y, float64 w, float64 h, float64 cut, float64 d) {
    float64 k = cut + d * UI_TAN_22_5;
    g_ui_wm_s.pts.clear();
    g_ui_wm_s.pts.reserve(16);
    ui_wm_pt(x + k, y + d);
    ui_wm_pt(x + w - k, y + d);
    ui_wm_pt(x + w - d, y + k);
    ui_wm_pt(x + w - d, y + h - k);
    ui_wm_pt(x + w - k, y + h - d);
    ui_wm_pt(x + k, y + h - d);
    ui_wm_pt(x + d, y + h - k);
    ui_wm_pt(x + d, y + k);
}

inline void ui_wm_diamond(float64 cx, float64 cy, float64 rx, float64 ry) {
    g_ui_wm_s.pts.clear();
    g_ui_wm_s.pts.reserve(8);
    ui_wm_pt(cx, cy - ry);
    ui_wm_pt(cx + rx, cy);
    ui_wm_pt(cx, cy + ry);
    ui_wm_pt(cx - rx, cy);
}

inline vec2 ui_wm_ring_at(uint64 i, bool inner) {
    uint64 k = (inner ? 16 : 0) + 2 * (i % 8);
    return vec2(static_cast<float64>(g_ui_wm_s.ring[k]), static_cast<float64>(g_ui_wm_s.ring[k + 1]));
}

inline void ui_wm_ring_quad(std::vector<float32>& out, uint64 i) {
    uint64 a = 2 * (i % 8);
    uint64 b = 2 * ((i + 1) % 8);
    out.clear();
    out.reserve(8);
    out.push_back(g_ui_wm_s.ring[a]);
    out.push_back(g_ui_wm_s.ring[a + 1]);
    out.push_back(g_ui_wm_s.ring[b]);
    out.push_back(g_ui_wm_s.ring[b + 1]);
    out.push_back(g_ui_wm_s.ring[16 + b]);
    out.push_back(g_ui_wm_s.ring[16 + b + 1]);
    out.push_back(g_ui_wm_s.ring[16 + a]);
    out.push_back(g_ui_wm_s.ring[16 + a + 1]);
}

inline void ui_wm_border_build(float64 x, float64 y, float64 w, float64 h, float64 cut) {
    g_ui_wm_s.ring.clear();
    g_ui_wm_s.ring.reserve(32);
    ui_wm_octagon(x, y, w, h, cut, 0.0);
    for (uint64 i = 0; i < 16; i++)
        g_ui_wm_s.ring.push_back(g_ui_wm_s.pts[i]);
    ui_wm_octagon(x, y, w, h, cut, 1.0);
    g_ui_wm_s.fill.clear();
    g_ui_wm_s.fill.reserve(16);
    for (uint64 i = 0; i < 16; i++) {
        g_ui_wm_s.ring.push_back(g_ui_wm_s.pts[i]);
        g_ui_wm_s.fill.push_back(g_ui_wm_s.pts[i]);
    }
    ui_wm_ring_quad(g_ui_wm_s.q1, 1);
    ui_wm_ring_quad(g_ui_wm_s.q3, 3);
    ui_wm_ring_quad(g_ui_wm_s.q5, 5);
    ui_wm_ring_quad(g_ui_wm_s.q7, 7);
    vec2 tl = ui_wm_ring_at(0, false);
    vec2 tr = ui_wm_ring_at(1, false);
    vec2 rt = ui_wm_ring_at(2, false);
    vec2 rb = ui_wm_ring_at(3, false);
    vec2 br = ui_wm_ring_at(4, false);
    vec2 bl = ui_wm_ring_at(5, false);
    vec2 lb = ui_wm_ring_at(6, false);
    vec2 lt = ui_wm_ring_at(7, false);
    g_ui_wm_s.top_p = tl;
    g_ui_wm_s.top_s = vec2(tr.x - tl.x, 1.0);
    g_ui_wm_s.bot_p = vec2(bl.x, bl.y - 1.0);
    g_ui_wm_s.bot_s = vec2(br.x - bl.x, 1.0);
    // The right (last stop) and left (first stop) runs as one ui_gfx_fills_each batch.
    g_ui_wm_s.sides.clear();
    g_ui_wm_s.sides.reserve(8);
    g_ui_wm_s.sides.push_back(static_cast<float32>(rt.x - 1.0));
    g_ui_wm_s.sides.push_back(static_cast<float32>(rt.y));
    g_ui_wm_s.sides.push_back(static_cast<float32>(1.0));
    g_ui_wm_s.sides.push_back(static_cast<float32>(rb.y - rt.y));
    g_ui_wm_s.sides.push_back(static_cast<float32>(lt.x));
    g_ui_wm_s.sides.push_back(static_cast<float32>(lt.y));
    g_ui_wm_s.sides.push_back(static_cast<float32>(1.0));
    g_ui_wm_s.sides.push_back(static_cast<float32>(lb.y - lt.y));
}

inline void ui_wm_border_draw(float64 cut) {
    color first = g_ui_theme.gradient[0];
    color last = g_ui_theme.gradient[g_ui_theme.gradient.size() - 1];
    if (cut > 0.0) {
        ui_gfx_polygon(g_ui_wm_s.q1, last);
        ui_gfx_polygon(g_ui_wm_s.q3, last);
        ui_gfx_polygon(g_ui_wm_s.q5, first);
        ui_gfx_polygon(g_ui_wm_s.q7, first);
    }
    ui_gfx_gradient(g_ui_wm_s.top_p, g_ui_wm_s.top_s, g_ui_theme.gradient, 0.0, true);
    ui_gfx_gradient(g_ui_wm_s.bot_p, g_ui_wm_s.bot_s, g_ui_theme.gradient, 0.0, true);
    if (g_ui_wm_s.side_cols.size() != 2)
        g_ui_wm_s.side_cols.resize(2);
    g_ui_wm_s.side_cols[0] = last;
    g_ui_wm_s.side_cols[1] = first;
    ui_gfx_fills_each(g_ui_wm_s.sides, g_ui_wm_s.side_cols);
}

inline void ui_wm_sweep(float64 x, float64 y, float64 w, float64 h, float64 cut) {
    int64 phase = ui_sys_ms() % UI_WM_SWEEP_EVERY_MS;
    float64 run = w - 2.0 * cut;
    if (phase >= UI_WM_SWEEP_MS || run <= 0.0)
        return;
    float64 f = static_cast<float64>(phase) / static_cast<float64>(UI_WM_SWEEP_MS);
    f = f * f * (3.0 - 2.0 * f);
    float64 sw = ui_snap(g_ui_m.wm_sweep_w);
    float64 sx = x + cut - sw + (run + sw) * f;
    color accent = ui_col_accent();
    if (g_ui_wm_s.sweep.size() != 3) {
        g_ui_wm_s.sweep.clear();
        g_ui_wm_s.sweep.reserve(3);
        for (int32 i = 0; i < 3; i++)
            g_ui_wm_s.sweep.push_back(UI_CLEAR);
    }
    g_ui_wm_s.sweep[0] = accent.with_alpha(0);
    g_ui_wm_s.sweep[2] = accent.with_alpha(0);
    ui_gfx_clip_push(vec2(x + cut, y + h - 2.0), vec2(run, 2.0));
    // ui_gfx_gradient_line copies the stops, so the middle one is changed in place between the two strokes.
    g_ui_wm_s.sweep[1] = accent.lerp(g_ui_theme.text, g_ui_theme.wm_sweep_mix);
    ui_gfx_gradient_line(vec2(sx, y + h - 0.5), vec2(sx + sw, y + h - 0.5), g_ui_wm_s.sweep, 1.0);
    g_ui_wm_s.sweep[1] = accent.with_alpha(g_ui_theme.wm_sweep_a);
    ui_gfx_gradient_line(vec2(sx, y + h - 1.5), vec2(sx + sw, y + h - 1.5), g_ui_wm_s.sweep, 1.0);
    ui_gfx_clip_pop();
}

inline void ui_wm_glow(float64 x, float64 y, float64 w, float64 h, float64 cut) {
    // Where the fill's (1 px inset) octagon leaves its cuts.
    float64 k = cut + UI_TAN_22_5;
    float64 span = h - k - 1.0;
    if (span <= 0.0)
        return;
    float64 a0 = static_cast<float64>(g_ui_theme.head_a);
    float64 ak = a0 * (1.0 - (k - 1.0) / span);
    color top = g_ui_theme.card_top;
    top.a = g_ui_theme.head_a;
    color mid = g_ui_theme.card_top;
    mid.a = static_cast<uint8>(ak);
    color clear = g_ui_theme.card_top;
    clear.a = 0;
    if (k > 1.0) {
        ui_gfx_corners(vec2(x + k, y + 1.0), vec2(w - 2.0 * k, k - 1.0), top, top, mid, mid, 0.0);
        color corner = g_ui_theme.card_top;
        corner.a = static_cast<uint8>((a0 + ak) * 0.5);
        ui_gfx_triangle(vec2(x + k, y + 1.0), vec2(x + k, y + k), vec2(x + 1.0, y + k), corner);
        ui_gfx_triangle(vec2(x + w - k, y + 1.0), vec2(x + w - 1.0, y + k), vec2(x + w - k, y + k), corner);
    }
    ui_gfx_corners(vec2(x + 1.0, y + k), vec2(w - 2.0, h - 2.0 * k), mid, mid, clear, clear, 0.0);
}

inline bool ui_wm_sizes_valid(const std::vector<std::string>& parts) {
    if (g_ui_wm_s.font != g_ui_font_wm || g_ui_wm_s.pad != g_ui_m.pad || g_ui_wm_s.mh != g_ui_m.wm_h
        || g_ui_wm_s.msep != g_ui_m.wm_sep_r || g_ui_wm_s.mdot != g_ui_m.wm_dot_r || g_ui_wm_s.parts.size() != parts.size())
        return false;
    for (uint64 i = 0; i < parts.size(); i++) {
        if (g_ui_wm_s.parts[i] != parts[i])
            return false;
    }
    return true;
}

inline void ui_wm_measure(const std::vector<std::string>& parts, float64 base, float64 gap, float64 sep) {
    g_ui_wm_s.font = g_ui_font_wm;
    g_ui_wm_s.pad = g_ui_m.pad;
    g_ui_wm_s.mh = g_ui_m.wm_h;
    g_ui_wm_s.msep = g_ui_m.wm_sep_r;
    g_ui_wm_s.mdot = g_ui_m.wm_dot_r;
    g_ui_wm_s.parts.clear();
    g_ui_wm_s.pw.clear();
    g_ui_wm_s.ph.clear();
    float64 w = base;
    for (uint64 i = 0; i < parts.size(); i++) {
        vec2 ts = ui_gfx_text_size(g_ui_font_wm, parts[i]);
        g_ui_wm_s.parts.push_back(parts[i]);
        g_ui_wm_s.pw.push_back(ts.x);
        g_ui_wm_s.ph.push_back(ts.y);
        w += ts.x + (i == 0 ? 0.0 : 2.0 * gap + 2.0 * sep);
    }
    g_ui_wm_s.w = ui_snap(w);
    g_ui_wm_s.gw = -1.0;
}

inline void ui_wm_layout(float64 x, float64 y, float64 w, float64 h, float64 cut, float64 tx, float64 gap, float64 sep, float64 dot_r) {
    g_ui_wm_s.gx = x;
    g_ui_wm_s.gy = y;
    g_ui_wm_s.gw = w;
    g_ui_wm_s.gh = h;
    g_ui_wm_s.gcut = cut;
    ui_wm_border_build(x, y, w, h, cut);
    g_ui_wm_s.dot_x = tx + dot_r;
    tx += 2.0 * dot_r + gap;
    g_ui_wm_s.seps.clear();
    g_ui_wm_s.tx.clear();
    g_ui_wm_s.ty.clear();
    for (uint64 i = 0; i < g_ui_wm_s.pw.size(); i++) {
        if (i != 0) {
            g_ui_wm_s.seps.push_back(static_cast<float32>(tx + gap + sep));
            g_ui_wm_s.seps.push_back(static_cast<float32>(y + h * 0.5));
            g_ui_wm_s.seps.push_back(static_cast<float32>(sep));
            tx += 2.0 * gap + 2.0 * sep;
        }
        g_ui_wm_s.tx.push_back(ui_snap(tx));
        g_ui_wm_s.ty.push_back(y + (h - g_ui_wm_s.ph[i]) * 0.5);
        tx += g_ui_wm_s.pw[i];
    }
}

inline void ui_watermark(const std::vector<std::string>& parts, bool interactive) {
    ui_ensure_fonts();
    ui_sync_accent();
    float64 edge = ui_snap(g_ui_m.pad * g_ui_style.wm_edge);
    float64 gap = ui_snap(g_ui_m.pad * g_ui_style.wm_gap);
    float64 h = ui_snap(g_ui_m.wm_h);
    // The RT logo, padded the same above, below and to its left.
    float64 mark_h = ui_snap(h * g_ui_style.wm_rt);
    float64 mark_w = ui_rt_logo_w(mark_h);
    float64 inset = ui_snap((h - mark_h) * 0.5);
    float64 sep = max(g_ui_m.wm_sep_r, 1.0);
    float64 dot_r = max(g_ui_m.wm_dot_r, 1.0);
    if (!ui_wm_sizes_valid(parts))
        ui_wm_measure(parts, inset + mark_w + gap + 2.0 * dot_r + gap + edge, gap, sep);
    float64 w = g_ui_wm_s.w;
    vec2 view = ui_gfx_view();
    if (!g_ui_wm.placed) {
        g_ui_wm.x = view.x - w - UI_WM_MARGIN;
        g_ui_wm.y = UI_WM_MARGIN;
    }
    if (interactive && g_ui_active == UI_ID_WATERMARK && g_ui_down) {
        g_ui_wm.placed = true;
        g_ui_wm.x = g_ui_mouse.x - g_ui_drag_dx;
        g_ui_wm.y = g_ui_mouse.y - g_ui_drag_dy;
    }
    g_ui_wm.x = ui_snap(ui_clamp(g_ui_wm.x, 0.0, max(view.x - w, 0.0)));
    g_ui_wm.y = ui_snap(ui_clamp(g_ui_wm.y, 0.0, max(view.y - h, 0.0)));
    g_ui_wm.w = w;
    g_ui_wm.h = h;
    g_ui_wm.live = interactive;

    float64 x = g_ui_wm.x;
    float64 y = g_ui_wm.y;
    float64 cut = min(ui_snap(g_ui_m.wm_cut), ui_snap(h * 0.5) - 1.0);
    float64 cy = y + h * 0.5;
    if (x != g_ui_wm_s.gx || y != g_ui_wm_s.gy || w != g_ui_wm_s.gw || h != g_ui_wm_s.gh || cut != g_ui_wm_s.gcut)
        ui_wm_layout(x, y, w, h, cut, x + inset + mark_w + gap, gap, sep, dot_r);
    ui_gfx_polygon(g_ui_wm_s.fill, g_ui_theme.panel);
    ui_wm_glow(x, y, w, h, cut);
    ui_wm_border_draw(cut);
    ui_wm_sweep(x, y, w, h, cut);
    ui_draw_rt_logo(x + inset, y + inset, mark_h);
    ui_gfx_disc(vec2(g_ui_wm_s.dot_x, cy), dot_r, g_ui_theme.wm_dot);
    if (g_ui_wm_s.seps.size() != 0)
        ui_gfx_discs(g_ui_wm_s.seps, ui_col_accent().with_alpha(g_ui_theme.wm_sep_a));
    for (uint64 i = 0; i < parts.size(); i++) {
        bool fallback = parts[i].size() != 0 && parts[i][0] == '~';
        color c = fallback ? g_ui_theme.muted : (i == 0 ? g_ui_theme.text : g_ui_theme.dim);
        ui_gfx_text(parts[i], vec2(g_ui_wm_s.tx[i], g_ui_wm_s.ty[i]), c, g_ui_font_wm);
    }
}

inline void ui_watermark_card(const std::vector<std::string>& parts, bool interactive) {
    ui_ensure_fonts();
    ui_sync_accent();
    float64 pad = g_ui_m.pad;
    float64 h = ui_snap(g_ui_m.wm_h);
    float64 bar_w = ui_snap(g_ui_m.accent_w);
    float64 edge = ui_snap(pad * g_ui_style.wm_pad);
    float64 mark_gap = ui_snap(pad * g_ui_style.wm_mark_gap);
    float64 part_gap = ui_snap(pad * g_ui_style.wm_part_gap);
    float64 gem = ui_snap(g_ui_m.wm_gem);
    float64 rx = gem * g_ui_style.wm_diamond_x;
    float64 ry = gem * g_ui_style.wm_diamond_y;
    float64 mark_w = ui_snap(2.0 * rx);
    // Part 0 is drawn bold below, so measure it bold or it overruns its slot. Each part is measured only when it (or the
    // scale) changed; an unchanged frame measures nothing.
    bool refont = g_ui_wm_card_scale != g_ui_scale_applied || g_ui_wm_card_pw.size() != parts.size();
    g_ui_wm_card_scale = g_ui_scale_applied;
    if (refont) {
        g_ui_wm_card_pw.assign(parts.size(), 0.0);
        g_ui_wm_card_parts.assign(parts.size(), std::string());
    }
    float64 text_w = 0.0;
    for (uint64 i = 0; i < parts.size(); i++) {
        if (refont || g_ui_wm_card_parts[i] != parts[i]) {
            g_ui_wm_card_parts[i] = parts[i];
            g_ui_wm_card_pw[i] = ui_gfx_text_size(i == 0 ? g_ui_font_bold : g_ui_font, parts[i]).x;
        }
        text_w += g_ui_wm_card_pw[i] + (i == 0 ? 0.0 : part_gap);
    }
    float64 w = bar_w + edge + mark_w + mark_gap + ui_snap(text_w) + edge;
    vec2 view = ui_gfx_view();
    if (!g_ui_wm.placed) {
        g_ui_wm.x = view.x - w - UI_WM_MARGIN;
        g_ui_wm.y = UI_WM_MARGIN;
    }
    if (interactive && g_ui_active == UI_ID_WATERMARK && g_ui_down) {
        g_ui_wm.placed = true;
        g_ui_wm.x = g_ui_mouse.x - g_ui_drag_dx;
        g_ui_wm.y = g_ui_mouse.y - g_ui_drag_dy;
    }
    g_ui_wm.x = ui_snap(ui_clamp(g_ui_wm.x, 0.0, max(view.x - w, 0.0)));
    g_ui_wm.y = ui_snap(ui_clamp(g_ui_wm.y, 0.0, max(view.y - h, 0.0)));
    g_ui_wm.w = w;
    g_ui_wm.h = h;
    g_ui_wm.live = interactive;

    float64 x = g_ui_wm.x;
    float64 y = g_ui_wm.y;
    float64 r = ui_snap(g_ui_m.round);
    ui_gfx_fill_corners(vec2(x, y), vec2(w, h), g_ui_theme.panel, r, UI_CORNER_TR | UI_CORNER_BR);
    // The faint edge on the other three sides: the rounded frame cut off where the bar ends (and its left rounding with
    // it), the top and bottom runs filled in should a rounding wider than the bar leave them open.
    float64 cut = max(bar_w, r);
    ui_gfx_clip_push(vec2(x + cut, y), vec2(w - cut, h));
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), g_ui_theme.panel_edge, 1.0, r);
    ui_gfx_clip_pop();
    if (cut > bar_w) {
        ui_gfx_fill(vec2(x + bar_w, y), vec2(cut - bar_w, 1.0), g_ui_theme.panel_edge, 0.0);
        ui_gfx_fill(vec2(x + bar_w, y + h - 1.0), vec2(cut - bar_w, 1.0), g_ui_theme.panel_edge, 0.0);
    }
    // The selected nav item's bar (ui_nav).
    ui_gfx_gradient(vec2(x, y), vec2(bar_w, h), g_ui_theme.gradient, 0.0, false);
    float64 mx = x + bar_w + edge;
    g_ui_wm_card_mark_x = mx;
    ui_wm_diamond(mx + mark_w * 0.5, y + h * 0.5, rx, ry);
    ui_gfx_polygon(g_ui_wm_s.pts, ui_col_accent());
    float64 tx = mx + mark_w + mark_gap;
    g_ui_wm_card_tx.resize(parts.size());
    for (uint64 i = 0; i < parts.size(); i++) {
        bool fallback = parts[i].size() != 0 && parts[i][0] == '~';
        color c = fallback ? g_ui_theme.muted : (i == 0 ? g_ui_theme.text : g_ui_theme.dim);
        g_ui_wm_card_tx[i] = ui_snap(tx);
        ui_text_at(parts[i], g_ui_wm_card_tx[i], y, h, c, i == 0 ? g_ui_font_bold : g_ui_font);
        tx += g_ui_wm_card_pw[i] + part_gap;
    }
}
