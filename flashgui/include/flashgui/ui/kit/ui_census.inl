template <typename T>
inline int64 ui_mb_vec(const std::vector<T>& v) { return static_cast<int64>(v.capacity()) * static_cast<int64>(sizeof(T)); }

template <typename T>
inline void ui_free_vec(std::vector<T>& v) { std::vector<T>().swap(v); }

inline int64 ui_mb_vstr(const std::vector<std::string>& v) {
    int64 b = static_cast<int64>(v.capacity()) * static_cast<int64>(sizeof(std::string));
    for (const std::string& s : v)
        b = b + static_cast<int64>(s.size());
    return b;
}

template <typename K, typename V>
inline int64 ui_mb_map(const std::map<K, V>& m) {
    return static_cast<int64>(m.size()) * (static_cast<int64>(sizeof(K)) + static_cast<int64>(sizeof(V)) + 48);
}

template <typename V>
inline int64 ui_mb_map_sk(const std::map<std::string, V>& m) {
    int64 b = static_cast<int64>(m.size()) * (static_cast<int64>(sizeof(std::string)) + static_cast<int64>(sizeof(V)) + 48);
    for (const auto& kv : m)
        b = b + static_cast<int64>(kv.first.size());
    return b;
}

template <typename K>
inline int64 ui_mb_map_sv(const std::map<K, std::string>& m) {
    int64 b = static_cast<int64>(m.size()) * (static_cast<int64>(sizeof(K)) + static_cast<int64>(sizeof(std::string)) + 48);
    for (const auto& kv : m)
        b = b + static_cast<int64>(kv.second.size());
    return b;
}

inline int64 ui_mb_map_ss(const std::map<std::string, std::string>& m) {
    int64 b = static_cast<int64>(m.size()) * (2 * static_cast<int64>(sizeof(std::string)) + 48);
    for (const auto& kv : m)
        b = b + static_cast<int64>(kv.first.size()) + static_cast<int64>(kv.second.size());
    return b;
}

template <typename T>
inline int64 ui_mb_set(const std::set<T>& s) { return static_cast<int64>(s.size()) * (static_cast<int64>(sizeof(T)) + 48); }

inline int64 ui_mb_set_s(const std::set<std::string>& s) {
    int64 b = static_cast<int64>(s.size()) * (static_cast<int64>(sizeof(std::string)) + 48);
    for (const std::string& x : s)
        b = b + static_cast<int64>(x.size());
    return b;
}

inline int64 ui_kit_font_count() {
    return static_cast<int64>(g_ui_fonts.size() + g_ui_fonts_bold.size() + g_ui_fonts_brand.size()
        + g_ui_fonts_logo.size());
}

inline void ui_kit_census(int64& entries, int64& bytes) {
    int64 e = 0;
    int64 b = 0;
    // ui/kit/ui_color.em
    // ui_pk
    e = e + static_cast<int64>(g_ui_pk.label.size() + g_ui_pk.hex.size() + g_ui_pk.alpha_ck.cells.size() + g_ui_pk.preview_ck.cells.size());
    b = b + (static_cast<int64>(g_ui_pk.label.size()) + static_cast<int64>(g_ui_pk.hex.size()) + ui_mb_vec(g_ui_pk.alpha_ck.cells) + ui_mb_vec(g_ui_pk.preview_ck.cells));
    // ui/kit/ui_core.em
    // ui_icon
    e = e + static_cast<int64>(g_ui_icon.key.size() + g_ui_icon.path.size());
    b = b + (static_cast<int64>(g_ui_icon.key.size()) + static_cast<int64>(g_ui_icon.path.size()));
    // ui_no_icons
    e = e + static_cast<int64>(g_ui_no_icons.size());
    b = b + (ui_mb_vstr(g_ui_no_icons));
    // ui_no_marks
    e = e + static_cast<int64>(g_ui_no_marks.size());
    b = b + (ui_mb_vec(g_ui_no_marks));
    // ui_wraps
    e = e + static_cast<int64>(g_ui_wraps.size());
    b = b + (ui_mb_vec(g_ui_wraps));
    // ui/kit/ui_hud.em
    // ui_floats
    e = e + static_cast<int64>(g_ui_floats.size());
    b = b + (ui_mb_vec(g_ui_floats));
    // ui_hud_grad
    e = e + static_cast<int64>(g_ui_hud_grad.size());
    b = b + (ui_mb_vec(g_ui_hud_grad));
    // ui_wm_card_parts
    e = e + static_cast<int64>(g_ui_wm_card_parts.size());
    b = b + (ui_mb_vstr(g_ui_wm_card_parts));
    // ui_wm_card_pw
    e = e + static_cast<int64>(g_ui_wm_card_pw.size());
    b = b + (ui_mb_vec(g_ui_wm_card_pw));
    // ui_wm_card_tx
    e = e + static_cast<int64>(g_ui_wm_card_tx.size());
    b = b + (ui_mb_vec(g_ui_wm_card_tx));
    // ui_wm_s
    e = e + static_cast<int64>(g_ui_wm_s.pw.size() + g_ui_wm_s.ph.size() + g_ui_wm_s.pts.size() + g_ui_wm_s.ring.size() + g_ui_wm_s.sweep.size() + g_ui_wm_s.parts.size() + g_ui_wm_s.fill.size() + g_ui_wm_s.q1.size() + g_ui_wm_s.q3.size() + g_ui_wm_s.q5.size() + g_ui_wm_s.q7.size() + g_ui_wm_s.sides.size() + g_ui_wm_s.side_cols.size() + g_ui_wm_s.seps.size() + g_ui_wm_s.tx.size() + g_ui_wm_s.ty.size());
    b = b + (ui_mb_vec(g_ui_wm_s.pw) + ui_mb_vec(g_ui_wm_s.ph) + ui_mb_vec(g_ui_wm_s.pts) + ui_mb_vec(g_ui_wm_s.ring) + ui_mb_vec(g_ui_wm_s.sweep) + ui_mb_vstr(g_ui_wm_s.parts) + ui_mb_vec(g_ui_wm_s.fill) + ui_mb_vec(g_ui_wm_s.q1) + ui_mb_vec(g_ui_wm_s.q3) + ui_mb_vec(g_ui_wm_s.q5) + ui_mb_vec(g_ui_wm_s.q7) + ui_mb_vec(g_ui_wm_s.sides) + ui_mb_vec(g_ui_wm_s.side_cols) + ui_mb_vec(g_ui_wm_s.seps) + ui_mb_vec(g_ui_wm_s.tx) + ui_mb_vec(g_ui_wm_s.ty));
    // ui/kit/ui_input.em
    // ui_cap
    e = e + static_cast<int64>(g_ui_cap.label.size() + g_ui_cap.bits.size() + g_ui_cap.bits_seen.size() + g_ui_cap.down.size() + g_ui_cap.api.size() + g_ui_cap.hits.size() + g_ui_cap.flash_label.size() + g_ui_cap.flash.size());
    b = b + (static_cast<int64>(g_ui_cap.label.size()) + ui_mb_vec(g_ui_cap.bits) + ui_mb_vec(g_ui_cap.bits_seen) + ui_mb_vec(g_ui_cap.down) + ui_mb_vec(g_ui_cap.api) + ui_mb_vec(g_ui_cap.hits) + static_cast<int64>(g_ui_cap.flash_label.size()) + static_cast<int64>(g_ui_cap.flash.size()));
    // ui_edit
    e = e + static_cast<int64>(g_ui_edit.label.size() + g_ui_edit.text.size());
    b = b + (static_cast<int64>(g_ui_edit.label.size()) + static_cast<int64>(g_ui_edit.text.size()));
    // ui_key_text
    e = e + static_cast<int64>(g_ui_key_text.size());
    b = b + (ui_mb_map_sk(g_ui_key_text));
    // ui_keys_taken
    e = e + static_cast<int64>(g_ui_keys_taken.size());
    b = b + (ui_mb_vec(g_ui_keys_taken));
    // ui/kit/ui_menu.em
    // ui_feats
    e = e + static_cast<int64>(g_ui_feats.list.size() + g_ui_feats.write_note.size() + g_ui_feats.reserved.size() + g_ui_feats.extra_binds.size());
    b = b + (ui_mb_vec(g_ui_feats.list) + static_cast<int64>(g_ui_feats.write_note.size()) + ui_mb_vec(g_ui_feats.reserved) + ui_mb_vec(g_ui_feats.extra_binds));
    // ui/kit/ui_menu_cfg.em
    // ui_cfgs
    e = e + static_cast<int64>(g_ui_cfgs.path.size() + g_ui_cfgs.text.size() + g_ui_cfgs.last_save.size() + g_ui_cfgs.status.size());
    b = b + (static_cast<int64>(g_ui_cfgs.path.size()) + static_cast<int64>(g_ui_cfgs.text.size()) + static_cast<int64>(g_ui_cfgs.last_save.size()) + static_cast<int64>(g_ui_cfgs.status.size()));
    // ui_kv
    e = e + static_cast<int64>(g_ui_kv.keys.size() + g_ui_kv.vals.size() + g_ui_kv.error.size());
    b = b + (ui_mb_vstr(g_ui_kv.keys) + ui_mb_vstr(g_ui_kv.vals) + static_cast<int64>(g_ui_kv.error.size()));
    // ui/kit/ui_menu_log.em
    // ui_perf
    e = e + static_cast<int64>(g_ui_perf.log_path.size() + g_ui_perf.fault_path.size() + g_ui_perf.stage.size() + g_ui_perf.slow_stage.size());
    b = b + (static_cast<int64>(g_ui_perf.log_path.size()) + static_cast<int64>(g_ui_perf.fault_path.size()) + static_cast<int64>(g_ui_perf.stage.size()) + static_cast<int64>(g_ui_perf.slow_stage.size()));
    // ui_session
    e = e + static_cast<int64>(g_ui_session.watch.size() + g_ui_session.states.size() + g_ui_session.names.size() + g_ui_session.values.size());
    b = b + (ui_mb_vec(g_ui_session.watch) + ui_mb_vec(g_ui_session.states) + ui_mb_vstr(g_ui_session.names) + ui_mb_vstr(g_ui_session.values));
    // ui/kit/ui_menu_shell.em
    // ui_menu
    e = e + static_cast<int64>(g_ui_menu.title.size() + g_ui_menu.key_fallbacks.size() + g_ui_menu.edit.size() + g_ui_menu.capture.size() + g_ui_menu.tabs.size() + g_ui_menu.page_one.size() + g_ui_menu.heads.size() + g_ui_menu.views.size() + g_ui_menu.indicators.size() + g_ui_menu.foot_texts.size() + g_ui_menu.foot_cols.size() + g_ui_menu.wm_parts.size() + g_ui_menu.hud_names.size() + g_ui_menu.hud_keys.size());
    b = b + (static_cast<int64>(g_ui_menu.title.size()) + ui_mb_vec(g_ui_menu.key_fallbacks) + static_cast<int64>(g_ui_menu.edit.size()) + static_cast<int64>(g_ui_menu.capture.size()) + ui_mb_vec(g_ui_menu.tabs) + ui_mb_vstr(g_ui_menu.page_one) + ui_mb_vec(g_ui_menu.heads) + ui_mb_vec(g_ui_menu.views) + ui_mb_vec(g_ui_menu.indicators) + ui_mb_vstr(g_ui_menu.foot_texts) + ui_mb_vec(g_ui_menu.foot_cols) + ui_mb_vstr(g_ui_menu.wm_parts) + ui_mb_vstr(g_ui_menu.hud_names) + ui_mb_vstr(g_ui_menu.hud_keys));
    // ui/kit/ui_popup.em
    // ui_list_top
    e = e + static_cast<int64>(g_ui_list_top.size());
    b = b + (ui_mb_map_sk(g_ui_list_top));
    // ui_pop
    e = e + static_cast<int64>(g_ui_pop.label.size() + g_ui_pop.items.size() + g_ui_pop.icons.size() + g_ui_pop.marks.size() + g_ui_pop.prev.size() + g_ui_pop.placed.size() + g_ui_pop.tips.size() + g_ui_pop.tips_label.size());
    b = b + (static_cast<int64>(g_ui_pop.label.size()) + ui_mb_vstr(g_ui_pop.items) + ui_mb_vstr(g_ui_pop.icons) + ui_mb_vec(g_ui_pop.marks) + static_cast<int64>(g_ui_pop.prev.size()) + static_cast<int64>(g_ui_pop.placed.size()) + ui_mb_vstr(g_ui_pop.tips) + static_cast<int64>(g_ui_pop.tips_label.size()));
    // ui_pop_tops
    e = e + static_cast<int64>(g_ui_pop_tops.size());
    b = b + (ui_mb_map_sk(g_ui_pop_tops));
    // ui/kit/ui_style.em
    // ui_cfg
    e = e + static_cast<int64>(g_ui_cfg.font_path.size() + g_ui_cfg.font_bold_path.size() + g_ui_cfg.font_brand_path.size() + g_ui_cfg.font_logo_path.size() + g_ui_cfg.logo_path.size() + g_ui_cfg.subtitle.size() + g_ui_cfg.tip_prefixes.size() + g_ui_cfg.tip_heads.size() + g_ui_cfg.tip_cols.size());
    b = b + (static_cast<int64>(g_ui_cfg.font_path.size()) + static_cast<int64>(g_ui_cfg.font_bold_path.size()) + static_cast<int64>(g_ui_cfg.font_brand_path.size()) + static_cast<int64>(g_ui_cfg.font_logo_path.size()) + static_cast<int64>(g_ui_cfg.logo_path.size()) + static_cast<int64>(g_ui_cfg.subtitle.size()) + ui_mb_vstr(g_ui_cfg.tip_prefixes) + ui_mb_vstr(g_ui_cfg.tip_heads) + ui_mb_vec(g_ui_cfg.tip_cols));
    // ui_fonts
    e = e + static_cast<int64>(g_ui_fonts.size());
    b = b + (ui_mb_map(g_ui_fonts));
    // ui_fonts_bold
    e = e + static_cast<int64>(g_ui_fonts_bold.size());
    b = b + (ui_mb_map(g_ui_fonts_bold));
    // ui_fonts_brand
    e = e + static_cast<int64>(g_ui_fonts_brand.size());
    b = b + (ui_mb_map(g_ui_fonts_brand));
    // ui_fonts_logo
    e = e + static_cast<int64>(g_ui_fonts_logo.size());
    b = b + (ui_mb_map(g_ui_fonts_logo));
    // ui_style
    e = e + static_cast<int64>(g_ui_style.check_mark.size());
    b = b + (ui_mb_vec(g_ui_style.check_mark));
    // ui/kit/ui_theme.em
    // ui_theme
    e = e + static_cast<int64>(g_ui_theme.gradient.size());
    b = b + (ui_mb_vec(g_ui_theme.gradient));
    // ui/kit/ui_widgets.em
    // ui_fill_grad
    e = e + static_cast<int64>(g_ui_fill_grad.size());
    b = b + (ui_mb_vec(g_ui_fill_grad));
    // ui_text_boxes
    e = e + static_cast<int64>(g_ui_text_boxes.size());
    b = b + (ui_mb_map_sk(g_ui_text_boxes));
    // ui/kit/ui_window.em
    // ui_body
    e = e + static_cast<int64>(g_ui_body.col_y.size());
    b = b + (ui_mb_vec(g_ui_body.col_y));
    // ui_group_closed
    e = e + static_cast<int64>(g_ui_group_closed.size());
    b = b + (ui_mb_map_sk(g_ui_group_closed));
    // ui_group_h
    e = e + static_cast<int64>(g_ui_group_h.size());
    b = b + (ui_mb_map_sk(g_ui_group_h));
    // ui_grp
    e = e + static_cast<int64>(g_ui_grp.title.size());
    b = b + (static_cast<int64>(g_ui_grp.title.size()));
    // ui_page_off
    e = e + static_cast<int64>(g_ui_page_off.size());
    b = b + (ui_mb_map(g_ui_page_off));
    // ui_tab_w
    e = e + static_cast<int64>(g_ui_tab_w.size());
    b = b + (ui_mb_vec(g_ui_tab_w));
    // ui_tip
    e = e + static_cast<int64>(g_ui_tip.text.size() + g_ui_tip.wrapped.size() + g_ui_tip.lines.size());
    b = b + (static_cast<int64>(g_ui_tip.text.size()) + static_cast<int64>(g_ui_tip.wrapped.size()) + ui_mb_vstr(g_ui_tip.lines));
    entries = e;
    bytes = b;
}

inline void ui_kit_free_all() {
    ui_pic_shutdown();
    // The worker / job slots too: unload is final and the host already stopped the worker routines.
    ui_free_vec(g_ui_pics.jobs);
    ui_free_vec(g_ui_pics.workers);
    g_ui_pics.bad.clear();
    try {
    // ui/kit/ui_color.em
    g_ui_pk.label.clear();
    g_ui_pk.hex.clear();
    ui_free_vec(g_ui_pk.alpha_ck.cells);
    ui_free_vec(g_ui_pk.preview_ck.cells);
    // ui/kit/ui_core.em
    g_ui_icon.key.clear();
    g_ui_icon.path.clear();
    ui_free_vec(g_ui_no_icons);
    ui_free_vec(g_ui_no_marks);
    ui_free_vec(g_ui_wraps);
    // ui/kit/ui_hud.em
    ui_free_vec(g_ui_floats);
    ui_free_vec(g_ui_hud_grad);
    ui_free_vec(g_ui_wm_card_parts);
    ui_free_vec(g_ui_wm_card_pw);
    ui_free_vec(g_ui_wm_card_tx);
    ui_free_vec(g_ui_wm_s.pw);
    ui_free_vec(g_ui_wm_s.ph);
    ui_free_vec(g_ui_wm_s.pts);
    ui_free_vec(g_ui_wm_s.ring);
    ui_free_vec(g_ui_wm_s.sweep);
    ui_free_vec(g_ui_wm_s.parts);
    ui_free_vec(g_ui_wm_s.fill);
    ui_free_vec(g_ui_wm_s.q1);
    ui_free_vec(g_ui_wm_s.q3);
    ui_free_vec(g_ui_wm_s.q5);
    ui_free_vec(g_ui_wm_s.q7);
    ui_free_vec(g_ui_wm_s.sides);
    ui_free_vec(g_ui_wm_s.side_cols);
    ui_free_vec(g_ui_wm_s.seps);
    ui_free_vec(g_ui_wm_s.tx);
    ui_free_vec(g_ui_wm_s.ty);
    // ui/kit/ui_input.em
    g_ui_cap.label.clear();
    ui_free_vec(g_ui_cap.bits);
    ui_free_vec(g_ui_cap.bits_seen);
    ui_free_vec(g_ui_cap.down);
    ui_free_vec(g_ui_cap.api);
    ui_free_vec(g_ui_cap.hits);
    g_ui_cap.flash_label.clear();
    g_ui_cap.flash.clear();
    g_ui_edit.label.clear();
    g_ui_edit.text.clear();
    g_ui_key_text.clear();
    ui_free_vec(g_ui_keys_taken);
    // ui/kit/ui_menu.em
    ui_free_vec(g_ui_feats.list);
    g_ui_feats.write_note.clear();
    ui_free_vec(g_ui_feats.reserved);
    ui_free_vec(g_ui_feats.extra_binds);
    // ui/kit/ui_menu_cfg.em
    g_ui_cfgs.path.clear();
    g_ui_cfgs.text.clear();
    g_ui_cfgs.last_save.clear();
    g_ui_cfgs.status.clear();
    ui_free_vec(g_ui_kv.keys);
    ui_free_vec(g_ui_kv.vals);
    g_ui_kv.error.clear();
    // ui/kit/ui_menu_log.em
    g_ui_perf.log_path.clear();
    g_ui_perf.fault_path.clear();
    g_ui_perf.stage.clear();
    g_ui_perf.slow_stage.clear();
    ui_free_vec(g_ui_session.watch);
    ui_free_vec(g_ui_session.states);
    ui_free_vec(g_ui_session.names);
    ui_free_vec(g_ui_session.values);
    // ui/kit/ui_menu_shell.em
    g_ui_menu.title.clear();
    ui_free_vec(g_ui_menu.key_fallbacks);
    g_ui_menu.edit.clear();
    g_ui_menu.capture.clear();
    ui_free_vec(g_ui_menu.tabs);
    ui_free_vec(g_ui_menu.page_one);
    ui_free_vec(g_ui_menu.heads);
    ui_free_vec(g_ui_menu.views);
    ui_free_vec(g_ui_menu.indicators);
    ui_free_vec(g_ui_menu.foot_texts);
    ui_free_vec(g_ui_menu.foot_cols);
    ui_free_vec(g_ui_menu.wm_parts);
    ui_free_vec(g_ui_menu.hud_names);
    ui_free_vec(g_ui_menu.hud_keys);
    // ui/kit/ui_popup.em
    g_ui_list_top.clear();
    g_ui_pop.label.clear();
    ui_free_vec(g_ui_pop.items);
    ui_free_vec(g_ui_pop.icons);
    ui_free_vec(g_ui_pop.marks);
    g_ui_pop.prev.clear();
    g_ui_pop.placed.clear();
    ui_free_vec(g_ui_pop.tips);
    g_ui_pop.tips_label.clear();
    g_ui_pop_tops.clear();
    // ui/kit/ui_style.em
    g_ui_cfg.font_path.clear();
    g_ui_cfg.font_bold_path.clear();
    g_ui_cfg.font_brand_path.clear();
    g_ui_cfg.font_logo_path.clear();
    g_ui_cfg.logo_path.clear();
    g_ui_cfg.subtitle.clear();
    ui_free_vec(g_ui_cfg.tip_prefixes);
    ui_free_vec(g_ui_cfg.tip_heads);
    ui_free_vec(g_ui_cfg.tip_cols);
    g_ui_fonts.clear();
    g_ui_fonts_bold.clear();
    g_ui_fonts_brand.clear();
    g_ui_fonts_logo.clear();
    ui_free_vec(g_ui_style.check_mark);
    // ui/kit/ui_theme.em
    ui_free_vec(g_ui_theme.gradient);
    // ui/kit/ui_widgets.em
    ui_free_vec(g_ui_fill_grad);
    g_ui_text_boxes.clear();
    // ui/kit/ui_window.em
    ui_free_vec(g_ui_body.col_y);
    g_ui_group_closed.clear();
    g_ui_group_h.clear();
    g_ui_grp.title.clear();
    g_ui_page_off.clear();
    ui_free_vec(g_ui_tab_w);
    g_ui_tip.text.clear();
    g_ui_tip.wrapped.clear();
    ui_free_vec(g_ui_tip.lines);
    } catch (std::exception& e) {
    } catch (...) {
    }
}
