#pragma once

inline color ui_col_accent();

inline color ui_col_accent_hi();

inline color ui_fade(color c, float64 f);

inline void ui_set_subtitle(const std::string& s);

inline float64 ui_stroke_w();

inline int64 ui_font_at(std::map<int64, int64>& cache, const std::string& path, int64 px, int64 fallback);

inline void ui_load_fonts();

inline int64 ui_rt_font(int64 px);

inline int32 ui_scale_index(float64 s);

inline float64 ui_scale_preset(float64 s);

inline float64 ui_min_w_at(float64 s);

inline float64 ui_min_h_at(float64 s);

inline void ui_apply_scale(float64 s);

inline void ui_settle_scale();

inline void ui_ensure_fonts();

inline int32 ui_id();

inline bool ui_mouse_in(float64 x, float64 y, float64 w, float64 h);

inline bool ui_hovered(float64 x, float64 y, float64 w, float64 h);

inline bool ui_hovered_in(float64 rx, float64 ry, float64 rw, float64 rh, float64 x, float64 y, float64 w, float64 h);

inline bool ui_lit(bool hot, int32 id);

inline bool ui_take_press(int32 id, bool hot);

inline bool ui_button_behavior(int32 id, bool hot);

inline void ui_reset_press(bool down, bool rdown);

inline float64 ui_clamp(float64 v, float64 lo, float64 hi);

inline float64 ui_snap(float64 v);

inline float64 ui_content_x();

inline float64 ui_content_w();

inline void ui_area_body();

inline void ui_need(float64 w);

inline float64 ui_row_begin(float64 h);

inline void ui_row_mark(float64 x, float64 y, float64 h, float64 label_end, float64 limit);

inline float64 ui_max_text_w(const std::vector<std::string>& texts);

inline void ui_text_at(const std::string& text, float64 x, float64 row_y, float64 row_h, color c, int64 font);

inline void ui_text_center(const std::string& text, float64 x, float64 y, float64 w, float64 h, color c, int64 font);

inline std::string ui_fit_text_with(int64 font, const std::string& text, float64 max_w, const std::string& mark);

inline std::string ui_fit_text_in(int64 font, const std::string& text, float64 max_w);

inline std::string ui_fit_text(const std::string& text, float64 max_w);

inline void ui_wrap_lines(const std::string& text, float64 w, int64 font, std::vector<std::string>& out);

inline void ui_wrap_line(const std::string& line, float64 w, int64 font, std::vector<std::string>& out);

inline void ui_draw_box(float64 x, float64 y, float64 w, float64 h, color bg, color border);

inline void ui_draw_field(float64 x, float64 y, float64 w, float64 h, bool lit, bool focus);

inline void ui_draw_check(float64 x, float64 y, float64 s, bool on, bool lit);

inline void ui_row_hover(float64 x, float64 y, float64 w, float64 h, bool lit);

inline void ui_draw_chevron(float64 cx, float64 cy, float64 s, bool up, color c);

inline void ui_draw_cross(float64 cx, float64 cy, float64 s, color c, float64 t);

inline void ui_glyph_quad(float64 x0, float64 y0, float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3,
    color c);

inline void ui_draw_glyph(int32 g, float64 cx, float64 cy, float64 s, color c);

inline float64 ui_border_round();

inline void ui_border_fill(float64 x, float64 y, float64 w, float64 h, color c, int32 corners);

inline void ui_gradient_border(float64 x, float64 y, float64 w, float64 h);

inline void ui_scroll_thumb(float64 gx, float64 gw, float64 ty, float64 th, bool lit, color idle, color hot);

inline void ui_icon_next(const std::string& key);

inline void ui_icon_next_path(const std::string& path);

inline float64 ui_item_icon(float64 h);

inline float64 ui_icon_w();

inline bool ui_icon_any(const std::string& icon, float64 x, float64 y, float64 size);

inline float64 ui_icon_take(float64 x, float64 y, float64 h);

inline void ui_logo_load();

inline void ui_draw_logo(float64 cx, float64 top, float64 size);

inline float64 ui_rt_outline(float64 h);

inline float64 ui_rt_t_dx(float64 p, float64 o);

inline float64 ui_rt_logo_w(float64 h);

inline void ui_draw_rt_logo(float64 x, float64 y, float64 h);

inline void ui_draw_brand(float64 cx, float64 top, float64 box_w, float64 box_h);

inline void ui_begin(const std::string& title, bool down, bool tap, bool rdown, bool rtap, float64 wheel);

inline bool ui_nav(const std::string& label, int32 index, int32& current);

inline bool ui_head_square(float64 bx, float64 by, float64 s, const std::string& icon, int32 glyph, const std::string& tip,
    bool lit);

inline bool ui_head_button(const std::string& icon, int32 glyph, const std::string& tip, bool lit);

inline bool ui_head_back(const std::string& tip);

inline bool ui_tabs(const std::vector<std::string>& labels, int32& current);

inline void ui_body_split(float64 side_w, float64& cx, float64& cy, float64& cw, float64& ch);

inline void ui_body_begin(int64 page, int32 cols);

inline float64 ui_scroll_step();

inline void ui_body_end();

inline std::string ui_group_label(const std::string& title);

inline bool ui_group_begin(const std::string& title, int32 col);

inline void ui_group_end();

inline void ui_footer(const std::vector<std::string>& texts, const std::vector<color>& cols);

inline void ui_resize_grip();

inline void ui_end();

inline bool ui_close_clicked();

inline void ui_tip_layout();

inline void ui_tooltip_draw();

inline void ui_label(const std::string& label, float64 x, float64 y, float64 h, float64 max_w);

inline float64 ui_label_line(const std::string& label, float64 x, float64 y, float64 w);

inline void ui_tip(const std::string& text);

inline bool ui_badge(const std::string& text, color c);

inline void ui_subheader(const std::string& text);

inline void ui_status_colored(const std::string& text, color c);

inline void ui_status(const std::string& text);

inline void ui_heading(const std::string& text);

inline void ui_separator();

inline void ui_text_colored(const std::string& text, color c);

inline void ui_hint(const std::string& text);

inline void ui_progress_bar(float64 x, float64 y, float64 w, float64 h, float64 frac);

inline void ui_progress(const std::string& text, float64 frac);

inline bool ui_toggle_note(const std::string& label, const std::string& note, bool& v);

inline bool ui_toggle(const std::string& label, bool& v);

inline int32 ui_toggles(const std::vector<std::string>& labels, std::vector<bool>& values, const std::vector<std::string>& tips);

inline float64 ui_on_grid(float64 v, float64 lo, float64 hi, float64 step);

inline int32 ui_step_decimals(float64 step);

inline std::string ui_format_value(float64 v, float64 step);

inline bool ui_slider_track(float64 x, float64 cy, float64 w, float64& v, float64 lo, float64 hi, float64 step, color fill);

inline bool ui_slider(const std::string& label, float64& v, float64 lo, float64 hi, float64 step);

inline bool ui_number(const std::string& label, float64& v, float64 lo, float64 hi, float64 step);

inline bool ui_button(const std::string& label);

inline int32 ui_buttons(const std::vector<std::string>& labels);

inline bool ui_small_button(const std::string& label, float64 x, float64 y, float64 w, float64 h, bool enabled);

inline bool ui_text_button(const std::string& text, const std::string& button, bool enabled);

inline void ui_text_box_wrap(ui_text_box_t& tb, const std::string& text, float64 tw);

inline int32 ui_text_box_scroll(int32 top, int32 max_top, bool over);

inline void ui_text_box(const std::string& id, const std::string& text, const std::string& hint, int32 rows);

inline bool ui_editing();

inline std::string ui_edit_label();

inline void ui_edit_end();

inline void ui_edit_start(const std::string& label, const std::string& text);

inline bool ui_shift_down();

inline std::string ui_text_char(int64 k, bool shift);

inline void ui_edit_key(int64 k);

inline bool ui_key_repeats(int64 k);

inline void ui_edit_repeat(int64 now);

inline bool ui_edit_apply(const std::string& label, float64& v, float64 lo, float64 hi, float64 step);

inline bool ui_value_field(const std::string& label, float64& v, float64 lo, float64 hi, float64 step, float64 x, float64 y,
    float64 w, float64 h, bool framed);

inline void ui_draw_caret(float64 x, float64 y, float64 h);

inline void ui_text_edit_start(const std::string& id, const std::string& text, uint64 max_chars);

inline bool ui_text_input(const std::string& id, std::string& text, const std::string& hint, float64 x, float64 y, float64 w,
    float64 h);

inline bool ui_text_input_max(const std::string& id, std::string& text, const std::string& hint, float64 x, float64 y,
    float64 w, float64 h, uint64 max_chars);

inline bool ui_toggle_text(const std::string& label, bool& v, std::string& text, const std::string& hint, uint64 max_chars);

inline int32 ui_input_row(const std::string& id, std::string& text, const std::string& hint,
    const std::vector<std::string>& buttons);

inline std::string ui_search_row(const std::string& id, std::string& text);

inline std::string ui_key_name(int64 vk);

inline bool ui_capturing();

inline std::string ui_capture_label();

inline bool ui_keys_muted();

inline void ui_keys_mute_tick();

inline int64 ui_capture_read(bool snapshot);

inline void ui_capture_start(const std::string& label);

inline void ui_capture_cancel();

inline void ui_capture_poll(bool live);

inline ui_keybind_t ui_kb_make(int64 vk, int32 mode);

inline bool kb_poll(ui_keybind_t& kb);

inline bool kb_held(const ui_keybind_t& kb);

inline bool kb_active(ui_keybind_t& kb, bool enabled);

inline void ui_bind_flash(const std::string& label, const std::string& text);

inline bool ui_bind_flashing(const std::string& label);

inline void kb_rebind(ui_keybind_t& kb, int64 vk, bool swallow);

inline bool ui_key_allowed(int64 k, int64 current);

inline float64 ui_keybind_width(const std::string& label, const ui_keybind_t& kb, bool modes);

inline bool ui_keybind(const std::string& label, ui_keybind_t& kb, float64 row_y, float64 right_x, bool modes, bool clearable);

inline bool ui_checkbox_bind(const std::string& label, bool& enabled, ui_keybind_t& kb);

inline bool ui_checkbox_key(const std::string& label, bool& enabled, ui_keybind_t& kb, bool modes);

inline bool ui_checkbox_key_pair(const std::string& label, bool& enabled, ui_keybind_t& kb, bool modes, const std::string& tip,
    const std::string& label2, bool& v2, const std::string& tip2);

inline bool ui_label_bind(const std::string& label, ui_keybind_t& kb, bool modes, bool clearable);

inline void ui_popup_close();

inline int32 ui_popup_item();

inline int32 ui_popup_max_top();

inline float64 ui_popup_thumb_h();

inline float64 ui_popup_thumb_y();

inline void ui_popup_input();

inline void ui_popup_set_items(const std::vector<std::string>& items, const std::vector<std::string>& icons, bool multi,
    const std::vector<bool>& marks, int32 mode);

inline void ui_popup_fix_top(const std::vector<bool>& marks, bool multi, int32 mode);

inline void ui_popup_place(const std::vector<std::string>& items, const std::vector<std::string>& icons, bool multi,
    const std::vector<bool>& marks, int32 mode, float64 w, float64 right_x, float64 top, float64 bottom);

inline void ui_popup_menu_open(const std::string& label, float64 x, float64 y);

inline bool ui_popup_menu_is_open(const std::string& label);

inline void ui_popup_menu_move(const std::string& label, float64 x, float64 y);

inline int32 ui_popup_menu_rows(int32 n, float64 ih);

inline vec2 ui_popup_menu_size(const std::vector<std::string>& options);

inline int32 ui_popup_menu(const std::string& label, const std::vector<std::string>& options);

inline int32 ui_popup_menu(const std::string& label, const std::vector<std::string>& options, int32 current);

inline void ui_popup_draw();

inline void ui_draw_combo_arrow(float64 fx, float64 fy, float64 fw, float64 fh, float64 cw, bool open);

inline float64 ui_combo_line_h();

inline float64 ui_combo_need(const std::string& label, const std::vector<std::string>& names, float64 cw);

inline bool ui_combo(const std::string& label, int32& index, const std::vector<std::string>& names);

inline bool ui_combo_icons(const std::string& label, int32& index, const std::vector<std::string>& names,
    const std::vector<std::string>& icons);

inline bool ui_multi_combo(const std::string& label, const std::vector<std::string>& names, std::vector<bool>& picked);

inline bool ui_multi_icons_fit(const std::vector<std::string>& icons, const std::vector<bool>& picked, float64 is,
    float64 avail);

inline bool ui_multi_combo_icons(const std::string& label, const std::vector<std::string>& names,
    const std::vector<std::string>& icons, std::vector<bool>& picked);

inline void ui_multi_tips(const std::string& label, const std::vector<std::string>& tips);

inline bool ui_multi_list(const std::string& label, const std::vector<std::string>& names, const std::vector<std::string>& icons,
    const std::vector<std::string>& notes, std::vector<bool>& picked, int32 rows);

inline uint8 ui_channel(float64 x);

inline color ui_hsv_color(float64 h, float64 s, float64 v, uint8 a);

inline float64 ui_hue_of(color c);

inline void ui_sat_val_of(color c, float64& s, float64& v);

inline void ui_sync_accent();

inline std::string ui_hex_of(color c);

inline bool ui_parse_hex(const std::string& text, color& c);

inline bool ui_picker_is_open(const std::string& label);

inline void ui_picker_set(color c);

inline void ui_draw_checker(ui_checker_t& ck, float64 x, float64 y, float64 w, float64 h, float64 cell);

inline void ui_draw_swatch(float64 x, float64 y, float64 w, float64 h, color c, bool lit, bool open);

inline void ui_picker_open(const std::string& label, color c);

inline bool ui_picker_bind(const std::string& label, color& c, float64 x, float64 y, float64 w, float64 h);

inline void ui_picker_input();

inline void ui_draw_bar_marker(float64 x, float64 y, float64 w);

inline bool ui_picker_drag(int32 id, float64 x, float64 y, float64 w, float64 h);

inline void ui_picker_draw();

inline bool ui_color_picker(const std::string& label, color& c);

inline bool ui_checkbox_color_button(const std::string& label, bool& v, color& c, const std::string& button, bool& clicked);

inline bool ui_checkbox_color(const std::string& label, bool& v, color& c);

inline void ui_spinner(float64 cx, float64 cy, float64 r);

inline void ui_grid_begin();

inline void ui_grid_end();

inline int32 ui_card_picture(const std::string& image, float64 x, float64 y, float64 size);

inline bool ui_card_enabled(const std::string& image, const std::string& title, const std::string& tag, bool selected, bool enabled);

inline bool ui_card(const std::string& image, const std::string& title, const std::string& tag, bool selected);

inline bool ui_float_drag(float64& x, float64& y, float64 w, float64 h, bool interactive);

inline bool ui_float_drag_at(int32 slot, float64& x, float64& y, float64 w, float64 h, bool interactive);

inline void ui_hud_panel(float64 x, float64 y, float64 w, float64 h);

inline void ui_hud_panel_faded(float64 x, float64 y, float64 w, float64 h, float64 f);

inline void ui_wm_pt(float64 x, float64 y);

inline void ui_wm_octagon(float64 x, float64 y, float64 w, float64 h, float64 cut, float64 d);

inline void ui_wm_diamond(float64 cx, float64 cy, float64 rx, float64 ry);

inline vec2 ui_wm_ring_at(uint64 i, bool inner);

inline void ui_wm_ring_quad(std::vector<float32>& out, uint64 i);

inline void ui_wm_border_build(float64 x, float64 y, float64 w, float64 h, float64 cut);

inline void ui_wm_border_draw(float64 cut);

inline void ui_wm_sweep(float64 x, float64 y, float64 w, float64 h, float64 cut);

inline void ui_wm_glow(float64 x, float64 y, float64 w, float64 h, float64 cut);

inline bool ui_wm_sizes_valid(const std::vector<std::string>& parts);

inline void ui_wm_measure(const std::vector<std::string>& parts, float64 base, float64 gap, float64 sep);

inline void ui_wm_layout(float64 x, float64 y, float64 w, float64 h, float64 cut, float64 tx, float64 gap, float64 sep,
                  float64 dot_r);

inline void ui_watermark(const std::vector<std::string>& parts, bool interactive);

inline void ui_watermark_card(const std::vector<std::string>& parts, bool interactive);

inline vec2 ui_tilemap_screen(const ui_tilemap_t& m, float64 mx, float64 my);

inline vec2 ui_tilemap_point(const ui_tilemap_t& m, float64 sx, float64 sy);

inline void ui_tilemap_fix(ui_tilemap_t& m);

inline void ui_tilemap_look(ui_tilemap_t& m, float64 mx, float64 my, float64 scale);

inline void ui_tilemap_zoom(ui_tilemap_t& m, float64 f, float64 sx, float64 sy);

inline void ui_tilemap_fit(ui_tilemap_t& m);

inline int64 ui_tile_key(int32 z, int32 col, int32 row);

inline std::string ui_tile_path(ui_tilemap_t& m, int32 z, int32 col, int32 row);

inline vec2 ui_tilemap_tile_at(const ui_tilemap_t& m, int32 z, int32 col, int32 row);

inline int32 ui_tilemap_level(const ui_tilemap_t& m);

inline void ui_tilemap_stand_in(ui_tilemap_t& m, int32 z, int32 col, int32 row, vec2 pos, vec2 size);

inline void ui_tilemap_tiles(ui_tilemap_t& m, int32 z);

inline bool ui_tilemap_in_hole(const ui_tilemap_t& m);

inline void ui_tilemap_hole(ui_tilemap_t& m, float64 x, float64 y, float64 w, float64 h);

inline void ui_tilemap_clamp(ui_tilemap_t& m);

inline void ui_tilemap(ui_tilemap_t& m, float64 x, float64 y, float64 w, float64 h);

inline bool ui_tilemap_button(ui_tilemap_t& m, int32 glyph, const std::string& tip, bool lit);

inline bool ui_tilemap_corner_button(ui_tilemap_t& m, const std::string& text, const std::string& tip, bool open,
    vec2& top_right);

inline color ui_col_text();

inline color ui_col_dim();

inline color ui_col_muted();

inline color ui_col_deny();

inline color ui_col_warn();

inline color ui_col_active();

inline color ui_col_border();

inline color ui_col_border_hi();

inline color ui_col_bg();

inline color ui_col_bg2();

inline color ui_col_hover();

inline color ui_col_pressed();

inline color ui_col_group();

inline color ui_col_card_top();

inline color ui_col_panel();

inline color ui_col_panel_edge();

inline color ui_col_knob();

inline color ui_col_shadow();

inline color ui_col_wm_dot();

inline color ui_col_check_lo();

inline color ui_col_check_hi();

inline color ui_col_teal();

inline color ui_col_purple();

inline color ui_col_purple_hi();

inline color ui_col_indigo();

inline color ui_col_indigo_hi();

inline std::vector<color> ui_col_gradient();

inline color ui_get_accent();

inline void ui_set_accent(color c);

inline void ui_set_theme(const ui_theme_t& t);

inline float64 ui_m_pad();

inline float64 ui_m_row_h();

inline float64 ui_m_frame_h();

inline float64 ui_m_combo_h();

inline float64 ui_m_item_h();

inline float64 ui_m_wm_h();

inline float64 ui_m_wm_cut();

inline float64 ui_m_wm_sep_r();

inline float64 ui_m_wm_dot_r();

inline float64 ui_m_dot_r();

inline float64 ui_m_accent_w();

inline float64 ui_m_round();

inline float64 ui_m_box();

inline float64 ui_m_side_w();

inline float64 ui_m_head_h();

inline float64 ui_m_foot_h();

inline float64 ui_m_font_px();

inline float64 ui_m_font_small_px();

inline float64 ui_m_stroke();

inline float64 ui_m_line_gap();

inline float64 ui_style_wm_rt();

inline float64 ui_style_wm_gap();

inline float64 ui_style_wm_edge();

inline float64 ui_style_bar_min();

inline float64 ui_style_text_line();

inline float64 ui_scale();

inline void ui_set_scale(float64 s);

inline int32 ui_scale_index_current();

inline int64 ui_font();

inline int64 ui_font_bold();

inline int64 ui_font_small();

inline int64 ui_font_brand();

inline float64 ui_font_h();

inline int64 ui_font_bold_at(int64 px);

inline float64 ui_window_x();

inline float64 ui_window_y();

inline float64 ui_window_w();

inline float64 ui_window_h();

inline void ui_set_window_pos(float64 x, float64 y);

inline void ui_set_window_size(float64 w, float64 h);

inline vec2 ui_mouse();

inline bool ui_mouse_down();

inline bool ui_mouse_pressed();

inline bool ui_mouse_released();

inline int32 ui_active_id();

inline void ui_set_active_id(int32 id);

inline vec2 ui_drag_offset();

inline void ui_set_drag_offset(float64 dx, float64 dy);

inline float64 ui_scroll();

inline void ui_set_tooltip(const std::string& text);

inline bool ui_kb_live();

inline void ui_set_kb_live(bool live);

inline void ui_reserve_keys(const std::vector<int64>& keys);

inline void ui_clear_reserved_keys();

inline float64 ui_cursor();

inline void ui_set_cursor(float64 y);

inline void ui_cursor_advance(float64 dy);

inline void ui_set_fonts(const std::string& regular, const std::string& bold);

inline void ui_set_brand_font(const std::string& path, float64 scale);

inline void ui_set_logo_font(const std::string& path);

inline void ui_set_logo_path(const std::string& path);

inline void ui_set_tooltip_kinds(const std::vector<std::string>& prefixes, const std::vector<std::string>& heads,
                          const std::vector<color>& cols);

inline std::string ui_get_subtitle();

inline void ui_set_watermark_rect(float64 x, float64 y, float64 w, float64 h, bool interactive);

inline bool ui_watermark_placed();

inline void ui_set_watermark_placed(bool p);

inline float64 ui_watermark_x();

inline float64 ui_watermark_y();

inline float64 ui_watermark_w();

inline float64 ui_watermark_h();

inline void ui_feat_add(const ui_feat_t& f);

inline ui_feat_t ui_feat_make(int32 id, const std::string& name, int32 risk, const std::string& what);

inline void ui_feat_add_toggle(int32 id, const std::string& name, int32 risk, const std::string& what, bool* enabled_p,
                        UiFeatStatusFn status_fn);

inline void ui_feat_add_keyed(int32 id, const std::string& name, int32 risk, const std::string& what, bool* enabled_p,
                       ui_keybind_t* bind_p, UiFeatStatusFn status_fn);

inline void ui_feat_add_computed(int32 id, const std::string& name, int32 risk, const std::string& what,
                          UiFeatEnabledFn enabled_fn, ui_keybind_t* bind_p, UiFeatStatusFn status_fn);

inline void ui_feat_add_with_active(int32 id, const std::string& name, int32 risk, const std::string& what, bool* enabled_p,
                             ui_keybind_t* bind_p, UiFeatActiveFn active_fn, UiFeatStatusFn status_fn);

inline int32 ui_feat_count();

inline ui_feat_t* ui_feat_at(int32 index);

inline ui_feat_t* ui_feat_get(int32 id);

inline void ui_feat_set_hud_name(int32 id, const std::string& name);

inline void ui_feat_set_write_note(const std::string& note);

inline int32 ui_feat_risk(int32 id);

inline bool ui_feat_enabled(const ui_feat_t& f);

inline bool ui_kb_lit(const ui_keybind_t& kb, bool enabled);

inline bool ui_kb_held(const ui_keybind_t& kb);

inline bool ui_kb_on(const ui_keybind_t& kb);

inline bool ui_feat_active(const ui_feat_t& f);

inline int32 ui_feat_state(const ui_feat_t& f, bool attached, bool writes);

inline void ui_tip_status(const std::string& tip, const std::string& status);

inline void ui_feat_risk_badge(int32 risk);

inline void ui_feat_tip(int32 id, const std::string& tip, const std::string& status);

inline bool ui_check_key(const std::string& label, bool& enabled, ui_keybind_t& kb, bool modes);

inline bool ui_label_key_switch(const std::string& label, ui_keybind_t& kb, bool modes, bool clearable, bool switch_on);

inline bool ui_label_key(const std::string& label, ui_keybind_t& kb, bool modes, bool clearable);

inline bool ui_label_key_lit(const std::string& label, ui_keybind_t& kb, bool clearable, bool lit);

inline bool ui_feat_row_as(int32 id, const std::string& label, const std::string& tip, const std::string& status);

inline bool ui_feat_row(int32 id, const std::string& tip, const std::string& status);

inline void ui_menu_set_reserved_keys(const std::vector<int64>& keys);

inline void ui_menu_add_bind(ui_keybind_t* kb);

inline std::vector<int64> ui_menu_bind_keys();

inline bool ui_menu_key_reserved(int64 vk);

inline bool ui_menu_key_clashes(int64 vk);

inline bool ui_feat_safe_start();

inline void ui_feat_set_safe_start(bool on);

inline int32 ui_feat_safe_off();

inline int32 ui_feat_apply_safe_start();

inline void ui_feat_safe_start_row(const std::string& label);

inline std::string ui_clock_or(const std::string& fallback);

inline std::string ui_clock();

inline void ui_perf_setup(const std::string& log_path, const std::string& fault_path);

inline int64 ui_perf_ms(int64 ticks);

inline int64 ui_perf_frames();

inline std::string ui_perf_stage_name();

inline void ui_perf_log(const std::string& line);

inline void ui_fault_log(const std::string& line);

inline void ui_perf_stage_end(int64 t);

inline void ui_perf_stage(const std::string& stage);

inline void ui_perf_fault();

inline void ui_perf_frame_begin();

inline void ui_perf_frame_end();

inline void ui_watchdog_setup(UiHeartbeatFn heartbeat_fn, UiInputReleaseFn release_fn, UiStallFn stall_fn);

inline void ui_watchdog_tick();

inline void ui_watch_value(const std::string& name, UiWatchFn fn);

inline int32 ui_watch_count();

inline std::string ui_fmt_onoff(bool v);

inline std::string ui_fmt_f1(float64 v);

inline void ui_session_set_sinks(UiFeatSinkFn feat_fn, UiStatesSinkFn states_fn, UiValuesSinkFn values_fn);

inline void ui_session_begin();

inline void ui_session_tick(int64 now, bool attached, bool writes);

inline void ui_menu_setup(const std::string& title, int64 key_vk, const std::vector<int64>& fallbacks);

inline int32 ui_menu_add_tab(const std::string& name, const std::vector<std::string>& pages, UiTabDrawFn draw_fn);

inline ui_tab_t* ui_menu_tab_at(int32 tab);

inline void ui_menu_tab_cols(int32 tab, const std::vector<int32>& cols);

inline void ui_menu_tab_edits(int32 tab, UiTabFn reset_fn, UiTabFn commit_fn);

inline void ui_menu_tab_tick(int32 tab, UiTabFn tick_fn);

inline void ui_menu_tab_shutdown(int32 tab, UiTabFn shutdown_fn);

inline void ui_menu_set_tab(int32 tab);

inline int32 ui_menu_add_head_button(const std::string& icon, int32 glyph, const std::string& tip, UiMenuHookFn click_fn,
    UiIndicatorFn lit_fn);

inline int32 ui_menu_add_view(const std::string& name, float64 side_w, UiViewCanvasFn canvas_fn, UiTabDrawFn draw_fn);

inline ui_view_t* ui_menu_view_at(int32 view);

inline void ui_menu_view_min_size(int32 view, float64 w, float64 h);

inline void ui_menu_view_side(int32 view, float64 side_w);

inline float64 ui_menu_view_side_w(int32 view);

inline int32 ui_menu_view();

inline void ui_menu_open_view(int32 view);

inline vec2 ui_menu_window_size();

inline void ui_menu_view_fit();

inline void ui_menu_toggle_view(int32 view);

inline void ui_menu_add_indicator(const std::string& on_text, const std::string& off_text, color on_col, UiIndicatorFn fn);

inline void ui_menu_set_watermark(UiWmPartsFn parts_fn);

inline void ui_menu_set_wheel(UiWheelReadFn read_fn, UiWheelFrameFn frame_fn);

inline float64 ui_wheel_none();

inline void ui_menu_on_close(UiMenuHookFn fn);

inline bool ui_menu_is_open();

inline bool ui_menu_is_shown();

inline bool ui_menu_typing();

inline void ui_menu_read_button(int64 vk, ui_menu_button_t& b, bool& down, bool& tap);

inline void ui_menu_button_baseline(int64 vk, ui_menu_button_t& b);

inline void ui_menu_mouse_baseline();

inline float64 ui_menu_wheel();

inline void ui_menu_reset_edits();

inline void ui_menu_commit_edits(bool held);

inline void ui_menu_set_open(bool open);

inline bool ui_menu_key_pressed();

inline bool ui_menu_poll(bool fg);

inline void ui_menu_reserve_keys(bool menu_key_row);

inline int32 ui_menu_page_bar(ui_tab_t& t);

inline int32 ui_menu_cols(const ui_tab_t& t, int32 page);

inline void ui_menu_footer();

inline void ui_menu_draw_tab(int64 now);

inline void ui_menu_draw_view(int64 now);

inline void ui_menu_draw(int64 now);

inline bool ui_menu_hud_rows();

inline void ui_menu_keys_hud(bool interactive);

inline void ui_menu_hud();

inline void ui_menu_tick(int64 now);

inline void ui_menu_scale_row(const std::string& label);

inline void ui_menu_key_row(const std::string& label, const std::string& extra);

inline void ui_menu_hud_toggles();

inline void ui_menu_accent_rows();

inline void ui_menu_fix_key();

inline void ui_menu_shutdown();

inline std::string ui_kv_trim(const std::string& s);

inline int64 ui_kv_index(const std::string& key);

inline void ui_kv_set(const std::string& key, const std::string& value);

inline std::string ui_kv_text();

inline bool ui_kv_load(const std::string& path);

inline bool ui_kv_save(const std::string& path);

inline void ui_cfg_set_io(const ui_cfg_io_t& io);

inline void ui_cfg_clear();

inline void ui_cfg_set(const std::string& key, const std::string& value);

inline std::string ui_cfg_get(const std::string& key);

inline std::string ui_cfg_text();

inline std::string ui_cfg_error();

inline int64 ui_cfg_count();

inline bool ui_cfg_file_save();

inline bool ui_cfg_file_load();

inline void ui_cfg_pairs(std::vector<std::string>& keys, std::vector<std::string>& vals);

inline void ui_cfg_keep_order(const std::vector<std::string>& order);

inline bool ui_parse_float(const std::string& s, float64& out);

inline void ui_cfg_set_bool(const std::string& key, bool v);

inline void ui_cfg_set_int(const std::string& key, int64 v);

inline void ui_cfg_set_float(const std::string& key, float64 v);

inline bool ui_cfg_bool(const std::string& key, bool def);

inline int64 ui_cfg_int(const std::string& key, int64 def);

inline float64 ui_cfg_float(const std::string& key, float64 def);

inline int64 ui_cfg_int_in(const std::string& key, int64 def, int64 lo, int64 hi);

inline float64 ui_cfg_float_in(const std::string& key, float64 def, float64 lo, float64 hi);

inline void ui_cfg_set_color(const std::string& key, const color& c);

inline void ui_cfg_get_color(const std::string& key, color& c);

inline void ui_cfg_set_bind(const std::string& name, const ui_keybind_t& kb);

inline void ui_cfg_get_bind(const std::string& name, ui_keybind_t& kb, int32 fixed_mode, bool clearable);

inline void ui_cfg_store_feats(const std::string& prefix);

inline void ui_cfg_apply_feats(const std::string& prefix);

inline void ui_cfg_store_menu();

inline void ui_cfg_apply_menu();

inline void ui_cfg_setup(const std::string& path, UiCfgHookFn store_fn, UiCfgHookFn apply_fn);

inline std::string ui_cfg_path();

inline bool ui_cfg_autosave();

inline void ui_cfg_set_status(const std::string& status);

inline void ui_cfg_store();

inline void ui_cfg_note_save(bool ok, const std::string& what);

inline bool ui_cfg_save();

inline void ui_cfg_save_if_changed();

inline void ui_cfg_apply();

inline bool ui_cfg_load();

inline void ui_cfg_tick(int64 now);

inline void ui_cfg_rows();

template <typename T>
inline int64 ui_mb_vec(const std::vector<T>& v);

template <typename T>
inline void ui_free_vec(std::vector<T>& v);

inline int64 ui_mb_vstr(const std::vector<std::string>& v);

template <typename K, typename V>
inline int64 ui_mb_map(const std::map<K, V>& m);

template <typename V>
inline int64 ui_mb_map_sk(const std::map<std::string, V>& m);

template <typename K>
inline int64 ui_mb_map_sv(const std::map<K, std::string>& m);

inline int64 ui_mb_map_ss(const std::map<std::string, std::string>& m);

template <typename T>
inline int64 ui_mb_set(const std::set<T>& s);

inline int64 ui_mb_set_s(const std::set<std::string>& s);

inline int64 ui_kit_font_count();

inline void ui_kit_census(int64& entries, int64& bytes);

inline void ui_kit_free_all();
