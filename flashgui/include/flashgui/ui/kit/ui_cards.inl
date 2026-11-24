inline void ui_spinner(float64 cx, float64 cy, float64 r) {
    int64 now = ui_sys_ms();
    float64 turn = static_cast<float64>(now % UI_SPIN_MS) / static_cast<float64>(UI_SPIN_MS);
    float64 pulse = static_cast<float64>(now % UI_SPIN_PULSE_MS) / static_cast<float64>(UI_SPIN_PULSE_MS);
    float64 breath = pulse < 0.5 ? pulse * 2.0 : 2.0 - pulse * 2.0;
    float64 sweep = g_ui_style.spin_arc_min + g_ui_style.spin_arc_span * breath;
    float64 th = max(g_ui_style.bar_min, ui_snap(r * g_ui_style.spin_w));
    color c = ui_col_accent();
    ui_gfx_ring(vec2(cx, cy), r, c.with_alpha(g_ui_theme.spin_ring_a), th);
    ui_gfx_arc(vec2(cx, cy), r, turn * 360.0, sweep, c, th);
}

inline void ui_grid_begin() {
    float64 w = ui_snap(ui_content_w());
    float64 gap = ui_snap(g_ui_m.pad * g_ui_style.card_gap);
    int32 cols = max(1, static_cast<int32>((w + gap) / (g_ui_m.card_w + gap)));
    ui_need(g_ui_m.card_w);
    g_ui_grid.x = ui_snap(ui_content_x());
    g_ui_grid.y = ui_snap(g_ui_cursor + g_ui_m.pad * g_ui_style.card_top);
    g_ui_grid.gap = gap;
    g_ui_grid.cols = cols;
    g_ui_grid.cw = static_cast<float64>(static_cast<int64>((w - gap * static_cast<float64>(cols - 1)) / static_cast<float64>(cols)));
    g_ui_grid.ch = g_ui_grid.cw + ui_snap(g_ui_m.row_h);
    g_ui_grid.n = 0;
}

inline void ui_grid_end() {
    int32 rows = (g_ui_grid.n + g_ui_grid.cols - 1) / g_ui_grid.cols;
    float64 h = rows > 0 ? static_cast<float64>(rows) * (g_ui_grid.ch + g_ui_grid.gap) - g_ui_grid.gap : 0.0;
    g_ui_cursor = g_ui_grid.y + h + ui_snap(g_ui_m.pad * 0.5);
}

inline int32 ui_card_picture(const std::string& image, float64 x, float64 y, float64 size) {
    if (image.size() == 0)
        return UI_PIC_NONE;
    if (image.find(".png") != std::string::npos)
        return ui_pic(image, x, y, size);
    if (ui_gfx_icon_key(image, x, y, size))
        return UI_PIC_SHOWN;
    return ui_gfx_icon_loading(image) ? UI_PIC_LOADING : UI_PIC_NONE;
}

inline bool ui_card_enabled(const std::string& image, const std::string& title, const std::string& tag, bool selected, bool enabled) {
    ui_grid_t& g = g_ui_grid;
    int32 i = g.n;
    g.n = g.n + 1;
    float64 x = g.x + static_cast<float64>(i % g.cols) * (g.cw + g.gap);
    float64 y = g.y + static_cast<float64>(i / g.cols) * (g.ch + g.gap);
    float64 cw = g.cw;
    float64 ch = g.ch;
    int32 id = ui_id();
    bool hot = ui_hovered(x, y, cw, ch);
    bool clicked = ui_button_behavior(id, enabled && hot);
    float64 vis_top = g_ui_in_group ? g_ui_clip_y : g_ui_body.y;
    float64 vis_bottom = g_ui_in_group ? g_ui_clip_y + g_ui_clip_h : g_ui_body.y + g_ui_body.h;
    if (y + ch < vis_top || y > vis_bottom)
        return clicked;
    bool lit = enabled && ui_lit(hot, id);
    float64 r = g_ui_m.round;
    color edge = selected ? ui_col_accent() : (lit ? g_ui_theme.border_hi : g_ui_theme.border);
    ui_draw_box(x, y, cw, ch, lit ? g_ui_theme.hover : g_ui_theme.bg2, edge);
    color top = g_ui_theme.card_top;
    ui_gfx_corners(vec2(x + 1.0, y + 1.0), vec2(cw - 2.0, cw - 2.0), top, top, g_ui_theme.bg2, g_ui_theme.bg2, r);
    float64 ip = ui_snap(g_ui_m.pad * 0.5);
    float64 is = cw - 2.0 * ip;
    int32 pic = ui_card_picture(image, x + ip, y + ip, is);
    if (pic == UI_PIC_LOADING)
        ui_spinner(x + cw * 0.5, y + cw * 0.5, ui_snap(max(is * g_ui_style.card_spin, g_ui_style.card_spin_min)));
    else if (pic == UI_PIC_NONE && title.size() != 0)
        ui_text_center(title.substr(0, 1), x, y, cw, cw, g_ui_theme.muted, g_ui_font_brand_big);
    float64 pad = ui_snap(g_ui_m.pad);
    float64 tw = ui_gfx_text_size(g_ui_font, title).x;
    bool cut = tw > cw - pad;
    int64 font = selected ? g_ui_font_bold : g_ui_font;
    ui_text_center(cut ? ui_fit_text_in(font, title, cw - pad) : title, x, y + cw, cw, ch - cw,
        !enabled ? g_ui_theme.muted : selected || lit ? g_ui_theme.text : g_ui_theme.dim, font);
    if (!enabled)
        ui_gfx_fill(vec2(x + 1.0, y + 1.0), vec2(cw - 2.0, cw - 2.0), g_ui_theme.panel, r);
    if (cut && hot && g_ui_active == 0)
        g_ui_tip.text = title;
    if (tag.size() != 0) {
        vec2 ts = ui_gfx_text_size(g_ui_font_small, tag);
        float64 th = ui_snap(ts.y + g_ui_style.badge_pad_y);
        float64 tx = x + ip;
        float64 ty = y + ip;
        ui_gfx_fill(vec2(tx, ty), vec2(ui_snap(ts.x + pad), th), g_ui_theme.panel, th * 0.5);
        ui_text_center(tag, tx, ty, ui_snap(ts.x + pad), th, g_ui_theme.active, g_ui_font_small);
    }
    if (selected) {
        // The stroke is centred on its rect: inset by half of it, the border stays inside the card.
        float64 sw = g_ui_style.card_sel_w;
        ui_gfx_frame(vec2(x + sw * 0.5, y + sw * 0.5), vec2(cw - sw, ch - sw), ui_col_accent(), sw, r);
        float64 bs = ui_snap(g_ui_m.box);
        ui_draw_check(x + cw - ip - bs, y + ip, bs, true, false);
    }
    return clicked;
}

inline bool ui_card(const std::string& image, const std::string& title, const std::string& tag, bool selected) {
    return ui_card_enabled(image, title, tag, selected, true);
}
