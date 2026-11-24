inline color ui_col_text() { return g_ui_theme.text; }

inline color ui_col_dim() { return g_ui_theme.dim; }

inline color ui_col_muted() { return g_ui_theme.muted; }

inline color ui_col_deny() { return g_ui_theme.deny; }

inline color ui_col_warn() { return g_ui_theme.warn; }

inline color ui_col_active() { return g_ui_theme.active; }

inline color ui_col_border() { return g_ui_theme.border; }

inline color ui_col_border_hi() { return g_ui_theme.border_hi; }

inline color ui_col_bg() { return g_ui_theme.bg; }

inline color ui_col_bg2() { return g_ui_theme.bg2; }

inline color ui_col_hover() { return g_ui_theme.hover; }

inline color ui_col_pressed() { return g_ui_theme.pressed; }

inline color ui_col_group() { return g_ui_theme.group; }

inline color ui_col_card_top() { return g_ui_theme.card_top; }

inline color ui_col_panel() { return g_ui_theme.panel; }

inline color ui_col_panel_edge() { return g_ui_theme.panel_edge; }

inline color ui_col_knob() { return g_ui_theme.knob; }

inline color ui_col_shadow() { return g_ui_theme.shadow; }

inline color ui_col_wm_dot() { return g_ui_theme.wm_dot; }

inline color ui_col_check_lo() { return g_ui_theme.check_lo; }

inline color ui_col_check_hi() { return g_ui_theme.check_hi; }

inline color ui_col_teal() { return g_ui_theme.teal; }

inline color ui_col_purple() { return g_ui_theme.purple; }

inline color ui_col_purple_hi() { return g_ui_theme.purple_hi; }

inline color ui_col_indigo() { return g_ui_theme.indigo; }

inline color ui_col_indigo_hi() { return g_ui_theme.indigo_hi; }

inline std::vector<color> ui_col_gradient() { return g_ui_theme.gradient; }

inline color ui_get_accent() { return g_ui_theme.accent; }

inline void ui_set_accent(color c) {
    g_ui_theme.accent = c;
}

inline void ui_set_theme(const ui_theme_t& t) {
    g_ui_theme = t;
}

inline float64 ui_m_pad() { return g_ui_m.pad; }

inline float64 ui_m_row_h() { return g_ui_m.row_h; }

inline float64 ui_m_frame_h() { return g_ui_m.frame_h; }

inline float64 ui_m_combo_h() { return g_ui_m.combo_h; }

inline float64 ui_m_item_h() { return g_ui_m.item_h; }

inline float64 ui_m_wm_h() { return g_ui_m.wm_h; }

inline float64 ui_m_wm_cut() { return g_ui_m.wm_cut; }

inline float64 ui_m_wm_sep_r() { return g_ui_m.wm_sep_r; }

inline float64 ui_m_wm_dot_r() { return g_ui_m.wm_dot_r; }

inline float64 ui_m_dot_r() { return g_ui_m.dot_r; }

inline float64 ui_m_accent_w() { return g_ui_m.accent_w; }

inline float64 ui_m_round() { return g_ui_m.round; }

inline float64 ui_m_box() { return g_ui_m.box; }

inline float64 ui_m_side_w() { return g_ui_m.side_w; }

inline float64 ui_m_head_h() { return g_ui_m.head_h; }

inline float64 ui_m_foot_h() { return g_ui_m.foot_h; }

inline float64 ui_m_font_px() { return g_ui_m.font; }

inline float64 ui_m_font_small_px() { return g_ui_m.font_small; }

inline float64 ui_m_stroke() { return g_ui_m.stroke; }

inline float64 ui_m_line_gap() { return g_ui_m.line_gap; }

inline float64 ui_style_wm_rt() { return g_ui_style.wm_rt; }

inline float64 ui_style_wm_gap() { return g_ui_style.wm_gap; }

inline float64 ui_style_wm_edge() { return g_ui_style.wm_edge; }

inline float64 ui_style_bar_min() { return g_ui_style.bar_min; }

inline float64 ui_style_text_line() { return g_ui_style.text_line; }

inline float64 ui_scale() { return g_ui_scale; }

inline void ui_set_scale(float64 s) { g_ui_scale = ui_scale_preset(s); }

inline int32 ui_scale_index_current() { return ui_scale_index(g_ui_scale); }

inline int64 ui_font() { return g_ui_font; }

inline int64 ui_font_bold() { return g_ui_font_bold; }

inline int64 ui_font_small() { return g_ui_font_small; }

inline int64 ui_font_brand() { return g_ui_font_brand; }

inline float64 ui_font_h() { return g_ui_font_h; }

inline int64 ui_font_bold_at(int64 px) {
    return ui_font_at(g_ui_fonts_bold, g_ui_cfg.font_bold_path, px, g_ui_font_bold);
}

inline float64 ui_window_x() { return g_ui_x; }

inline float64 ui_window_y() { return g_ui_y; }

inline float64 ui_window_w() { return g_ui_w; }

inline float64 ui_window_h() { return g_ui_h; }

inline void ui_set_window_pos(float64 x, float64 y) { g_ui_x = x; g_ui_y = y; }

inline void ui_set_window_size(float64 w, float64 h) { g_ui_w = w; g_ui_h = h; }

inline vec2 ui_mouse() { return g_ui_mouse; }

inline bool ui_mouse_down() { return g_ui_down; }

inline bool ui_mouse_pressed() { return g_ui_pressed; }

inline bool ui_mouse_released() { return g_ui_released; }

inline int32 ui_active_id() { return g_ui_active; }

inline void ui_set_active_id(int32 id) { g_ui_active = id; }

inline vec2 ui_drag_offset() { return vec2(g_ui_drag_dx, g_ui_drag_dy); }

inline void ui_set_drag_offset(float64 dx, float64 dy) { g_ui_drag_dx = dx; g_ui_drag_dy = dy; }

inline float64 ui_scroll() { return g_ui_scroll; }

inline void ui_set_tooltip(const std::string& text) { g_ui_tip.text = text; }

inline bool ui_kb_live() { return g_kb_live; }

inline void ui_set_kb_live(bool live) { g_kb_live = live; }

inline void ui_reserve_keys(const std::vector<int64>& keys) { g_ui_keys_taken = keys; }

inline void ui_clear_reserved_keys() { g_ui_keys_taken.clear(); }

inline float64 ui_cursor() { return g_ui_cursor; }

inline void ui_set_cursor(float64 y) { g_ui_cursor = y; }

inline void ui_cursor_advance(float64 dy) { g_ui_cursor = g_ui_cursor + dy; }

inline void ui_set_fonts(const std::string& regular, const std::string& bold) {
    g_ui_cfg.font_path = regular;
    g_ui_cfg.font_bold_path = bold;
}

inline void ui_set_brand_font(const std::string& path, float64 scale) {
    g_ui_cfg.font_brand_path = path;
    g_ui_cfg.brand_scale = scale;
}

inline void ui_set_logo_font(const std::string& path) {
    g_ui_cfg.font_logo_path = path;
}

inline void ui_set_logo_path(const std::string& path) {
    g_ui_cfg.logo_path = path;
}

inline void ui_set_tooltip_kinds(const std::vector<std::string>& prefixes, const std::vector<std::string>& heads, const std::vector<color>& cols) {
    g_ui_cfg.tip_prefixes = prefixes;
    g_ui_cfg.tip_heads = heads;
    g_ui_cfg.tip_cols = cols;
}

inline std::string ui_get_subtitle() { return g_ui_cfg.subtitle; }

inline void ui_set_watermark_rect(float64 x, float64 y, float64 w, float64 h, bool interactive) {
    g_ui_wm.x = x;
    g_ui_wm.y = y;
    g_ui_wm.w = w;
    g_ui_wm.h = h;
    g_ui_wm.live = interactive;
}

inline bool ui_watermark_placed() { return g_ui_wm.placed; }

inline void ui_set_watermark_placed(bool p) { g_ui_wm.placed = p; }

inline float64 ui_watermark_x() { return g_ui_wm.x; }

inline float64 ui_watermark_y() { return g_ui_wm.y; }

inline float64 ui_watermark_w() { return g_ui_wm.w; }

inline float64 ui_watermark_h() { return g_ui_wm.h; }
