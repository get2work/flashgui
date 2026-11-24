inline void ui_popup_close() {
    g_ui_pop.label = "";
}

inline int32 ui_popup_item() {
    float64 top = static_cast<float64>(g_ui_pop.top);
    float64 last = min(top + static_cast<float64>(g_ui_pop.shown), static_cast<float64>(g_ui_pop.items.size())) - 1.0;
    float64 row = static_cast<float64>(static_cast<int64>((g_ui_mouse.y - g_ui_pop.y - 1.0) / g_ui_pop.ih));
    return static_cast<int32>(ui_clamp(top + row, top, last));
}

inline int32 ui_popup_max_top() {
    int32 n = static_cast<int32>(g_ui_pop.items.size());
    return n > g_ui_pop.shown ? n - g_ui_pop.shown : 0;
}

inline float64 ui_popup_thumb_h() {
    float64 track = g_ui_pop.h - 2.0;
    float64 n = static_cast<float64>(g_ui_pop.items.size());
    return n > 0.0 ? max(track * static_cast<float64>(g_ui_pop.shown) / n, g_ui_pop.ih) : track;
}

inline float64 ui_popup_thumb_y() {
    int32 max_top = ui_popup_max_top();
    float64 t = max_top > 0 ? static_cast<float64>(g_ui_pop.top) / static_cast<float64>(max_top) : 0.0;
    return g_ui_pop.y + 1.0 + (g_ui_pop.h - 2.0 - ui_popup_thumb_h()) * t;
}

inline void ui_popup_input() {
    uint8 bits = 0;
    bool esc = (ui_in_key_bits(vk::escape, bits) && (bits & UI_KEY_DOWN) != 0) || ui_in_key_down(vk::escape);
    bool esc_press = esc && !g_ui_pop.esc_down;
    g_ui_pop.esc_down = esc;
    if (esc_press && !ui_editing() && !ui_capturing() && g_ui_cap.mute_vk != vk::escape)
        g_ui_pop.label = "";
    g_ui_pop.prev = g_ui_pop.label;
    g_ui_pop.pick = -1;
    g_ui_pop.live = g_ui_pop.label.size() != 0;
    if (!g_ui_pop.live)
        return;
    bool over = ui_mouse_in(g_ui_pop.x, g_ui_pop.y, g_ui_pop.w, g_ui_pop.h);
    int32 max_top = ui_popup_max_top();
    // The wheel over it is used up here, scrolling or not, so the body behind never sees it.
    if (over && g_ui_scroll != 0.0 && max_top > 0) {
        int32 step = static_cast<int32>(UI_SCROLL_ROWS);
        g_ui_pop.top = g_ui_pop.top + (g_ui_scroll > 0.0 ? -step : step);
        g_ui_pop.top = g_ui_pop.top < 0 ? 0 : (g_ui_pop.top > max_top ? max_top : g_ui_pop.top);
    }
    if (over)
        g_ui_scroll = 0.0;
    bool on_bar = over && g_ui_pop.bar_w > 0.0 && g_ui_mouse.x >= g_ui_pop.x + g_ui_pop.w - 1.0 - g_ui_pop.bar_w;
    if (ui_take_press(UI_ID_POPUP, over)) {
        g_ui_pop.press = on_bar ? UI_POP_BAR : ui_popup_item();
        if (on_bar) {
            float64 ty = ui_popup_thumb_y();
            float64 th = ui_popup_thumb_h();
            // A press off the thumb centres it there, then drags as usual.
            g_ui_drag_dy = g_ui_mouse.y >= ty && g_ui_mouse.y < ty + th ? g_ui_mouse.y - ty : th * 0.5;
        }
    } else if (g_ui_pressed) {
        g_ui_pressed = false;
        g_ui_active = UI_ID_NOWHERE;
        g_ui_pop.label = "";
    }
    if (g_ui_active == UI_ID_POPUP && g_ui_pop.press == UI_POP_BAR && g_ui_down && max_top > 0) {
        float64 span = max(g_ui_pop.h - 2.0 - ui_popup_thumb_h(), 1.0);
        float64 t = ui_clamp((g_ui_mouse.y - g_ui_drag_dy - g_ui_pop.y - 1.0) / span, 0.0, 1.0);
        g_ui_pop.top = static_cast<int32>(t * static_cast<float64>(max_top) + 0.5);
    }
    if (g_ui_active == UI_ID_POPUP && g_ui_released && over && g_ui_pop.press >= 0 && ui_popup_item() == g_ui_pop.press)
        g_ui_pop.pick = g_ui_pop.press;
    // A right press elsewhere closes it and stays for the keybind under it, which reopens on its own box.
    if (g_ui_rpressed && over)
        g_ui_rpressed = false;
    else if (g_ui_rpressed)
        g_ui_pop.label = "";
}

inline void ui_popup_set_items(const std::vector<std::string>& items, const std::vector<std::string>& icons, bool multi, const std::vector<bool>& marks, int32 mode) {
    g_ui_pop.seen = true;
    if (g_ui_pop.items.size() != items.size()) {
        g_ui_pop.items = items;
    } else {
        for (uint64 i = 0; i < items.size(); i++) {
            if (g_ui_pop.items[i] != items[i])
                g_ui_pop.items[i] = items[i];
        }
    }
    if (g_ui_pop.icons.size() != icons.size()) {
        g_ui_pop.icons = icons;
    } else {
        for (uint64 i = 0; i < icons.size(); i++) {
            if (g_ui_pop.icons[i] != icons[i])
                g_ui_pop.icons[i] = icons[i];
        }
    }
    g_ui_pop.multi = multi;
    if (g_ui_pop.marks.size() != marks.size())
        g_ui_pop.marks.resize(marks.size());
    for (uint64 i = 0; i < marks.size(); i++)
        g_ui_pop.marks[i] = marks[i];
    g_ui_pop.mode = mode;
}

inline void ui_popup_fix_top(const std::vector<bool>& marks, bool multi, int32 mode) {
    int32 n = static_cast<int32>(g_ui_pop.items.size());
    int32 shown = g_ui_pop.shown;
    int32 max_top = n > shown ? n - shown : 0;
    if (g_ui_pop.placed != g_ui_pop.label) {
        g_ui_pop.placed = g_ui_pop.label;
        g_ui_pop.top = g_ui_pop_tops.count(g_ui_pop.label) != 0 ? g_ui_pop_tops[g_ui_pop.label] : 0;
        int32 sel = mode;
        for (int32 i = 0; sel < 0 && multi && i < n && static_cast<uint64>(i) < marks.size(); i++) {
            if (marks[static_cast<uint64>(i)])
                sel = i;
        }
        // A multi popup with a marked item already in view stays put.
        for (int32 i = g_ui_pop.top; mode < 0 && multi && i < g_ui_pop.top + shown && i < n; i++) {
            if (i >= 0 && static_cast<uint64>(i) < marks.size() && marks[static_cast<uint64>(i)])
                sel = -1;
        }
        if (sel >= 0 && sel < g_ui_pop.top)
            g_ui_pop.top = sel;
        else if (sel >= 0 && sel >= g_ui_pop.top + shown)
            g_ui_pop.top = sel - shown + 1;
    }
    g_ui_pop.top = g_ui_pop.top < 0 ? 0 : (g_ui_pop.top > max_top ? max_top : g_ui_pop.top);
    g_ui_pop_tops[g_ui_pop.label] = g_ui_pop.top;
}

inline void ui_popup_place(const std::vector<std::string>& items, const std::vector<std::string>& icons, bool multi, const std::vector<bool>& marks, int32 mode, float64 w, float64 right_x, float64 top, float64 bottom) {
    float64 ih = ui_snap(g_ui_m.item_h);
    float64 below_y = ui_snap(bottom + g_ui_style.popup_gap);
    float64 above_y = ui_snap(top - g_ui_style.popup_gap);
    float64 vis_top = g_ui_in_group ? g_ui_clip_y : g_ui_body.y;
    float64 vis_bottom = g_ui_in_group ? g_ui_clip_y + g_ui_clip_h : g_ui_body.y + g_ui_body.h;
    float64 mid = (top + bottom) * 0.5;
    if (g_ui_body.live && (mid < vis_top || mid > vis_bottom)) {
        g_ui_pop.label = "";
        return;
    }
    ui_popup_set_items(items, icons, multi, marks, mode);
    g_ui_pop.at_point = false;
    g_ui_pop.ih = ih;
    int32 n = static_cast<int32>(items.size());
    int32 want = n < UI_POPUP_ROWS ? n : UI_POPUP_ROWS;
    vec2 view = ui_gfx_view();
    float64 below = view.y - below_y - 2.0;
    float64 above = above_y - 2.0;
    bool up = below < ih * static_cast<float64>(want) && above > below;
    int32 fit = static_cast<int32>(max(up ? above : below, ih) / ih);
    int32 shown = want < fit ? want : fit;
    g_ui_pop.shown = shown;
    g_ui_pop.bar_w = n > shown ? ui_snap(g_ui_m.pad) : 0.0;
    g_ui_pop.w = w + g_ui_pop.bar_w;
    g_ui_pop.h = ih * static_cast<float64>(shown) + 2.0;
    g_ui_pop.x = max(right_x - g_ui_pop.w, 0.0);
    g_ui_pop.y = up ? max(above_y - g_ui_pop.h, 0.0) : below_y;
    ui_popup_fix_top(marks, multi, mode);
}

inline void ui_popup_menu_open(const std::string& label, float64 x, float64 y) {
    g_ui_pop.label = label;
    g_ui_pop.at_point = true;
    g_ui_pop.ax = x;
    g_ui_pop.ay = y;
    g_ui_pop.press = -1;
    g_ui_pop.pick = -1;
}

inline bool ui_popup_menu_is_open(const std::string& label) {
    return g_ui_pop.at_point && g_ui_pop.label.size() != 0 && g_ui_pop.label == label;
}

inline void ui_popup_menu_move(const std::string& label, float64 x, float64 y) {
    if (!ui_popup_menu_is_open(label))
        return;
    g_ui_pop.ax = x;
    g_ui_pop.ay = y;
}

inline int32 ui_popup_menu_rows(int32 n, float64 ih) {
    int32 want = n < UI_POPUP_ROWS ? n : UI_POPUP_ROWS;
    vec2 view = ui_gfx_view();
    int32 fit = view.y > 0.0 ? static_cast<int32>(max(view.y - 2.0, ih) / ih) : want;
    return want < fit ? want : fit;
}

inline vec2 ui_popup_menu_size(const std::vector<std::string>& options) {
    float64 ih = ui_snap(g_ui_m.item_h);
    float64 pad = ui_snap(g_ui_m.pad);
    int32 n = static_cast<int32>(options.size());
    int32 shown = ui_popup_menu_rows(n, ih);
    float64 bar = n > shown ? pad : 0.0;
    return vec2(ui_snap(max(ui_max_text_w(options), g_ui_m.stub_w) + 2.0 * pad + 2.0) + bar,
        ih * static_cast<float64>(shown) + 2.0);
}

inline int32 ui_popup_menu(const std::string& label, const std::vector<std::string>& options) {
    return ui_popup_menu(label, options, -1);
}

inline int32 ui_popup_menu(const std::string& label, const std::vector<std::string>& options, int32 current) {
    if (!ui_popup_menu_is_open(label))
        return -1;
    if (g_ui_pop.pick >= 0 && static_cast<uint64>(g_ui_pop.pick) < options.size()) {
        int32 pick = g_ui_pop.pick;
        g_ui_pop.pick = -1;
        g_ui_pop.label = "";
        return pick;
    }
    if (options.size() == 0) {
        g_ui_pop.label = "";
        return -1;
    }
    ui_popup_set_items(options, g_ui_no_icons, false, g_ui_no_marks, current);
    float64 ih = ui_snap(g_ui_m.item_h);
    float64 pad = ui_snap(g_ui_m.pad);
    g_ui_pop.ih = ih;
    int32 n = static_cast<int32>(options.size());
    int32 shown = ui_popup_menu_rows(n, ih);
    vec2 size = ui_popup_menu_size(options);
    vec2 view = ui_gfx_view();
    g_ui_pop.shown = shown;
    g_ui_pop.bar_w = n > shown ? pad : 0.0;
    g_ui_pop.w = size.x;
    g_ui_pop.h = size.y;
    float64 x = ui_snap(g_ui_pop.ax);
    float64 y = ui_snap(g_ui_pop.ay);
    if (view.x > 0.0 && x + g_ui_pop.w > view.x)
        x = x - g_ui_pop.w;
    if (view.y > 0.0 && y + g_ui_pop.h > view.y)
        y = y - g_ui_pop.h;
    if (view.x > 0.0)
        x = min(x, view.x - g_ui_pop.w);
    if (view.y > 0.0)
        y = min(y, view.y - g_ui_pop.h);
    g_ui_pop.x = max(x, 0.0);
    g_ui_pop.y = max(y, 0.0);
    g_ui_pop.at_point = true;
    ui_popup_fix_top(g_ui_no_marks, false, current);
    return -1;
}

inline void ui_popup_draw() {
    if (!g_ui_pop.seen)
        g_ui_pop.label = "";
    g_ui_pop.seen = false;
    if (g_ui_pop.label.size() == 0) {
        g_ui_pop.placed = "";
        return;
    }
    float64 x = g_ui_pop.x;
    float64 y = g_ui_pop.y;
    float64 w = g_ui_pop.w;
    float64 pad = ui_snap(g_ui_m.pad);
    ui_draw_box(x, y, w, g_ui_pop.h, g_ui_theme.bg2, g_ui_theme.border_hi);
    float64 bs = ui_snap(g_ui_m.box);
    float64 rw = w - 2.0 - g_ui_pop.bar_w;
    bool dragging = g_ui_active == UI_ID_POPUP && g_ui_pop.press == UI_POP_BAR;
    ui_gfx_clip_push(vec2(x + 1.0, y + 1.0), vec2(w - 2.0, max(g_ui_pop.h - 2.0, 0.0)));
    for (int32 s = 0; s < g_ui_pop.shown; s++) {
        int32 item = g_ui_pop.top + s;
        if (item < 0 || static_cast<uint64>(item) >= g_ui_pop.items.size())
            continue;
        uint64 i = static_cast<uint64>(item);
        float64 iy = y + 1.0 + static_cast<float64>(s) * g_ui_pop.ih;
        bool hot = ui_mouse_in(x + 1.0, iy, rw, g_ui_pop.ih) && !dragging;
        bool held = g_ui_active == UI_ID_POPUP && g_ui_pop.press == item;
        if (hot || held)
            ui_gfx_fill(vec2(x + 1.0, iy), vec2(rw, g_ui_pop.ih), g_ui_theme.hover, 0.0);
        float64 tx = x + 1.0 + pad;
        bool marked = g_ui_pop.multi && i < g_ui_pop.marks.size() && g_ui_pop.marks[i];
        if (g_ui_pop.multi) {
            ui_draw_check(ui_snap(tx), ui_snap(iy + (g_ui_pop.ih - bs) * 0.5), bs, marked, hot);
            tx = tx + bs + pad;
        }
        if (g_ui_pop.icons.size() != 0) {
            float64 is = ui_item_icon(g_ui_pop.ih);
            if (i < g_ui_pop.icons.size())
                ui_icon_any(g_ui_pop.icons[i], ui_snap(tx), ui_snap(iy + (g_ui_pop.ih - is) * 0.5), is);
            tx = tx + is + ui_snap(pad * 0.5);
        }
        bool current = item == g_ui_pop.mode || (g_ui_pop.at_point && (hot || held));
        if (current) {
            float64 in = g_ui_style.mark_inset;
            vec2 bar = vec2(ui_snap(g_ui_m.accent_w), g_ui_pop.ih - 2.0 * in);
            ui_gfx_fill(vec2(x + 1.0, iy + in), bar, ui_col_accent(), 0.0);
        }
        // A popup menu's option in use (ui_popup_menu's current) reads in the accent, so it stands apart from the hovered.
        bool in_use = g_ui_pop.at_point && item == g_ui_pop.mode;
        color c = in_use ? ui_col_accent() : (current || marked || hot ? g_ui_theme.text : g_ui_theme.dim);
        ui_text_at(g_ui_pop.items[i], tx, iy, g_ui_pop.ih, c, g_ui_font);
        if (hot && g_ui_pop.tips_label == g_ui_pop.label && i < g_ui_pop.tips.size() && g_ui_pop.tips[i].size() != 0)
            g_ui_tip.text = g_ui_pop.tips[i];
    }
    if (g_ui_pop.bar_w > 0.0) {
        float64 gx = x + w - 1.0 - g_ui_pop.bar_w;
        float64 bw = max(ui_snap(g_ui_m.scroll_w), g_ui_style.bar_min);
        float64 bx = ui_snap(gx + (g_ui_pop.bar_w - bw) * 0.5);
        ui_gfx_fill(vec2(bx, y + 1.0), vec2(bw, g_ui_pop.h - 2.0), g_ui_theme.border, bw * 0.5);
        bool bar_hot = ui_mouse_in(gx, y, g_ui_pop.bar_w + 1.0, g_ui_pop.h);
        color idle = ui_fade(ui_col_accent(), g_ui_theme.thumb_idle_a);
        ui_scroll_thumb(gx, g_ui_pop.bar_w, ui_popup_thumb_y() + 1.0, ui_popup_thumb_h() - 2.0,
            dragging || (bar_hot && g_ui_active == 0), idle, ui_col_accent());
    }
    ui_gfx_clip_pop();
}

inline void ui_draw_combo_arrow(float64 fx, float64 fy, float64 fw, float64 fh, float64 cw, bool open) {
    ui_draw_chevron(fx + fw - cw * 0.5 - g_ui_style.arrow_nudge, fy + fh * 0.5, ui_snap(g_ui_m.frame_h) * g_ui_style.arrow,
        open, g_ui_theme.dim);
}

inline float64 ui_combo_line_h() {
    return ui_snap(g_ui_m.combo_h + g_ui_m.row_h - g_ui_m.frame_h);
}

inline float64 ui_combo_need(const std::string& label, const std::vector<std::string>& names, float64 cw) {
    float64 pad = g_ui_m.pad;
    float64 lw = ui_icon_w() + ui_gfx_text_size(g_ui_font, label).x;
    return max(lw, min(ui_max_text_w(names), g_ui_m.stub_w) + 2.0 * pad + cw);
}

inline bool ui_combo(const std::string& label, int32& index, const std::vector<std::string>& names) {
    return ui_combo_icons(label, index, names, g_ui_no_icons);
}

inline bool ui_combo_icons(const std::string& label, int32& index, const std::vector<std::string>& names, const std::vector<std::string>& icons) {
    if (names.size() == 0)
        return false;
    if (index < 0 || static_cast<uint64>(index) >= names.size())
        index = 0;
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 lh = ui_combo_line_h();
    float64 y = ui_row_begin(ui_snap(g_ui_m.row_h * g_ui_style.text_line) + lh);
    float64 fw = w;
    float64 fx = x;
    float64 fh = ui_snap(g_ui_m.combo_h);
    float64 cw = ui_snap(ui_snap(g_ui_m.frame_h) * g_ui_style.arrow_w);
    ui_need(ui_combo_need(label, names, cw));
    float64 fy = ui_snap(ui_label_line(label, x, y, w) + (lh - fh) * 0.5);
    bool changed = false;
    if (g_ui_pop.label == label && g_ui_pop.pick >= 0 && static_cast<uint64>(g_ui_pop.pick) < names.size()) {
        changed = g_ui_pop.pick != index;
        index = g_ui_pop.pick;
        g_ui_pop.label = "";
    }
    int32 id = ui_id();
    bool hot = ui_hovered(fx, fy, fw, fh);
    // ui_popup_input already closed it on the press if it was open: a second click closes.
    if (ui_button_behavior(id, hot))
        g_ui_pop.label = g_ui_pop.prev == label ? "" : label;
    bool open = g_ui_pop.label == label;
    float64 pad = ui_snap(g_ui_m.pad);
    ui_draw_field(fx, fy, fw, fh, ui_lit(hot, id), open);
    float64 is = ui_item_icon(fh);
    float64 iw = icons.size() != 0 ? is + ui_snap(pad * 0.5) : 0.0;
    float64 tx = fx + pad;
    if (static_cast<uint64>(index) < icons.size() && ui_icon_any(icons[static_cast<uint64>(index)], tx,
        ui_snap(fy + (fh - is) * 0.5), is))
        tx = tx + iw;
    std::string name = ui_fit_text(names[static_cast<uint64>(index)], fx + fw - cw - tx);
    ui_text_at(name, tx, fy, fh, g_ui_theme.text, g_ui_font);
    ui_draw_combo_arrow(fx, fy, fw, fh, cw, open);
    if (open) {
        float64 piw = icons.size() != 0 ? ui_item_icon(ui_snap(g_ui_m.item_h)) + ui_snap(pad * 0.5) : 0.0;
        ui_popup_place(names, icons, false, g_ui_no_marks, index,
            max(fw, ui_snap(piw + ui_max_text_w(names) + 2.0 * pad + 2.0)), fx + fw, fy, fy + fh);
    }
    return changed;
}

inline bool ui_multi_combo(const std::string& label, const std::vector<std::string>& names, std::vector<bool>& picked) {
    return ui_multi_combo_icons(label, names, g_ui_no_icons, picked);
}

inline bool ui_multi_icons_fit(const std::vector<std::string>& icons, const std::vector<bool>& picked, float64 is, float64 avail) {
    if (icons.size() == 0)
        return false;
    float64 n = 0.0;
    for (uint64 i = 0; i < picked.size(); i++) {
        if (!picked[i])
            continue;
        if (i >= icons.size() || !ui_gfx_icon_ready(icons[i]))
            return false;
        n = n + 1.0;
    }
    return n > 0.0 && n * (is + ui_snap(g_ui_m.pad * 0.5)) <= avail;
}

inline bool ui_multi_combo_icons(const std::string& label, const std::vector<std::string>& names, const std::vector<std::string>& icons, std::vector<bool>& picked) {
    picked.resize(names.size());
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 lh = ui_combo_line_h();
    float64 y = ui_row_begin(ui_snap(g_ui_m.row_h * g_ui_style.text_line) + lh);
    float64 fw = w;
    float64 fx = x;
    float64 fh = ui_snap(g_ui_m.combo_h);
    float64 cw = ui_snap(ui_snap(g_ui_m.frame_h) * g_ui_style.arrow_w);
    ui_need(ui_combo_need(label, names, cw));
    float64 fy = ui_snap(ui_label_line(label, x, y, w) + (lh - fh) * 0.5);
    bool changed = false;
    if (g_ui_pop.label == label && g_ui_pop.pick >= 0 && static_cast<uint64>(g_ui_pop.pick) < picked.size()) {
        picked[g_ui_pop.pick] = !picked[g_ui_pop.pick];
        changed = true;
    }
    int32 id = ui_id();
    bool hot = ui_hovered(fx, fy, fw, fh);
    // ui_popup_input already closed it on the press if it was open: a second click closes.
    if (ui_button_behavior(id, hot))
        g_ui_pop.label = g_ui_pop.prev == label ? "" : label;
    bool open = g_ui_pop.label == label;
    float64 pad = ui_snap(g_ui_m.pad);
    ui_draw_field(fx, fy, fw, fh, ui_lit(hot, id), open);
    float64 is = ui_item_icon(fh);
    float64 igap = ui_snap(pad * 0.5);
    bool as_icons = ui_multi_icons_fit(icons, picked, is, fw - pad - cw);
    float64 ix = fx + pad;
    for (uint64 i = 0; as_icons && i < picked.size(); i++) {
        if (!picked[i])
            continue;
        ui_icon_any(icons[i], ix, ui_snap(fy + (fh - is) * 0.5), is);
        ix = ix + is + igap;
    }
    std::string joined = "";
    for (uint64 i = 0; i < names.size(); i++) {
        if (!picked[i])
            continue;
        if (joined.size() != 0)
            joined = joined + ", ";
        joined = joined + names[i];
    }
    if (joined.size() == 0)
        ui_text_at("None", fx + pad, fy, fh, g_ui_theme.muted, g_ui_font);
    else if (!as_icons)
        ui_text_at(ui_fit_text_with(g_ui_font, joined, fw - pad - cw, "..."), fx + pad, fy, fh, g_ui_theme.text,
            g_ui_font);
    ui_draw_combo_arrow(fx, fy, fw, fh, cw, open);
    if (open) {
        float64 iw = icons.size() != 0 ? ui_item_icon(ui_snap(g_ui_m.item_h)) + igap : 0.0;
        float64 pw = max(fw, ui_snap(g_ui_m.box + iw + ui_max_text_w(names) + 3.0 * pad + 2.0));
        ui_popup_place(names, icons, true, picked, -1, pw, fx + fw, fy, fy + fh);
    }
    return changed;
}

inline void ui_multi_tips(const std::string& label, const std::vector<std::string>& tips) {
    if (g_ui_pop.label != label)
        return;
    g_ui_pop.tips_label = label;
    if (g_ui_pop.tips.size() != tips.size()) {
        g_ui_pop.tips = tips;
        return;
    }
    for (uint64 i = 0; i < tips.size(); i++) {
        if (g_ui_pop.tips[i] != tips[i])
            g_ui_pop.tips[i] = tips[i];
    }
}

inline bool ui_multi_list(const std::string& label, const std::vector<std::string>& names, const std::vector<std::string>& icons, const std::vector<std::string>& notes, std::vector<bool>& picked, int32 rows) {
    picked.resize(names.size());
    int32 n = static_cast<int32>(names.size());
    int32 shown = n < rows ? n : rows;
    if (shown < 1)
        shown = 1;
    int32 max_top = n > shown ? n - shown : 0;
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 ih = ui_snap(g_ui_m.item_h);
    float64 pad = ui_snap(g_ui_m.pad);
    float64 h = ih * static_cast<float64>(shown) + 2.0;
    float64 y = ui_snap(ui_row_begin(h + ui_snap(pad * 0.5)));
    float64 bs = ui_snap(g_ui_m.box);
    float64 is = ui_snap(ih - 2.0 * g_ui_style.list_icon_inset);
    ui_need(g_ui_m.stub_w + bs + is + 3.0 * pad);
    int32 top = g_ui_list_top.count(label) != 0 ? g_ui_list_top[label] : 0;
    if (ui_hovered(x, y, w, h) && g_ui_scroll != 0.0 && g_ui_active == 0) {
        top = top + (g_ui_scroll > 0.0 ? -1 : 1) * static_cast<int32>(UI_SCROLL_ROWS);
        g_ui_scroll = 0.0;
    }
    float64 bar_x = x + w - pad;
    float64 th = max(h * static_cast<float64>(shown) / static_cast<float64>(n > 0 ? n : 1), ih);
    int32 bar_id = ui_id();
    bool bar_hot = max_top > 0 && ui_hovered(bar_x, y, pad, h);
    if (ui_take_press(bar_id, bar_hot))
        g_ui_drag_dy = th * 0.5;
    bool dragging = g_ui_active == bar_id && g_ui_down && max_top > 0;
    if (dragging) {
        float64 t = ui_clamp((g_ui_mouse.y - g_ui_drag_dy - y) / max(h - th, 1.0), 0.0, 1.0);
        top = static_cast<int32>(t * static_cast<float64>(max_top) + 0.5);
    }
    top = top < 0 ? 0 : (top > max_top ? max_top : top);
    g_ui_list_top[label] = top;

    ui_draw_box(x, y, w, h, g_ui_theme.bg2, g_ui_theme.border);
    // Clips do not intersect on their own: cut the box to the visible group (or body) by hand.
    float64 vx = g_ui_in_group ? g_ui_clip_x : g_ui_body.x;
    float64 vy = g_ui_in_group ? g_ui_clip_y : g_ui_body.y;
    float64 vr = g_ui_in_group ? g_ui_clip_x + g_ui_clip_w : g_ui_body.x + g_ui_body.w;
    float64 vb = g_ui_in_group ? g_ui_clip_y + g_ui_clip_h : g_ui_body.y + g_ui_body.h;
    float64 cx = max(x + 1.0, vx);
    float64 cy = max(y + 1.0, vy);
    ui_gfx_clip_push(vec2(cx, cy), vec2(max(min(x + w - 1.0, vr) - cx, 0.0), max(min(y + h - 1.0, vb) - cy, 0.0)));
    bool changed = false;
    float64 rw = (max_top > 0 ? bar_x : x + w - 1.0) - x - 1.0;
    if (n == 0)
        ui_text_at("Nothing to list", x + pad, y + 1.0, ih, g_ui_theme.muted, g_ui_font);
    for (int32 s = 0; s < shown; s++) {
        int32 id = ui_id();
        int32 i = top + s;
        if (i >= n)
            continue;
        uint64 k = static_cast<uint64>(i);
        float64 ry = y + 1.0 + static_cast<float64>(s) * ih;
        bool hot = ui_hovered(x + 1.0, ry, rw, ih) && !dragging;
        if (ui_button_behavior(id, hot)) {
            picked[k] = !picked[k];
            changed = true;
        }
        bool lit = ui_lit(hot, id);
        if (lit)
            ui_gfx_fill(vec2(x + 1.0, ry), vec2(rw, ih), g_ui_theme.hover, 0.0);
        float64 tx = x + 1.0 + pad;
        ui_draw_check(ui_snap(tx), ui_snap(ry + (ih - bs) * 0.5), bs, picked[k], lit);
        tx = tx + bs + pad;
        if (k < icons.size())
            ui_icon_any(icons[k], ui_snap(tx), ui_snap(ry + (ih - is) * 0.5), is);
        tx = tx + is + ui_snap(pad * 0.5);
        float64 nw = 0.0;
        if (k < notes.size() && notes[k].size() != 0) {
            nw = ui_snap(ui_gfx_text_size(g_ui_font_small, notes[k]).x);
            ui_text_at(notes[k], x + 1.0 + rw - pad - nw, ry, ih, g_ui_theme.muted, g_ui_font_small);
            nw = nw + pad;
        }
        color c = picked[k] || lit ? g_ui_theme.text : g_ui_theme.dim;
        ui_text_at(ui_fit_text(names[k], max(x + 1.0 + rw - pad - nw - tx, 0.0)), tx, ry, ih, c, g_ui_font);
    }
    ui_gfx_clip_pop();
    if (max_top > 0) {
        float64 ty = y + (h - th) * static_cast<float64>(top) / static_cast<float64>(max_top);
        bool bar_lit = dragging || (bar_hot && g_ui_active == 0);
        ui_scroll_thumb(bar_x, pad, ty + 1.0, th - 2.0, bar_lit, g_ui_theme.border_hi, g_ui_theme.dim);
    }
    ui_row_mark(x, y, h, x, x);
    return changed;
}
