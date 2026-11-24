inline void ui_set_subtitle(const std::string& s) {
    g_ui_cfg.subtitle = s;
}

inline float64 ui_stroke_w() {
    return max(g_ui_m.stroke, 1.0);
}

inline int64 ui_font_at(std::map<int64, int64>& cache, const std::string& path, int64 px, int64 fallback) {
    if (cache.count(px) == 0) {
        int64 f = ui_gfx_font(path, px);
        cache[px] = f != 0 ? f : fallback;
    }
    return cache[px];
}

inline void ui_load_fonts() {
    int64 px = static_cast<int64>(g_ui_m.font + 0.5);
    int64 small = static_cast<int64>(g_ui_m.font_small + 0.5);
    const std::string& path = g_ui_cfg.font_path;
    std::string brand = g_ui_cfg.font_brand_path.size() != 0 ? g_ui_cfg.font_brand_path : g_ui_cfg.font_bold_path;
    g_ui_font = ui_font_at(g_ui_fonts, path, px, ui_gfx_font_default());
    g_ui_font_bold = ui_font_at(g_ui_fonts_bold, g_ui_cfg.font_bold_path, px, g_ui_font);
    g_ui_font_small = ui_font_at(g_ui_fonts, path, small, g_ui_font);
    g_ui_font_brand = ui_font_at(g_ui_fonts_brand, brand, px, g_ui_font_bold);
    int64 big = static_cast<int64>(g_ui_m.font * g_ui_cfg.brand_scale + 0.5);
    g_ui_font_brand_big = ui_font_at(g_ui_fonts_brand, brand, big, g_ui_font_brand);
    g_ui_font_wm = ui_font_at(g_ui_fonts, path, static_cast<int64>(g_ui_m.wm_font_info + 0.5), g_ui_font_small);
    g_ui_font_h = ui_gfx_text_size(g_ui_font, "Ag").y;
    g_ui_dots_w = ui_gfx_text_size(g_ui_font_bold, "...").x;
}

inline int64 ui_rt_font(int64 px) {
    if (g_ui_fonts_logo.count(px) == 0) {
        int64 f = g_ui_cfg.font_logo_path.size() != 0 ? ui_gfx_font_ranges(g_ui_cfg.font_logo_path, px, UI_RT_GLYPHS) : 0;
        g_ui_fonts_logo[px] = f != 0 ? f : ui_font_at(g_ui_fonts_bold, g_ui_cfg.font_bold_path, px, g_ui_font_bold);
    }
    return g_ui_fonts_logo[px];
}

inline int32 ui_scale_index(float64 s) {
    int32 best = 0;
    for (uint64 i = 1; i < UI_SCALE_PRESETS.size(); i++) {
        if (std::abs(UI_SCALE_PRESETS[i] - s) < std::abs(UI_SCALE_PRESETS[static_cast<uint64>(best)] - s))
            best = static_cast<int32>(i);
    }
    return best;
}

inline float64 ui_scale_preset(float64 s) {
    return UI_SCALE_PRESETS[static_cast<uint64>(ui_scale_index(s))];
}

inline float64 ui_min_w_at(float64 s) {
    ui_metrics_t b = g_ui_base;
    float64 col = max(b.col_min_w, g_ui_need_col_1);
    float64 n = static_cast<float64>(UI_MIN_COLS);
    // Plus the slack ui_body_begin allows, so the snapped layout still fits the columns.
    return max(UI_PRESET_MIN_W[static_cast<uint64>(ui_scale_index(s))],
        (b.side_w + n * col + (n + 1.0) * b.pad) * s + n * g_ui_style.col_slack);
}

inline float64 ui_min_h_at(float64 s) {
    return max(UI_PRESET_MIN_H[static_cast<uint64>(ui_scale_index(s))], g_ui_side_need_1 * s);
}

inline void ui_apply_scale(float64 s) {
    g_ui_scale_applied = s;
    ui_metrics_t b = g_ui_base;
    g_ui_m.pad = b.pad * s;
    g_ui_m.side_w = b.side_w * s;
    g_ui_m.logo = b.logo * s;
    g_ui_m.head_h = b.head_h * s;
    g_ui_m.nav_h = b.nav_h * s;
    g_ui_m.foot_h = b.foot_h * s;
    g_ui_m.row_h = b.row_h * s;
    g_ui_m.frame_h = b.frame_h * s;
    g_ui_m.combo_h = b.combo_h * s;
    g_ui_m.item_h = b.item_h * s;
    g_ui_m.box = b.box * s;
    g_ui_m.toggle_h = b.toggle_h * s;
    g_ui_m.knob_r = b.knob_r * s;
    g_ui_m.track_h = b.track_h * s;
    g_ui_m.track_min_w = b.track_min_w * s;
    g_ui_m.value_w = b.value_w * s;
    g_ui_m.number_w = b.number_w * s;
    g_ui_m.key_w = b.key_w * s;
    g_ui_m.stub_w = b.stub_w * s;
    g_ui_m.swatch_w = b.swatch_w * s;
    g_ui_m.icon = b.icon * s;
    g_ui_m.group_title_h = b.group_title_h * s;
    g_ui_m.accent_w = b.accent_w * s;
    g_ui_m.underline_h = b.underline_h * s;
    g_ui_m.tab_pad = b.tab_pad * s;
    g_ui_m.col_min_w = b.col_min_w * s;
    g_ui_m.grip = b.grip * s;
    g_ui_m.grip_dot = b.grip_dot * s;
    g_ui_m.grip_gap = b.grip_gap * s;
    g_ui_m.card_w = b.card_w * s;
    g_ui_m.tip_r = b.tip_r * s;
    g_ui_m.tip_max_w = b.tip_max_w * s;
    g_ui_m.dot_r = b.dot_r * s;
    g_ui_m.wm_h = b.wm_h * s;
    g_ui_m.wm_font_info = b.wm_font_info * s;
    g_ui_m.wm_cut = b.wm_cut * s;
    g_ui_m.wm_sep_r = b.wm_sep_r * s;
    g_ui_m.wm_dot_r = b.wm_dot_r * s;
    g_ui_m.wm_gem = b.wm_gem * s;
    g_ui_m.wm_sweep_w = b.wm_sweep_w * s;
    g_ui_m.scroll_w = b.scroll_w * s;
    g_ui_m.line_gap = b.line_gap * s;
    g_ui_m.round = b.round * s;
    g_ui_m.stroke = b.stroke * s;
    g_ui_m.font = b.font * s;
    g_ui_m.font_small = b.font_small * s;
    ui_load_fonts();
    // Text does not scale exactly with s (whole-pixel fonts): measured again from the next frame.
    g_ui_need_col_1 = 0.0;
}

inline void ui_settle_scale() {
    int64 now = ui_sys_ms();
    g_ui_scale = ui_scale_preset(g_ui_scale);
    if (g_ui_scale != g_ui_scale_seen) {
        g_ui_scale_seen = g_ui_scale;
        g_ui_scale_changed_at = now;
    }
    if (g_ui_scale == g_ui_scale_applied || (g_ui_active > 0 && g_ui_down))
        return;
    if (g_ui_released || now - g_ui_scale_changed_at >= UI_SCALE_SETTLE_MS) {
        ui_apply_scale(g_ui_scale);
        g_ui_refit = true;
    }
}

inline void ui_ensure_fonts() {
    if (g_ui_font != 0)
        return;
    g_ui_scale_seen = g_ui_scale;
    ui_apply_scale(g_ui_scale);
}
