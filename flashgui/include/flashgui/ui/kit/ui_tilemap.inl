inline vec2 ui_tilemap_screen(const ui_tilemap_t& m, float64 mx, float64 my) {
    return vec2(m.draw_ox + mx * m.draw_scale, m.draw_oy + my * m.draw_scale);
}

inline vec2 ui_tilemap_point(const ui_tilemap_t& m, float64 sx, float64 sy) {
    return vec2((sx - m.draw_ox) / m.draw_scale, (sy - m.draw_oy) / m.draw_scale);
}

inline void ui_tilemap_fix(ui_tilemap_t& m) {
    m.draw_scale = m.scale;
    m.draw_ox = m.x + m.w * 0.5 - m.cx * m.scale;
    m.draw_oy = m.y + m.h * 0.5 - m.cy * m.scale;
}

inline void ui_tilemap_look(ui_tilemap_t& m, float64 mx, float64 my, float64 scale) {
    m.cx = mx;
    m.cy = my;
    if (scale > 0.0)
        m.scale = ui_clamp(scale, m.scale_min, m.scale_max);
    m.placed = true;
}

inline void ui_tilemap_zoom(ui_tilemap_t& m, float64 f, float64 sx, float64 sy) {
    float64 px = m.cx + (sx - m.x - m.w * 0.5) / m.scale;
    float64 py = m.cy + (sy - m.y - m.h * 0.5) / m.scale;
    m.scale = ui_clamp(m.scale * f, m.scale_min, m.scale_max);
    m.cx = px - (sx - m.x - m.w * 0.5) / m.scale;
    m.cy = py - (sy - m.y - m.h * 0.5) / m.scale;
}

inline void ui_tilemap_fit(ui_tilemap_t& m) {
    float64 s = min(m.w / max(m.map_w, 1.0), m.h / max(m.map_h, 1.0));
    ui_tilemap_look(m, m.map_w * 0.5, m.map_h * 0.5, s > 0.0 ? s : m.scale_min);
}

inline int64 ui_tile_key(int32 z, int32 col, int32 row) {
    return (static_cast<int64>(z + 64) * 65536 + static_cast<int64>(col)) * 65536 + static_cast<int64>(row);
}

inline std::string ui_tile_path(ui_tilemap_t& m, int32 z, int32 col, int32 row) {
    int64 k = ui_tile_key(z, col, row);
    if (m.paths.count(k) == 0) {
        if (m.key_path_fn != nullptr)
            m.paths[k] = m.key_path_fn(m.key, z, col, row);
        else
            m.paths[k] = m.path_fn != nullptr ? m.path_fn(z, col, row) : "";
    }
    return m.paths[k];
}

inline vec2 ui_tilemap_tile_at(const ui_tilemap_t& m, int32 z, int32 col, int32 row) {
    float64 span = m.tile_px * std::pow(2.0, static_cast<float64>(-z));
    return ui_tilemap_screen(m, static_cast<float64>(col) * span, static_cast<float64>(row) * span);
}

inline int32 ui_tilemap_level(const ui_tilemap_t& m) {
    for (int32 z = m.z_min; z < m.z_max; z++) {
        if (std::pow(2.0, static_cast<float64>(z)) * UI_TILE_MAGNIFY >= m.scale)
            return z;
    }
    return m.z_max;
}

inline void ui_tilemap_stand_in(ui_tilemap_t& m, int32 z, int32 col, int32 row, vec2 pos, vec2 size) {
    for (int32 k = 1; z - k >= m.z_min; k++) {
        int32 f = 1 << k;
        int32 pc = col / f;
        int32 pr = row / f;
        std::string path = ui_tile_path(m, z - k, pc, pr);
        if (path.size() == 0)
            continue;
        int64 tex = ui_pic_tex(path, z - k == m.z_min, UI_TILE_IDLE_MS);
        if (tex == 0)
            continue;
        float64 inv = 1.0 / static_cast<float64>(f);
        vec2 uv0 = vec2(static_cast<float64>(col - pc * f) * inv, static_cast<float64>(row - pr * f) * inv);
        ui_gfx_image_uv(tex, pos, size, uv0, vec2(uv0.x + inv, uv0.y + inv), UI_WHITE);
        return;
    }
}

inline void ui_tilemap_tiles(ui_tilemap_t& m, int32 z) {
    float64 span = m.tile_px * std::pow(2.0, static_cast<float64>(-z));
    vec2 a = ui_tilemap_point(m, m.x, m.y);
    vec2 b = ui_tilemap_point(m, m.x + m.w, m.y + m.h);
    int32 cols = static_cast<int32>(std::ceil(m.map_w / span));
    int32 rows = static_cast<int32>(std::ceil(m.map_h / span));
    int32 c0 = max(static_cast<int32>(std::floor(a.x / span)), 0);
    int32 c1 = min(static_cast<int32>(std::floor(b.x / span)), cols - 1);
    int32 r0 = max(static_cast<int32>(std::floor(a.y / span)), 0);
    int32 r1 = min(static_cast<int32>(std::floor(b.y / span)), rows - 1);
    for (int32 row = r0; row <= r1; row++) {
        for (int32 col = c0; col <= c1; col++) {
            std::string path = ui_tile_path(m, z, col, row);
            if (path.size() == 0) {
                m.missing = m.missing + 1;
                continue;
            }
            vec2 pos = ui_tilemap_tile_at(m, z, col, row);
            vec2 end = ui_tilemap_tile_at(m, z, col + 1, row + 1);
            vec2 size = vec2(end.x - pos.x, end.y - pos.y);
            int64 tex = ui_pic_tex(path, false, UI_TILE_IDLE_MS);
            if (tex != 0) {
                ui_gfx_image(tex, pos, size, UI_WHITE);
                m.drawn = m.drawn + 1;
                continue;
            }
            // The stand-in asks for its coarse tile first: one load that covers many of these.
            ui_tilemap_stand_in(m, z, col, row, pos, size);
            ui_pic_tex(path, true, UI_TILE_IDLE_MS);
            m.loading = m.loading + 1;
        }
    }
}

inline bool ui_tilemap_in_hole(const ui_tilemap_t& m) {
    const std::vector<float64>& h = m.holes_last;
    for (uint64 i = 0; i + 3 < h.size(); i = i + 4) {
        if (ui_mouse_in(m.x + h[i], m.y + h[i + 1], h[i + 2], h[i + 3]))
            return true;
    }
    return false;
}

inline void ui_tilemap_hole(ui_tilemap_t& m, float64 x, float64 y, float64 w, float64 h) {
    m.holes.push_back(x - m.x);
    m.holes.push_back(y - m.y);
    m.holes.push_back(w);
    m.holes.push_back(h);
}

inline void ui_tilemap_clamp(ui_tilemap_t& m) {
    m.scale = ui_clamp(m.scale, m.scale_min, m.scale_max);
    m.cx = ui_clamp(m.cx, 0.0, m.map_w);
    m.cy = ui_clamp(m.cy, 0.0, m.map_h);
}

inline void ui_tilemap(ui_tilemap_t& m, float64 x, float64 y, float64 w, float64 h) {
    m.x = x;
    m.y = y;
    m.w = w;
    m.h = h;
    m.clicked = false;
    m.rclicked = false;
    m.drawn = 0;
    m.loading = 0;
    m.missing = 0;
    m.buttons = 0;
    m.corner_left = x + w;
    m.holes_last = m.holes;
    m.holes.clear();
    m.id = ui_id();
    m.hovered = false;
    // A drag lasts only while the map holds the press. One cut while the map was not drawn (the menu hidden, the host
    // in the background) never sees its release here.
    if (g_ui_active != m.id)
        m.dragging = false;
    if (w <= 0.0 || h <= 0.0)
        return;
    if (!m.placed)
        ui_tilemap_fit(m);
    bool hot = ui_hovered(x, y, w, h) && !ui_tilemap_in_hole(m);
    m.hovered = hot;
    if (ui_take_press(m.id, hot)) {
        m.press_at = g_ui_mouse;
        m.press_cx = m.cx;
        m.press_cy = m.cy;
        m.dragging = false;
    }
    if (g_ui_active == m.id) {
        float64 dx = g_ui_mouse.x - m.press_at.x;
        float64 dy = g_ui_mouse.y - m.press_at.y;
        if (!m.dragging && dx * dx + dy * dy >= UI_TILE_DRAG_PX * UI_TILE_DRAG_PX)
            m.dragging = true;
        if (m.dragging) {
            m.cx = m.press_cx - dx / m.scale;
            m.cy = m.press_cy - dy / m.scale;
        }
        if (g_ui_released) {
            m.clicked = !m.dragging && hot;
            m.click_at = g_ui_mouse;
            m.dragging = false;
        }
    }
    if (hot && g_ui_rpressed && g_ui_active == 0) {
        g_ui_rpressed = false;
        m.rclicked = true;
        m.click_at = g_ui_mouse;
    }
    if (hot && g_ui_scroll != 0.0 && (g_ui_active == 0 || g_ui_active == m.id)) {
        ui_tilemap_zoom(m, g_ui_scroll > 0.0 ? UI_TILE_ZOOM_STEP : 1.0 / UI_TILE_ZOOM_STEP, g_ui_mouse.x, g_ui_mouse.y);
        g_ui_scroll = 0.0;
        // A drag goes on from here at the new scale.
        m.press_at = g_ui_mouse;
        m.press_cx = m.cx;
        m.press_cy = m.cy;
    }
    ui_tilemap_clamp(m);
    ui_tilemap_fix(m);
    vec2 mp = ui_tilemap_point(m, g_ui_mouse.x, g_ui_mouse.y);
    m.mouse_mx = mp.x;
    m.mouse_my = mp.y;
    ui_gfx_clip_push(vec2(x, y), vec2(w, h));
    ui_gfx_fill(vec2(x, y), vec2(w, h), m.back, 0.0);
    m.level = ui_tilemap_level(m);
    ui_tilemap_tiles(m, m.level);
    if (m.layer_fn != nullptr)
        m.layer_fn();
    ui_gfx_clip_pop();
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), g_ui_theme.border, 1.0, 0.0);
}

inline bool ui_tilemap_button(ui_tilemap_t& m, int32 glyph, const std::string& tip, bool lit) {
    float64 s = ui_snap(g_ui_m.row_h * g_ui_style.tile_btn);
    float64 gap = ui_snap(g_ui_m.pad * g_ui_style.tile_btn_gap);
    float64 bx = m.x + m.w - gap - s;
    float64 by = m.y + gap + static_cast<float64>(m.buttons) * (s + gap);
    m.buttons = m.buttons + 1;
    int32 id = ui_id();
    if (m.w <= 0.0 || by + s > m.y + m.h)
        return false;
    ui_tilemap_hole(m, bx, by, s, s);
    bool hot = ui_hovered(bx, by, s, s);
    bool clicked = ui_button_behavior(id, hot);
    bool hover = ui_lit(hot, id);
    ui_gfx_fill(vec2(bx, by), vec2(s, s), hover ? g_ui_theme.hover : g_ui_theme.panel, g_ui_m.round);
    ui_gfx_frame(vec2(bx + 0.5, by + 0.5), vec2(s - 1.0, s - 1.0), lit ? ui_col_accent() : g_ui_theme.panel_edge, 1.0,
        g_ui_m.round);
    color c = lit ? ui_col_accent() : (hover ? g_ui_theme.text : g_ui_theme.dim);
    ui_draw_glyph(glyph, bx + s * 0.5, by + s * 0.5, ui_snap(s * g_ui_style.tile_glyph), c);
    if (hot && g_ui_active == 0)
        g_ui_tip.text = tip;
    return clicked;
}

inline bool ui_tilemap_corner_button(ui_tilemap_t& m, const std::string& text, const std::string& tip, bool open, vec2& top_right) {
    float64 s = ui_snap(g_ui_m.row_h * g_ui_style.tile_btn);
    float64 gap = ui_snap(g_ui_m.pad * g_ui_style.tile_btn_gap);
    float64 pad = ui_snap(g_ui_m.pad);
    float64 cw = ui_snap(s * g_ui_style.tile_chevron);
    std::string label = ui_fit_text(text, max(m.w * 0.5 - 3.0 * pad - 2.0 * cw, 0.0));
    float64 bw = ui_snap(ui_gfx_text_size(g_ui_font, label).x) + 3.0 * pad + 2.0 * cw;
    float64 right = m.corner_left - gap;
    float64 bx = right - bw;
    float64 by = m.y + m.h - gap - s;
    top_right = vec2(right, by);
    int32 id = ui_id();
    if (m.w <= 0.0 || bx < m.x + gap || by < m.y + gap + static_cast<float64>(m.buttons) * (s + gap))
        return false;
    m.corner_left = bx;
    ui_tilemap_hole(m, bx, by, bw, s);
    bool hot = ui_hovered(bx, by, bw, s);
    bool clicked = ui_button_behavior(id, hot);
    bool hover = ui_lit(hot, id);
    ui_gfx_fill(vec2(bx, by), vec2(bw, s), hover ? g_ui_theme.hover : g_ui_theme.panel, g_ui_m.round);
    ui_gfx_frame(vec2(bx + 0.5, by + 0.5), vec2(bw - 1.0, s - 1.0), open ? ui_col_accent() : g_ui_theme.panel_edge, 1.0,
        g_ui_m.round);
    ui_text_at(label, bx + pad, by, s, g_ui_theme.text, g_ui_font);
    color c = open ? ui_col_accent() : (hover ? g_ui_theme.text : g_ui_theme.dim);
    ui_draw_chevron(bx + bw - pad - cw, by + s * 0.5, cw, true, c);
    if (hot && g_ui_active == 0)
        g_ui_tip.text = tip;
    return clicked;
}
