inline void ui_menu_setup(const std::string& title, int64 key_vk, const std::vector<int64>& fallbacks) {
    g_ui_menu.title = title;
    g_ui_menu.key = ui_kb_make(key_vk, UI_KB_TOGGLE);
    g_ui_menu.key_fallbacks = fallbacks;
}

inline int32 ui_menu_add_tab(const std::string& name, const std::vector<std::string>& pages, UiTabDrawFn draw_fn) {
    ui_tab_t t;
    t.name = name;
    t.pages = pages;
    t.stage = "menu_overlay/tab_" + ui_to_lower(name);
    t.draw_fn = draw_fn;
    g_ui_menu.tabs.push_back(t);
    return static_cast<int32>(g_ui_menu.tabs.size()) - 1;
}

inline ui_tab_t* ui_menu_tab_at(int32 tab) {
    if (tab < 0 || static_cast<uint64>(tab) >= g_ui_menu.tabs.size())
        return nullptr;
    return &g_ui_menu.tabs[static_cast<uint64>(tab)];
}

inline void ui_menu_tab_cols(int32 tab, const std::vector<int32>& cols) {
    ui_tab_t* t = ui_menu_tab_at(tab);
    if (t != nullptr)
        t->cols = cols;
}

inline void ui_menu_tab_edits(int32 tab, UiTabFn reset_fn, UiTabFn commit_fn) {
    ui_tab_t* t = ui_menu_tab_at(tab);
    if (t == nullptr)
        return;
    t->reset_fn = reset_fn;
    t->commit_fn = commit_fn;
}

inline void ui_menu_tab_tick(int32 tab, UiTabFn tick_fn) {
    ui_tab_t* t = ui_menu_tab_at(tab);
    if (t != nullptr)
        t->tick_fn = tick_fn;
}

inline void ui_menu_tab_shutdown(int32 tab, UiTabFn shutdown_fn) {
    ui_tab_t* t = ui_menu_tab_at(tab);
    if (t != nullptr)
        t->shutdown_fn = shutdown_fn;
}

inline void ui_menu_set_tab(int32 tab) {
    g_ui_menu.tab = tab;
}

inline int32 ui_menu_add_head_button(const std::string& icon, int32 glyph, const std::string& tip, UiMenuHookFn click_fn, UiIndicatorFn lit_fn) {
    ui_menu_head_t h;
    h.icon = icon;
    h.glyph = glyph;
    h.tip = tip;
    h.click_fn = click_fn;
    h.lit_fn = lit_fn;
    g_ui_menu.heads.push_back(h);
    return static_cast<int32>(g_ui_menu.heads.size()) - 1;
}

inline int32 ui_menu_add_view(const std::string& name, float64 side_w, UiViewCanvasFn canvas_fn, UiTabDrawFn draw_fn) {
    ui_view_t v;
    v.name = name;
    v.stage = "menu_overlay/view_" + ui_to_lower(name);
    v.side_w = side_w;
    v.canvas_fn = canvas_fn;
    v.draw_fn = draw_fn;
    g_ui_menu.views.push_back(v);
    return static_cast<int32>(g_ui_menu.views.size()) - 1;
}

inline ui_view_t* ui_menu_view_at(int32 view) {
    if (view < 0 || static_cast<uint64>(view) >= g_ui_menu.views.size())
        return nullptr;
    return &g_ui_menu.views[static_cast<uint64>(view)];
}

inline void ui_menu_view_min_size(int32 view, float64 w, float64 h) {
    ui_view_t* v = ui_menu_view_at(view);
    if (v == nullptr)
        return;
    v->min_w = w;
    v->min_h = h;
}

inline void ui_menu_view_side(int32 view, float64 side_w) {
    ui_view_t* v = ui_menu_view_at(view);
    if (v != nullptr)
        v->side_w = side_w;
}

inline float64 ui_menu_view_side_w(int32 view) {
    ui_view_t* v = ui_menu_view_at(view);
    return v != nullptr ? v->side_w : 0.0;
}

inline int32 ui_menu_view() {
    return g_ui_menu.view;
}

inline void ui_menu_open_view(int32 view) {
    g_ui_menu.view = ui_menu_view_at(view) != nullptr ? view : -1;
}

inline vec2 ui_menu_window_size() {
    ui_menu_t& m = g_ui_menu;
    if (m.grown && g_ui_w == m.grown_w && g_ui_h == m.grown_h)
        return vec2(m.grow_from_w, m.grow_from_h);
    return vec2(g_ui_w, g_ui_h);
}

inline void ui_menu_view_fit() {
    ui_menu_t& m = g_ui_menu;
    if (m.sized_for == m.view)
        return;
    m.sized_for = m.view;
    vec2 own = ui_menu_window_size();
    g_ui_w = own.x;
    g_ui_h = own.y;
    m.grown = false;
    ui_view_t* v = ui_menu_view_at(m.view);
    if (v == nullptr)
        return;
    float64 s = g_ui_scale_applied > 0.0 ? g_ui_scale_applied : g_ui_scale;
    vec2 screen = ui_gfx_view();
    float64 w = max(g_ui_w, v->min_w * s);
    float64 h = max(g_ui_h, v->min_h * s);
    if (screen.x > 0.0 && screen.y > 0.0) {
        w = min(w, max(screen.x, g_ui_w));
        h = min(h, max(screen.y, g_ui_h));
    }
    w = ui_snap(w);
    h = ui_snap(h);
    if (w == g_ui_w && h == g_ui_h)
        return;
    m.grown = true;
    m.grow_from_w = g_ui_w;
    m.grow_from_h = g_ui_h;
    g_ui_w = w;
    g_ui_h = h;
    m.grown_w = w;
    m.grown_h = h;
    // ui_begin moves the whole window back on screen.
    g_ui_refit = true;
}

inline void ui_menu_toggle_view(int32 view) {
    ui_menu_open_view(g_ui_menu.view == view ? -1 : view);
}

inline void ui_menu_add_indicator(const std::string& on_text, const std::string& off_text, color on_col, UiIndicatorFn fn) {
    ui_indicator_t d;
    d.on_text = on_text;
    d.off_text = off_text;
    d.on_col = on_col;
    d.fn = fn;
    g_ui_menu.indicators.push_back(d);
}

inline void ui_menu_set_watermark(UiWmPartsFn parts_fn) {
    g_ui_menu.wm_fn = parts_fn;
}

inline void ui_menu_set_wheel(UiWheelReadFn read_fn, UiWheelFrameFn frame_fn) {
    g_ui_menu.wheel_fn = read_fn;
    g_ui_menu.wheel_frame_fn = frame_fn;
}

inline float64 ui_wheel_none() {
    return std::bit_cast<float64>(static_cast<uint64>(0x7FF8000000000000));
}

inline void ui_menu_on_close(UiMenuHookFn fn) {
    g_ui_menu.close_fn = fn;
}

inline bool ui_menu_is_open() {
    return g_ui_menu.open;
}

inline bool ui_menu_is_shown() {
    return g_ui_menu.shown;
}

inline bool ui_menu_typing() {
    return g_ui_menu.shown && (g_ui_menu.edit.size() != 0 || g_ui_menu.capture.size() != 0);
}

inline void ui_menu_read_button(int64 vk, ui_menu_button_t& b, bool& down, bool& tap) {
    tap = false;
    uint8 bits = 0;
    if (!ui_in_key_reader() || !ui_in_key_bits_reader(vk, bits)) {
        down = ui_in_key_down_api(vk);
        b.valid = false;
        return;
    }
    down = (bits & UI_KEY_DOWN) != 0;
    bool toggle = (bits & UI_KEY_TOGGLE) != 0;
    tap = b.valid && toggle != b.toggle && !down && !b.down;
    b.toggle = toggle;
    b.down = down;
    b.valid = true;
}

inline void ui_menu_button_baseline(int64 vk, ui_menu_button_t& b) {
    uint8 bits = 0;
    if (!ui_in_key_reader() || !ui_in_key_bits_reader(vk, bits)) {
        b.down = ui_in_key_down_api(vk);
        b.valid = false;
        return;
    }
    b.valid = true;
    b.down = (bits & UI_KEY_DOWN) != 0;
    b.toggle = (bits & UI_KEY_TOGGLE) != 0;
}

inline void ui_menu_mouse_baseline() {
    ui_menu_button_baseline(vk::lbutton, g_ui_menu.lb);
    ui_menu_button_baseline(vk::rbutton, g_ui_menu.rb);
    ui_reset_press(g_ui_menu.lb.down, g_ui_menu.rb.down);
}

inline float64 ui_menu_wheel() {
    ui_menu_t& m = g_ui_menu;
    float64 d = ui_in_wheel();
    if (d != 0.0 || m.wheel_fn == nullptr)
        return d;
    float64 s = m.wheel_fn();
    if (std::isnan(s))
        return d;
    float64 last = m.wheel_last;
    m.wheel_last = s;
    if (!(s > 0.0 || s < 0.0))
        return 0.0;
    int64 f = m.wheel_frame_fn != nullptr ? m.wheel_frame_fn() : -1;
    if (f < 0)
        return s != last ? s : 0.0;
    if (f == m.wheel_frame)
        return 0.0;
    m.wheel_frame = f;
    return s;
}

inline void ui_menu_reset_edits() {
    for (const ui_tab_t& t : g_ui_menu.tabs) {
        if (t.reset_fn != nullptr)
            t.reset_fn();
    }
}

inline void ui_menu_commit_edits(bool held) {
    if (held)
        return;
    for (const ui_tab_t& t : g_ui_menu.tabs) {
        if (t.commit_fn != nullptr)
            t.commit_fn();
    }
}

inline void ui_menu_set_open(bool open) {
    ui_menu_t& m = g_ui_menu;
    m.open = open;
    if (open) {
        ui_menu_mouse_baseline();
        return;
    }
    // A drag in progress is committed rather than lost; a typed edit or a key capture ends with the menu.
    ui_edit_end();
    ui_capture_cancel();
    ui_menu_commit_edits(false);
    m.edit = "";
    m.capture = "";
    if (m.close_fn != nullptr)
        m.close_fn();
}

inline bool ui_menu_key_pressed() {
    ui_keybind_t& kb = g_ui_menu.key;
    if (!ui_in_key_reader())
        return ui_in_key_fired(kb.vk);
    uint8 bits = 0;
    if (kb.vk == 0 || !ui_in_key_bits_reader(kb.vk, bits)) {
        kb.bits = 0;
        kb.seen = false;
        return false;
    }
    bool down = (bits & UI_KEY_DOWN) != 0;
    bool was = (kb.bits & UI_KEY_DOWN) != 0;
    bool press = kb.seen && ((down && !was) || ((bits & UI_KEY_TOGGLE) != (kb.bits & UI_KEY_TOGGLE) && !down && !was));
    kb.bits = bits;
    kb.seen = true;
    return press;
}

inline bool ui_menu_poll(bool fg) {
    ui_menu_t& m = g_ui_menu;
    m.fg = fg;
    // Every frame, menu shown or not: release the post-capture key mute once its key is up, so a bind set while holding
    // its key does not leave the feature keys muted after the menu closes.
    ui_keys_mute_tick();
    // A typed edit ends when the host leaves the front, before the key reads it.
    if (!fg && ui_editing()) {
        ui_edit_end();
        m.edit = "";
    }
    bool pressed = ui_menu_key_pressed();
    if (pressed && fg && m.capture.size() == 0 && m.edit.size() == 0)
        ui_menu_set_open(!m.open);
    m.shown = m.open && fg;
    return m.shown;
}

inline void ui_menu_reserve_keys(bool menu_key_row) {
    std::vector<int64> taken = g_ui_feats.reserved;
    if (menu_key_row) {
        for (int64 k : ui_menu_bind_keys())
            taken.push_back(k);
    } else {
        taken.push_back(g_ui_menu.key.vk);
    }
    ui_reserve_keys(taken);
}

inline int32 ui_menu_page_bar(ui_tab_t& t) {
    if (t.pages.size() != 0) {
        ui_tabs(t.pages, t.page);
        return t.page;
    }
    t.page = 0;
    if (g_ui_menu.page_one[0] != t.name)
        g_ui_menu.page_one[0] = t.name;
    ui_tabs(g_ui_menu.page_one, t.page);
    return t.page;
}

inline int32 ui_menu_cols(const ui_tab_t& t, int32 page) {
    if (t.cols.size() == 0)
        return 2;
    uint64 i = static_cast<uint64>(page) < t.cols.size() ? static_cast<uint64>(page) : t.cols.size() - 1;
    return t.cols[i];
}

inline void ui_menu_footer() {
    ui_menu_t& m = g_ui_menu;
    m.foot_texts.clear();
    m.foot_cols.clear();
    for (const ui_indicator_t& d : m.indicators) {
        bool on = d.fn != nullptr && d.fn();
        m.foot_texts.push_back(on ? d.on_text : d.off_text);
        m.foot_cols.push_back(on ? d.on_col : g_ui_theme.muted);
    }
    ui_footer(m.foot_texts, m.foot_cols);
}

inline void ui_menu_draw_tab(int64 now) {
    ui_menu_t& m = g_ui_menu;
    ui_tab_t& t = m.tabs[static_cast<uint64>(m.tab)];
    int32 page = ui_menu_page_bar(t);
    ui_body_begin(static_cast<int64>(m.tab * UI_MENU_PAGE_KEYS + page), ui_menu_cols(t, page));
    ui_menu_reserve_keys(false);
    ui_perf_stage(t.stage);
    if (t.draw_fn != nullptr)
        t.draw_fn(page, now);
    ui_perf_stage("menu_overlay/end");
    ui_body_end();
}

inline void ui_menu_draw_view(int64 now) {
    ui_menu_t& m = g_ui_menu;
    ui_view_t& v = m.views[static_cast<uint64>(m.view)];
    int32 view = m.view;
    if (ui_head_back("Back to " + m.tabs[static_cast<uint64>(m.tab)].name)) {
        ui_menu_open_view(-1);
        ui_menu_draw_tab(now);
        return;
    }
    int32 page = 0;
    if (m.page_one[0] != v.name)
        m.page_one[0] = v.name;
    ui_tabs(m.page_one, page);
    ui_menu_reserve_keys(false);
    ui_perf_stage(v.stage);
    float64 side = v.side_w * g_ui_scale_applied;
    bool groups = v.draw_fn != nullptr && (v.canvas_fn == nullptr || side > 0.0);
    if (v.canvas_fn != nullptr) {
        float64 cx = g_ui_body.x;
        float64 cy = g_ui_body.y;
        float64 cw = g_ui_body.w;
        float64 ch = g_ui_body.h;
        if (groups)
            ui_body_split(side, cx, cy, cw, ch);
        v.canvas_fn(cx, cy, cw, ch, now);
    }
    if (groups) {
        ui_body_begin(static_cast<int64>(-1 - view), 1);
        v.draw_fn(0, now);
        ui_body_end();
    }
    ui_perf_stage("menu_overlay/end");
}

inline void ui_menu_draw(int64 now) {
    ui_menu_t& m = g_ui_menu;
    if (m.tabs.size() == 0)
        return;
    ui_perf_stage("menu_overlay/head");
    // Back from another app or a closed menu: clicks made there are not taps on the menu.
    if (m.drawn_frame + 1 != g_ui_perf.frames)
        ui_menu_mouse_baseline();
    m.drawn_frame = g_ui_perf.frames;
    // Before any keybind row: a capturing key box owns the keys, and a typed edit gets them.
    ui_capture_poll(true);
    bool down = false;
    bool tap = false;
    bool rdown = false;
    bool rtap = false;
    ui_menu_read_button(vk::lbutton, m.lb, down, tap);
    ui_menu_read_button(vk::rbutton, m.rb, rdown, rtap);
    ui_menu_view_fit();
    ui_begin(m.title, down, tap, rdown, rtap, ui_menu_wheel());
    for (const ui_menu_head_t& h : m.heads) {
        bool lit = h.lit_fn != nullptr && h.lit_fn();
        if (ui_head_button(h.icon, h.glyph, h.tip, lit) && h.click_fn != nullptr)
            h.click_fn();
    }
    int32 n = static_cast<int32>(m.tabs.size());
    m.tab = m.tab < 0 ? 0 : (m.tab >= n ? n - 1 : m.tab);
    // No tab shows as selected while a view has the body; a click on one goes back to it.
    for (int32 i = 0; i < n; i++) {
        int32 cur = m.view >= 0 ? -1 : m.tab;
        if (ui_nav(m.tabs[static_cast<uint64>(i)].name, i, cur)) {
            m.tab = i;
            ui_menu_open_view(-1);
        }
    }
    if (m.view >= 0)
        ui_menu_draw_view(now);
    else
        ui_menu_draw_tab(now);
    ui_menu_footer();
    ui_end();
    if (ui_close_clicked())
        ui_menu_set_open(false);
    ui_menu_commit_edits(g_ui_active > 0 && g_ui_down);
    m.edit = ui_edit_label();
    m.capture = ui_capture_label();
}

inline bool ui_menu_hud_rows() {
    ui_menu_t& m = g_ui_menu;
    // A cheap FNV-1a signature of the active keyed set plus the scale; building a string signature cost as much as the
    // work it saved, so this stays integer arithmetic. The rebuild (ui_key_name) and the width re-measure run only when
    // it changes. Returns true when it rebuilt.
    uint64 sig = 14695981039346656037ull ^ static_cast<uint64>(g_ui_scale_applied * 1000.0 + 0.5);
    for (const ui_feat_t& f : g_ui_feats.list) {
        if (f.bind_p == nullptr || !ui_feat_enabled(f))
            continue;
        const ui_keybind_t& kb = *f.bind_p;
        if (kb.mode == UI_KB_ALWAYS_ON || kb.vk == 0)
            continue;
        if (!(kb.mode == UI_KB_TOGGLE && kb.on) && !(kb.mode == UI_KB_HOLD && (kb.bits & UI_KEY_DOWN) != 0))
            continue;
        sig = (sig ^ static_cast<uint64>(f.id)) * 1099511628211ull;
        sig = (sig ^ static_cast<uint64>(kb.vk)) * 1099511628211ull;
    }
    if (m.hud_ready && sig == m.hud_sig)
        return false;
    m.hud_ready = true;
    m.hud_sig = sig;
    m.hud_names.clear();
    m.hud_keys.clear();
    for (const ui_feat_t& f : g_ui_feats.list) {
        if (f.bind_p == nullptr || !ui_feat_enabled(f))
            continue;
        const ui_keybind_t& kb = *f.bind_p;
        if (kb.mode == UI_KB_ALWAYS_ON || kb.vk == 0)
            continue;
        if (!(kb.mode == UI_KB_TOGGLE && kb.on) && !(kb.mode == UI_KB_HOLD && (kb.bits & UI_KEY_DOWN) != 0))
            continue;
        m.hud_names.push_back(f.hud_name.size() != 0 ? f.hud_name : f.name);
        m.hud_keys.push_back(ui_key_name(kb.vk));
    }
    return true;
}

inline void ui_menu_keys_hud(bool interactive) {
    ui_menu_t& m = g_ui_menu;
    ui_ensure_fonts();
    bool rebuilt = ui_menu_hud_rows();
    uint64 n = m.hud_names.size();
    if (n == 0 && !interactive)
        return;
    float64 pad = ui_snap(g_ui_m.pad);
    float64 rh = ui_snap(g_ui_m.row_h * 0.85);
    float64 th = ui_snap(g_ui_m.wm_h);
    // Re-measured only when the active set (hence the scale too) changed, else the cached width is reused.
    if (rebuilt || m.hud_w <= 0.0) {
        float64 mw = ui_gfx_text_size(g_ui_font_bold, UI_MENU_KEYS_TITLE).x;
        for (uint64 i = 0; i < n; i++)
            mw = max(mw, ui_gfx_text_size(g_ui_font, m.hud_names[i]).x + 2.0 * pad
                + ui_gfx_text_size(g_ui_font, m.hud_keys[i]).x);
        m.hud_w = ui_snap(mw + 2.0 * pad);
    }
    float64 w = m.hud_w;
    float64 h = n == 0 ? th : th + rh * static_cast<float64>(n) + ui_snap(pad * 0.5);
    vec2 view = ui_gfx_view();
    ui_float_rect_t& r = m.keys_rect;
    float64 x = r.x;
    float64 y = r.y;
    if (!r.placed) {
        bool wm = m.watermark_on;
        x = (wm ? g_ui_wm.x + g_ui_wm.w : view.x - UI_WM_MARGIN) - w;
        y = wm ? g_ui_wm.y + g_ui_wm.h + UI_WM_MARGIN * 0.5 : UI_WM_MARGIN;
        if (wm && y + h > view.y)
            y = g_ui_wm.y - h - UI_WM_MARGIN * 0.5;
    }
    x = ui_snap(ui_clamp(x, 0.0, max(view.x - w, 0.0)));
    y = ui_snap(ui_clamp(y, 0.0, max(view.y - h, 0.0)));
    if (ui_float_drag_at(UI_MENU_KEYS_SLOT, x, y, w, h, interactive))
        r.placed = true;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    ui_hud_panel(x, y, w, h);
    ui_text_at(UI_MENU_KEYS_TITLE, x + pad, y, th, g_ui_theme.text, g_ui_font_bold);
    if (n == 0)
        return;
    ui_gfx_fill(vec2(x + pad, y + th - 1.0), vec2(w - 2.0 * pad, 1.0), g_ui_theme.panel_edge, 0.0);
    for (uint64 i = 0; i < n; i++) {
        float64 ry = y + th + rh * static_cast<float64>(i);
        ui_text_at(m.hud_names[i], x + pad, ry, rh, g_ui_theme.text, g_ui_font);
        float64 kw = ui_gfx_text_size(g_ui_font, m.hud_keys[i]).x;
        ui_text_at(m.hud_keys[i], ui_snap(x + w - pad - kw), ry, rh, ui_col_accent(), g_ui_font);
    }
}

inline void ui_menu_hud() {
    ui_menu_t& m = g_ui_menu;
    if (!m.fg)
        return;
    if (m.watermark_on && m.wm_fn != nullptr) {
        m.wm_fn(m.wm_parts);
        ui_watermark_card(m.wm_parts, m.shown);
    }
    if (m.keys_hud_on)
        ui_menu_keys_hud(m.shown);
}

inline void ui_menu_tick(int64 now) {
    ui_sync_accent();
    // ui_end pumps the pictures only while the menu draws: a closed menu's map tiles are freed on time here.
    ui_pic_tick();
    for (const ui_tab_t& t : g_ui_menu.tabs) {
        if (t.tick_fn != nullptr)
            t.tick_fn();
    }
    ui_cfg_tick(now);
}

inline void ui_menu_scale_row(const std::string& label) {
    int32 scale = ui_scale_index(g_ui_scale);
    if (ui_combo(label, scale, UI_SCALE_NAMES))
        g_ui_scale = ui_scale_preset(UI_SCALE_PRESETS[static_cast<uint64>(scale)]);
}

inline void ui_menu_key_row(const std::string& label, const std::string& extra) {
    ui_menu_reserve_keys(true);
    ui_label_key_lit(label, g_ui_menu.key, false, true);
    ui_tip(ui_key_name(g_ui_menu.key.vk) + " shows and hides this menu. It cannot be cleared. The cursor is freed "
        "while the menu is open." + extra);
    if (ui_capturing())
        ui_status_colored("Press a key or mouse button. Esc or a click on the box cancels.", g_ui_theme.purple);
}

inline void ui_menu_hud_toggles() {
    ui_toggle("Watermark", g_ui_menu.watermark_on);
    ui_toggle("Keybind list", g_ui_menu.keys_hud_on);
    ui_tip("Lists the active features and their keys: Toggle keys switched on, On hold keys while held. Always on "
        "binds are not listed. Hidden while none is. Drag it with the menu open.");
}

inline void ui_menu_accent_rows() {
    color accent = g_ui_theme.accent;
    if (ui_color_picker("Accent", accent))
        g_ui_theme.accent = accent;
    if (!(g_ui_theme.accent == g_ui_theme.teal) && ui_button("Reset accent"))
        g_ui_theme.accent = g_ui_theme.teal;
}

inline void ui_menu_fix_key() {
    ui_menu_t& m = g_ui_menu;
    if (!ui_menu_key_clashes(m.key.vk))
        return;
    for (int64 k : m.key_fallbacks) {
        if (!ui_menu_key_clashes(k)) {
            m.key = ui_kb_make(k, UI_KB_TOGGLE);
            return;
        }
    }
}

inline void ui_menu_shutdown() {
    g_ui_menu.open = false;
    g_ui_menu.shown = false;
    ui_edit_end();
    ui_capture_cancel();
    for (const ui_tab_t& t : g_ui_menu.tabs) {
        if (t.shutdown_fn != nullptr)
            t.shutdown_fn();
    }
    ui_pic_shutdown();
}
