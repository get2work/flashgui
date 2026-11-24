inline void ui_logo_load() {
    if (g_ui_logo.tried)
        return;
    if (g_ui_cfg.logo_path.size() == 0) {
        g_ui_logo.tried = true;
        return;
    }
    float64 aspect = 1.0;
    int64 tex = ui_gfx_texture(g_ui_cfg.logo_path, aspect);
    if (!ui_gfx_texture_settled(g_ui_cfg.logo_path))
        return;
    g_ui_logo.tried = true;
    if (tex == 0)
        ui_sys_log("ui: logo not loaded from " + g_ui_cfg.logo_path);
}

inline void ui_draw_logo(float64 cx, float64 top, float64 size) {
    float64 aspect = 1.0;
    int64 tex = g_ui_cfg.logo_path.size() != 0 ? ui_gfx_texture(g_ui_cfg.logo_path, aspect) : 0;
    if (tex == 0) {
        vec2 c = vec2(cx, top + size * 0.5);
        ui_gfx_ring(c, size * 0.5 - 1.0, g_ui_theme.text, ui_stroke_w());
        ui_gfx_disc(c, size * g_ui_style.logo_dot, g_ui_theme.text);
        return;
    }
    float64 w = size;
    float64 h = size * aspect;
    if (h > size) {
        h = size;
        w = size / aspect;
    }
    vec2 at = vec2(ui_snap(cx - w * 0.5), ui_snap(top + (size - h) * 0.5));
    ui_gfx_image(tex, at, vec2(ui_snap(w), ui_snap(h)), UI_WHITE);
}

inline float64 ui_rt_outline(float64 h) {
    return max(ui_snap(h * g_ui_style.rt_outline), 1.0);
}

inline float64 ui_rt_t_dx(float64 p, float64 o) {
    float64 need = std::ceil(p * g_ui_style.rt_touch + g_ui_style.rt_eat * o) + o + 1.0;
    return max(ui_snap(p * g_ui_style.rt_tuck), need);
}

inline float64 ui_rt_logo_w(float64 h) {
    float64 p = static_cast<float64>(static_cast<int64>(h / g_ui_style.rt_ink_h + 0.5));
    return ui_rt_t_dx(p, ui_rt_outline(h)) + ui_snap(p * (g_ui_style.rt_ink_w - g_ui_style.rt_tuck));
}

inline void ui_draw_rt_logo(float64 x, float64 y, float64 h) {
    int64 px = static_cast<int64>(h / g_ui_style.rt_ink_h + 0.5);
    if (px < 1)
        return;
    int64 font = ui_rt_font(px);
    float64 p = static_cast<float64>(px);
    float64 ox = ui_snap(x - p * g_ui_style.rt_ink_x);
    float64 oy = ui_snap(y - p * g_ui_style.rt_top);
    float64 o = ui_rt_outline(h);
    color edge = ui_col_accent().with_alpha(255);
    ui_gfx_text_outline("R", vec2(ox, oy), UI_BLACK, font, edge, o);
    ui_gfx_text_outline("T", vec2(ox + ui_rt_t_dx(p, o), oy + ui_snap(p * g_ui_style.rt_drop)), UI_BLACK, font, edge, o);
}

inline void ui_draw_brand(float64 cx, float64 top, float64 box_w, float64 box_h) {
    float64 h = ui_snap(box_h * g_ui_style.side_rt);
    float64 w = ui_rt_logo_w(h);
    if (w > box_w) {
        h = ui_snap(h * box_w / w);
        w = ui_rt_logo_w(h);
    }
    ui_draw_rt_logo(ui_snap(cx - w * 0.5), ui_snap(top + (box_h - h) * 0.5), h);
}

inline void ui_begin(const std::string& title, bool down, bool tap, bool rdown, bool rtap, float64 wheel) {
    ui_ensure_fonts();
    ui_sync_accent();
    ui_logo_load();
    g_ui_mouse = ui_in_mouse();
    g_ui_down = down;
    g_ui_pressed = (down && !g_ui_prev_down) || tap;
    g_ui_released = (!down && g_ui_prev_down) || tap;
    g_ui_prev_down = down;
    g_ui_rpressed = (rdown && !g_ui_prev_rdown) || rtap;
    g_ui_prev_rdown = rdown;
    g_ui_scroll = wheel;
    g_ui_next_id = 1;
    g_ui_need_w = 0.0;
    g_ui_closed = false;
    g_ui_body.live = false;
    g_ui_in_group = false;
    g_ui_tip.text = "";
    g_ui_icon.on = false;
    if (ui_capturing() && !g_ui_cap.seen)
        ui_capture_cancel();
    g_ui_cap.seen = false;
    // A press outside the box commits a typed edit and still goes on to whatever it landed on.
    if (ui_editing()) {
        if (!g_ui_edit.seen)
            ui_edit_end();
        else if (g_ui_pressed && !ui_mouse_in(g_ui_edit.x, g_ui_edit.y, g_ui_edit.w, g_ui_edit.h))
            g_ui_edit.commit = true;
        g_ui_edit.seen = false;
    }

    // The watermark is drawn over the menu, so it takes a press (and the scroll) on it before any widget can.
    bool wm_live = g_ui_wm.live;
    g_ui_wm.live = false;
    if (wm_live && ui_mouse_in(g_ui_wm.x, g_ui_wm.y, g_ui_wm.w, g_ui_wm.h)) {
        g_ui_scroll = 0.0;
        if (ui_take_press(UI_ID_WATERMARK, true)) {
            g_ui_drag_dx = g_ui_mouse.x - g_ui_wm.x;
            g_ui_drag_dy = g_ui_mouse.y - g_ui_wm.y;
        }
    }
    for (uint64 n = g_ui_floats.size(); n > 0; n--) {
        ui_float_rect_t& fl = g_ui_floats[n - 1];
        bool float_live = fl.live;
        fl.live = false;
        if (float_live && g_ui_active == 0 && ui_mouse_in(fl.x, fl.y, fl.w, fl.h)) {
            g_ui_scroll = 0.0;
            if (ui_take_press(UI_ID_FLOAT - static_cast<int32>(n - 1), true)) {
                g_ui_drag_dx = g_ui_mouse.x - fl.x;
                g_ui_drag_dy = g_ui_mouse.y - fl.y;
            }
        }
    }
    ui_popup_input();
    ui_picker_input();
    g_ui_wrap_n = 0;
    g_ui_nav_last = g_ui_nav_n;
    g_ui_nav_n = 0;

    float64 sw = ui_snap(g_ui_m.side_w);
    float64 hh = ui_snap(g_ui_m.head_h);
    float64 fh = ui_snap(g_ui_m.foot_h);
    float64 pad = g_ui_m.pad;
    // Sidebar head: the RT logo, its subtitle, a separator, then the nav items from nav_top down to the window footer.
    float64 brand_top = ui_snap(pad * g_ui_style.side_logo_top);
    float64 brand_h = ui_snap(g_ui_m.logo * g_ui_style.side_brand);
    uint64 dash = title.find(" - ");
    std::string sub = g_ui_cfg.subtitle.size() != 0 ? g_ui_cfg.subtitle
        : (dash != std::string::npos ? title.substr(dash + 3) : "");
    float64 sh = sub.size() != 0 ? ui_gfx_text_size(g_ui_font_small, sub).y : 0.0;
    float64 sub_top = brand_top + brand_h + ui_snap(pad * g_ui_style.side_name_gap);
    float64 sep_y = ui_snap(sub_top + sh + pad);
    float64 nav_top = sep_y + ui_snap(pad);
    // Last frame's item count, so the window cannot be made too short for the nav.
    g_ui_side_need = nav_top + static_cast<float64>(g_ui_nav_last) * ui_snap(g_ui_m.nav_h) + ui_snap(pad) + fh;
    g_ui_side_need_1 = g_ui_side_need / max(g_ui_scale_applied, UI_SCALE_EPS);
    float64 min_w = ui_min_w_at(g_ui_scale_applied);
    float64 min_h = ui_min_h_at(g_ui_scale_applied);

    vec2 view = ui_gfx_view();
    // An overlay smaller than the minimum caps it: the body then drops to one column rather than leave the screen.
    if (view.x > 0.0 && view.y > 0.0) {
        min_w = min(min_w, view.x);
        min_h = min(min_h, view.y);
    }
    if (g_ui_x < 0.0 && view.x > 0.0) {
        g_ui_x = max((view.x - g_ui_w) * 0.5, 0.0);
        g_ui_y = max((view.y - g_ui_h) * 0.5, 0.0);
    }
    // The drag itself is taken by ui_end, from a press on the chrome that no widget took.
    g_ui_win_id = ui_id();
    if (g_ui_active == g_ui_win_id && g_ui_down) {
        g_ui_x = g_ui_mouse.x - g_ui_drag_dx;
        g_ui_y = g_ui_mouse.y - g_ui_drag_dy;
    }

    // Taken here, before any row, so the grip wins a press over content that reaches the corner.
    g_ui_grip_id = ui_id();
    float64 gs = g_ui_m.grip;
    bool grip_hot = ui_hovered(g_ui_x + g_ui_w - gs, g_ui_y + g_ui_h - gs, gs, gs);
    if (ui_take_press(g_ui_grip_id, grip_hot)) {
        g_ui_drag_dx = g_ui_mouse.x - (g_ui_x + g_ui_w);
        g_ui_drag_dy = g_ui_mouse.y - (g_ui_y + g_ui_h);
    }
    if (g_ui_active == g_ui_grip_id && g_ui_down) {
        g_ui_w = ui_clamp(g_ui_mouse.x - g_ui_drag_dx - g_ui_x, min_w, max(view.x - g_ui_x, min_w));
        g_ui_h = ui_clamp(g_ui_mouse.y - g_ui_drag_dy - g_ui_y, min_h, max(view.y - g_ui_y, min_h));
    }
    // Every frame, not only while dragging, so a resized overlay or a scale change cannot leave the window off-screen
    // or too small.
    g_ui_w = ui_snap(ui_clamp(g_ui_w, min_w, max(view.x, min_w)));
    g_ui_h = ui_snap(ui_clamp(g_ui_h, min_h, max(view.y, min_h)));
    if (g_ui_refit && view.y > 0.0) {
        g_ui_y = ui_clamp(g_ui_y, 0.0, max(view.y - g_ui_h, 0.0));
        g_ui_refit = false;
    }
    g_ui_x = ui_snap(ui_clamp(g_ui_x, 0.0, max(view.x - g_ui_w, 0.0)));
    g_ui_y = ui_snap(ui_clamp(g_ui_y, 0.0, max(view.y - g_ui_m.head_h, 0.0)));

    float64 x = g_ui_x;
    float64 y = g_ui_y;

    // Both surfaces take the border's own shape (ui_border_fill), so no corner shows them outside the accent border; the
    // sidebar rounds only the window's top-left corner and keeps its inner edges square.
    ui_border_fill(x, y, g_ui_w, g_ui_h, g_ui_theme.bg, 0);
    ui_gfx_fill_corners(vec2(x + 0.5, y + 0.5), vec2(sw - 0.5, g_ui_h - fh - 0.5), g_ui_theme.bg2, ui_border_round(),
        UI_CORNER_TL);
    ui_gfx_fill(vec2(x + sw - 1.0, y), vec2(1.0, g_ui_h - fh), g_ui_theme.border, 0.0);
    ui_gfx_fill(vec2(x + sw, y + hh), vec2(g_ui_w - sw, 1.0), g_ui_theme.border, 0.0);

    ui_draw_brand(x + sw * 0.5, y + brand_top, sw - 2.0 * ui_snap(pad), brand_h);
    if (sub.size() != 0)
        ui_text_center(sub, x, y + sub_top, sw, sh, g_ui_theme.muted, g_ui_font_small);
    ui_gfx_fill(vec2(x + pad, y + sep_y), vec2(sw - 2.0 * pad, 1.0), g_ui_theme.border, 0.0);
    g_ui_nav_y = y + nav_top;
    g_ui_nav_top = g_ui_nav_y;
    g_ui_nav_bottom = y + g_ui_h - fh;
    g_ui_tabs_left = x + sw + ui_snap(pad);
    g_ui_tabs_right = max(x + g_ui_w - ui_snap(pad), g_ui_tabs_left);
    g_ui_head_right = x + g_ui_w;

    g_ui_body.x = x + sw + ui_snap(pad);
    g_ui_body.y = y + hh + 1.0 + ui_snap(pad);
    g_ui_body.w = g_ui_w - sw - 2.0 * ui_snap(pad);
    g_ui_body.h = max(g_ui_h - hh - 1.0 - fh - 2.0 * ui_snap(pad), 0.0);
    g_ui_cursor = g_ui_body.y;
    ui_area_body();
    ui_gfx_clip_push(vec2(x, y), vec2(g_ui_w, g_ui_h));
}

inline bool ui_nav(const std::string& label, int32 index, int32& current) {
    float64 x = g_ui_x;
    float64 y = g_ui_nav_y;
    float64 w = ui_snap(g_ui_m.side_w) - 1.0;
    float64 h = ui_snap(g_ui_m.nav_h);
    float64 rh = max(g_ui_nav_bottom - g_ui_nav_top, 0.0);
    g_ui_nav_y = y + h;
    g_ui_nav_n = g_ui_nav_n + 1;
    int32 id = ui_id();
    bool hot = ui_hovered_in(x, g_ui_nav_top, w, rh, x, y, w, h);
    bool clicked = ui_button_behavior(id, hot);
    if (clicked)
        current = index;
    if (y >= g_ui_nav_bottom)
        return clicked;
    bool cut = y + h > g_ui_nav_bottom;
    if (cut)
        ui_gfx_clip_push(vec2(x, g_ui_nav_top), vec2(w, rh));
    bool sel = current == index;
    bool lit = ui_lit(hot, id);
    if (sel) {
        ui_gfx_fill(vec2(x, y), vec2(w, h), g_ui_theme.group, 0.0);
        ui_gfx_gradient(vec2(x, y), vec2(ui_snap(g_ui_m.accent_w), h), g_ui_theme.gradient, 0.0, false);
    } else if (lit) {
        ui_gfx_fill(vec2(x, y), vec2(w, h), g_ui_theme.hover, 0.0);
    }
    color c = sel || lit ? g_ui_theme.text : g_ui_theme.dim;
    ui_text_at(label, x + ui_snap(g_ui_m.pad * g_ui_style.nav_indent), y, h, c, sel ? g_ui_font_bold : g_ui_font);
    if (cut)
        ui_gfx_clip_pop();
    return clicked;
}

inline bool ui_head_square(float64 bx, float64 by, float64 s, const std::string& icon, int32 glyph, const std::string& tip, bool lit) {
    int32 id = ui_id();
    bool hot = ui_hovered(bx, by, s, s);
    bool clicked = ui_button_behavior(id, hot);
    bool hover = ui_lit(hot, id);
    if (hover)
        ui_gfx_fill(vec2(bx, by), vec2(s, s), g_ui_theme.hover, g_ui_m.round);
    if (lit) {
        float64 uh = ui_snap(g_ui_m.underline_h);
        ui_gfx_gradient(vec2(bx, g_ui_y + ui_snap(g_ui_m.head_h) - uh), vec2(s, uh), g_ui_theme.gradient, 0.0, true);
    }
    float64 gs = ui_snap(s * g_ui_style.head_glyph);
    bool shown = icon.size() != 0 && ui_icon_any(icon, bx + s * 0.5 - gs, by + s * 0.5 - gs, 2.0 * gs);
    if (!shown)
        ui_draw_glyph(glyph, bx + s * 0.5, by + s * 0.5, gs, lit ? ui_col_accent() : (hover ? g_ui_theme.text
            : g_ui_theme.dim));
    if (hot && g_ui_active == 0)
        g_ui_tip.text = tip;
    return clicked;
}

inline bool ui_head_button(const std::string& icon, int32 glyph, const std::string& tip, bool lit) {
    float64 hh = ui_snap(g_ui_m.head_h);
    float64 s = ui_snap(hh * g_ui_style.head_btn);
    float64 gap = ui_snap((hh - s) * 0.5);
    float64 bx = g_ui_head_right - gap - s;
    if (bx < g_ui_tabs_left)
        return false;
    g_ui_head_right = bx;
    g_ui_tabs_right = max(min(g_ui_tabs_right, bx - ui_snap(g_ui_m.pad * 0.5)), g_ui_tabs_left);
    return ui_head_square(bx, g_ui_y + gap, s, icon, glyph, tip, lit);
}

inline bool ui_head_back(const std::string& tip) {
    float64 hh = ui_snap(g_ui_m.head_h);
    float64 s = ui_snap(hh * g_ui_style.head_btn);
    float64 gap = ui_snap((hh - s) * 0.5);
    float64 bx = g_ui_tabs_left;
    if (bx + s > g_ui_tabs_right)
        return false;
    g_ui_tabs_left = bx + s + ui_snap(g_ui_m.pad * 0.5);
    return ui_head_square(bx, g_ui_y + gap, s, "", UI_GLYPH_BACK, tip, false);
}

inline bool ui_tabs(const std::vector<std::string>& labels, int32& current) {
    uint64 n = labels.size();
    if (n == 0)
        return false;
    float64 left = g_ui_tabs_left;
    float64 avail = max(g_ui_tabs_right - left, 0.0);
    float64 y = g_ui_y;
    float64 h = ui_snap(g_ui_m.head_h);
    float64 nf = static_cast<float64>(n);
    if (g_ui_tab_w.size() != n)
        g_ui_tab_w.resize(n);
    float64 sum = 0.0;
    for (uint64 i = 0; i < n; i++) {
        g_ui_tab_w[i] = ui_snap(ui_gfx_text_size(g_ui_font_bold, labels[i]).x);
        sum = sum + g_ui_tab_w[i];
    }
    float64 tp = ui_snap(g_ui_m.tab_pad);
    if (sum + 2.0 * tp * nf > avail)
        tp = max(static_cast<float64>(static_cast<int64>((avail - sum) / (2.0 * nf))),
            ui_snap(g_ui_m.tab_pad * g_ui_style.tab_pad_min));
    float64 budget = max(avail - 2.0 * tp * nf, 0.0);
    float64 cap = sum;
    if (sum > budget) {
        cap = budget / nf;
        for (uint64 pass = 0; pass < n; pass++) {
            float64 below = 0.0;
            float64 above = 0.0;
            for (float64 tw : g_ui_tab_w) {
                if (tw <= cap)
                    below = below + tw;
                else
                    above = above + 1.0;
            }
            if (above == 0.0)
                break;
            cap = (budget - below) / above;
        }
        cap = max(static_cast<float64>(static_cast<int64>(cap)), 0.0);
    }

    bool clicked = false;
    float64 x = left;
    ui_gfx_clip_push(vec2(left, y), vec2(avail, h));
    for (uint64 i = 0; i < n; i++) {
        int32 index = static_cast<int32>(i);
        float64 tw = g_ui_tab_w[i];
        float64 lw = min(tw, cap);
        float64 w = min(lw + 2.0 * tp, max(left + avail - x, 0.0));
        int32 id = ui_id();
        bool hot = w > 0.0 && ui_hovered_in(left, y, avail, h, x, y, w, h);
        if (ui_button_behavior(id, hot)) {
            current = index;
            clicked = true;
        }
        bool sel = current == index;
        int64 font = sel ? g_ui_font_bold : g_ui_font;
        bool cut = tw > lw;
        if (w > 0.0) {
            color c = sel || ui_lit(hot, id) ? g_ui_theme.text : g_ui_theme.dim;
            ui_text_center(cut ? ui_fit_text_in(font, labels[i], lw) : labels[i], x, y, w, h, c, font);
            if (sel) {
                float64 uh = ui_snap(g_ui_m.underline_h);
                ui_gfx_gradient(vec2(x + tp, y + h - uh), vec2(lw, uh), g_ui_theme.gradient, 0.0, true);
            }
        }
        if (cut && hot && g_ui_active == 0)
            g_ui_tip.text = labels[i];
        x = x + w;
    }
    ui_gfx_clip_pop();
    return clicked;
}

inline void ui_body_split(float64 side_w, float64& cx, float64& cy, float64& cw, float64& ch) {
    float64 pad = ui_snap(g_ui_m.pad);
    float64 sw = ui_snap(min(side_w, max(g_ui_body.w - pad, 0.0)));
    cx = g_ui_body.x;
    cy = g_ui_body.y;
    cw = max(g_ui_body.w - sw - pad, 0.0);
    ch = g_ui_body.h;
    g_ui_body.x = g_ui_body.x + g_ui_body.w - sw;
    g_ui_body.w = sw;
    ui_area_body();
}

inline void ui_body_begin(int64 page, int32 cols) {
    float64 pad = ui_snap(g_ui_m.pad);
    int32 n = cols > 1 ? cols : 1;
    float64 cw = (g_ui_body.w - pad * static_cast<float64>(n - 1)) / static_cast<float64>(n);
    float64 col_min = max(g_ui_m.col_min_w, g_ui_need_col_1 * g_ui_scale_applied);
    // A couple of pixels of slack: ui_min_w_at works unsnapped.
    while (n > 1 && cw + g_ui_style.col_slack < col_min) {
        n = n - 1;
        cw = (g_ui_body.w - pad * static_cast<float64>(n - 1)) / static_cast<float64>(n);
    }
    if (g_ui_page_off.count(page) == 0)
        g_ui_page_off[page] = 0.0;
    g_ui_body.page = page;
    g_ui_body.off = g_ui_page_off[page];
    g_ui_body.cols = n;
    g_ui_body.col_w = ui_snap(cw);
    if (g_ui_body.col_y.size() != static_cast<uint64>(n))
        g_ui_body.col_y.resize(static_cast<uint64>(n));
    for (uint64 i = 0; i < g_ui_body.col_y.size(); i++)
        g_ui_body.col_y[i] = g_ui_body.y - g_ui_body.off;
    g_ui_body.live = true;
    ui_area_body();
    ui_gfx_clip_push(vec2(g_ui_body.x, g_ui_body.y), vec2(g_ui_body.w, g_ui_body.h));
}

inline float64 ui_scroll_step() {
    return (g_ui_scroll > 0.0 ? -1.0 : 1.0) * g_ui_m.row_h * UI_SCROLL_ROWS;
}

inline void ui_body_end() {
    float64 pad = ui_snap(g_ui_m.pad);
    float64 top = g_ui_body.y - g_ui_body.off;
    float64 bottom = top;
    for (float64 cy : g_ui_body.col_y)
        bottom = max(bottom, cy - pad);
    float64 content = bottom - top;
    float64 max_off = max(content - g_ui_body.h, 0.0);
    bool over = ui_hovered(g_ui_body.x, g_ui_body.y, g_ui_body.w, g_ui_body.h);
    if (over && g_ui_scroll != 0.0 && g_ui_active == 0) {
        g_ui_body.off = g_ui_body.off + ui_scroll_step();
        g_ui_scroll = 0.0;
    }
    ui_gfx_clip_pop();
    g_ui_body.live = false;
    float64 th = content > 0.0 ? max(g_ui_body.h * g_ui_body.h / content, g_ui_m.row_h) : g_ui_body.h;
    float64 gx = g_ui_body.x + g_ui_body.w;
    bool bar_hot = max_off > 0.0 && ui_hovered(gx, g_ui_body.y, pad, g_ui_body.h);
    if (bar_hot && g_ui_scroll != 0.0 && g_ui_active == 0) {
        g_ui_body.off = g_ui_body.off + ui_scroll_step();
        g_ui_scroll = 0.0;
    }
    float64 ty = max_off > 0.0 ? g_ui_body.y + (g_ui_body.h - th) * ui_clamp(g_ui_body.off, 0.0, max_off) / max_off
        : g_ui_body.y;
    if (ui_take_press(UI_ID_SCROLLBAR, bar_hot)) {
        bool on_thumb = g_ui_mouse.y >= ty && g_ui_mouse.y < ty + th;
        g_ui_drag_dy = on_thumb ? g_ui_mouse.y - ty : th * 0.5;
    }
    bool dragging = g_ui_active == UI_ID_SCROLLBAR && g_ui_down;
    if (dragging && max_off > 0.0) {
        float64 t = ui_clamp((g_ui_mouse.y - g_ui_drag_dy - g_ui_body.y) / max(g_ui_body.h - th, 1.0), 0.0, 1.0);
        g_ui_body.off = t * max_off;
    }
    g_ui_body.off = ui_clamp(g_ui_body.off, 0.0, max_off);
    g_ui_page_off[g_ui_body.page] = g_ui_body.off;
    if (max_off <= 0.0)
        return;
    ty = g_ui_body.y + (g_ui_body.h - th) * g_ui_body.off / max_off;
    float64 bw = max(ui_snap(g_ui_m.scroll_w), g_ui_style.bar_min);
    bool lit = dragging || (bar_hot && g_ui_active == 0);
    float64 bx = ui_snap(gx + (pad - bw) * 0.5);
    ui_gfx_fill(vec2(bx, g_ui_body.y), vec2(bw, g_ui_body.h), g_ui_theme.bg2, bw * 0.5);
    ui_scroll_thumb(gx, pad, ty, th, lit, g_ui_theme.border_hi, g_ui_theme.dim);
}

inline std::string ui_group_label(const std::string& title) {
    uint64 at = title.find("##");
    return at != std::string::npos ? title.substr(0, at) : title;
}

inline bool ui_group_begin(const std::string& title, int32 col) {
    int32 c = g_ui_body.cols > 0 ? col % g_ui_body.cols : 0;
    float64 pad = ui_snap(g_ui_m.pad);
    float64 x = g_ui_body.x + static_cast<float64>(c) * (g_ui_body.col_w + pad);
    float64 y = ui_snap(g_ui_body.col_y[static_cast<uint64>(c)]);
    float64 w = c == g_ui_body.cols - 1 ? g_ui_body.x + g_ui_body.w - x : g_ui_body.col_w;
    float64 hh = ui_snap(g_ui_m.group_title_h);
    bool open = !(g_ui_group_closed.count(title) != 0 && g_ui_group_closed[title]);
    int32 id = ui_id();
    bool hot = ui_hovered(x, y, w, hh);
    if (ui_button_behavior(id, hot)) {
        open = !open;
        g_ui_group_closed[title] = !open;
    }
    float64 h = hh;
    if (open && g_ui_group_h.count(title) != 0)
        h = max(g_ui_group_h[title], hh);
    g_ui_grp.title = title;
    g_ui_grp.open = open;
    g_ui_grp.col = c;
    g_ui_grp.x = x;
    g_ui_grp.y = y;
    g_ui_grp.w = w;

    bool lit = ui_lit(hot, id);
    ui_draw_box(x, y, w, ui_snap(h), g_ui_theme.group, lit ? g_ui_theme.border_hi : g_ui_theme.border);
    color top = g_ui_theme.card_top;
    top.a = lit ? g_ui_theme.head_a_lit : g_ui_theme.head_a;
    color base = g_ui_theme.group;
    base.a = 0;
    ui_gfx_corners(vec2(x + 1.0, y + 1.0), vec2(w - 2.0, hh - 1.0), top, top, base, base, g_ui_m.round);
    float64 aw = ui_snap(g_ui_m.accent_w);
    color bar = open ? ui_col_accent() : g_ui_theme.border_hi;
    float64 bar_y = y + ui_snap(hh * g_ui_style.group_bar_top);
    ui_gfx_fill(vec2(x + 1.0, bar_y), vec2(aw, ui_snap(hh * g_ui_style.group_bar_h)), bar, 0.0);
    std::string label = ui_group_label(title);
    ui_text_at(label, x + pad + aw, y, hh, open || lit ? g_ui_theme.text : g_ui_theme.dim, g_ui_font_bold);
    float64 chev_x = hh * g_ui_style.group_chevron_x;
    ui_need(aw + ui_gfx_text_size(g_ui_font_bold, label).x + 2.0 * chev_x + pad);
    ui_draw_chevron(x + w - pad - chev_x, y + hh * 0.5, hh * g_ui_style.group_chevron, open,
        lit ? g_ui_theme.text : g_ui_theme.muted);
    if (!open)
        return false;
    ui_gfx_fill(vec2(x + 1.0, y + hh), vec2(w - 2.0, 1.0), g_ui_theme.border, 0.0);
    // Cut to the body as well, in case nested clips do not intersect.
    g_ui_in_group = true;
    g_ui_clip_x = x + 1.0;
    g_ui_clip_y = max(y + hh + 1.0, g_ui_body.y);
    g_ui_clip_w = w - 2.0;
    g_ui_clip_h = max(g_ui_body.y + g_ui_body.h - g_ui_clip_y, 0.0);
    ui_gfx_clip_push(vec2(g_ui_clip_x, g_ui_clip_y), vec2(g_ui_clip_w, g_ui_clip_h));
    g_ui_area_x = x + pad;
    g_ui_area_w = w - 2.0 * pad;
    g_ui_cursor = y + hh + 1.0 + ui_snap(pad * 0.5);
    return true;
}

inline void ui_group_end() {
    float64 h = ui_snap(g_ui_m.group_title_h);
    if (g_ui_grp.open) {
        ui_gfx_clip_pop();
        h = ui_snap(g_ui_cursor + g_ui_m.pad * 0.5 - g_ui_grp.y);
        g_ui_group_h[g_ui_grp.title] = h;
    }
    g_ui_body.col_y[static_cast<uint64>(g_ui_grp.col)] = g_ui_grp.y + h + ui_snap(g_ui_m.pad);
    g_ui_in_group = false;
    ui_area_body();
}

inline void ui_footer(const std::vector<std::string>& texts, const std::vector<color>& cols) {
    float64 fh = ui_snap(g_ui_m.foot_h);
    float64 fy = g_ui_y + g_ui_h - fh;
    ui_gfx_fill(vec2(g_ui_x, fy), vec2(g_ui_w, 1.0), g_ui_theme.border, 0.0);
    float64 r = g_ui_m.dot_r;
    // The gaps are the clear pixels between the logo and the 1 px line on each side: the footer's top line (row fy) and
    // the window's own border (ui_gradient_border: column g_ui_x, row g_ui_y + g_ui_h - 1), then the first indicator.
    // The logo square fills what is left between the two rows less two gaps, so the gaps stay equal after snapping.
    float64 room = fh - 2.0;
    float64 gap = ui_snap((room - ui_snap(fh * g_ui_style.foot_logo)) * 0.5);
    float64 ls = room - 2.0 * gap;
    float64 lx = g_ui_x + 1.0 + gap;
    float64 ly = fy + 1.0 + gap;
    ui_draw_logo(lx + ls * 0.5, ly, ls);
    float64 x = lx + ls + gap;
    g_ui_logo.foot_x = lx;
    g_ui_logo.foot_y = ly;
    g_ui_logo.foot_s = ls;
    g_ui_logo.foot_next_x = x;
    for (uint64 i = 0; i < texts.size() && i < cols.size(); i++) {
        color c = cols[i];
        int64 font = c == g_ui_theme.muted ? g_ui_font : g_ui_font_bold;
        ui_gfx_disc(vec2(x + r, fy + fh * 0.5), r, c);
        float64 tx = ui_snap(x + 2.0 * r + g_ui_m.pad * g_ui_style.footer_dot_gap);
        ui_text_at(texts[i], tx, fy, fh, c, font);
        x = tx + ui_gfx_text_size(font, texts[i]).x + ui_snap(g_ui_m.pad * g_ui_style.footer_gap);
    }
}

inline void ui_resize_grip() {
    float64 gs = g_ui_m.grip;
    bool hot = ui_hovered(g_ui_x + g_ui_w - gs, g_ui_y + g_ui_h - gs, gs, gs);
    bool held = g_ui_active == g_ui_grip_id;
    color c = held ? ui_col_accent() : (hot && g_ui_active == 0 ? g_ui_theme.dim : g_ui_theme.muted);
    float64 d = max(ui_snap(g_ui_m.grip_dot), g_ui_style.bar_min);
    float64 step = d + max(ui_snap(g_ui_m.grip_gap), 1.0);
    // Clear of the window's rounded corner and its 1 px border.
    float64 in = ui_snap(g_ui_m.round) + 1.0;
    float64 r = g_ui_x + g_ui_w - in - d;
    float64 b = g_ui_y + g_ui_h - in - d;
    int32 last = g_ui_style.grip_n - 1;
    for (int32 i = 0; i <= last; i++) {
        for (int32 j = 0; j <= last; j++) {
            if (i + j < last)
                continue;
            float64 dx = static_cast<float64>(last - j) * step;
            float64 dy = static_cast<float64>(last - i) * step;
            ui_gfx_fill(vec2(r - dx, b - dy), vec2(d, d), c, 0.0);
        }
    }
}

inline void ui_end() {
    // A press on the chrome (sidebar, top bar, footer) that no widget took drags the window.
    if (g_ui_pressed) {
        float64 sw = ui_snap(g_ui_m.side_w);
        float64 hh = ui_snap(g_ui_m.head_h);
        bool in_win = ui_mouse_in(g_ui_x, g_ui_y, g_ui_w, g_ui_h);
        bool in_body = ui_mouse_in(g_ui_x + sw, g_ui_y + hh + 1.0, g_ui_w - sw,
            g_ui_h - hh - 1.0 - ui_snap(g_ui_m.foot_h));
        if (ui_take_press(g_ui_win_id, in_win && !in_body)) {
            g_ui_drag_dx = g_ui_mouse.x - g_ui_x;
            g_ui_drag_dy = g_ui_mouse.y - g_ui_y;
        } else {
            g_ui_pressed = false;
            g_ui_active = UI_ID_NOWHERE;
        }
    }
    ui_resize_grip();
    ui_gradient_border(g_ui_x, g_ui_y, g_ui_w, g_ui_h);
    ui_gfx_clip_pop();
    ui_popup_draw();
    ui_picker_draw();
    ui_tooltip_draw();
    ui_pic_pump();
    // Before ui_settle_scale, which resets it on a scale change.
    float64 col = g_ui_need_w + 2.0 * ui_snap(g_ui_m.pad);
    g_ui_need_col_1 = max(g_ui_need_col_1, col / max(g_ui_scale_applied, UI_SCALE_EPS));
    ui_settle_scale();
    if (g_ui_released || !g_ui_down)
        g_ui_active = 0;
}

inline bool ui_close_clicked() {
    return g_ui_closed;
}

inline void ui_tip_layout() {
    const std::string& text = g_ui_tip.text;
    g_ui_tip.wrapped = text;
    g_ui_tip.font = g_ui_font;
    g_ui_tip.kind = -1;
    uint64 skip = 0;
    for (uint64 i = 0; i < g_ui_cfg.tip_prefixes.size(); i++) {
        uint64 n = g_ui_cfg.tip_prefixes[i].size();
        if (text.size() >= n && text.substr(0, n) == g_ui_cfg.tip_prefixes[i]) {
            g_ui_tip.kind = static_cast<int32>(i);
            skip = n;
            break;
        }
    }
    while (skip > 0 && skip < text.size() && (text.substr(skip, 1) == " " || text.substr(skip, 1) == "\n"))
        skip = skip + 1;
    ui_wrap_lines(skip > 0 ? text.substr(skip) : text, g_ui_m.tip_max_w, g_ui_font, g_ui_tip.lines);
    g_ui_tip.w = 0.0;
    for (const std::string& l : g_ui_tip.lines)
        g_ui_tip.w = max(g_ui_tip.w, ui_gfx_text_size(g_ui_font, l).x);
    if (g_ui_tip.kind >= 0) {
        float64 hw = ui_gfx_text_size(g_ui_font_bold, g_ui_cfg.tip_heads[static_cast<uint64>(g_ui_tip.kind)]).x;
        g_ui_tip.w = max(g_ui_tip.w, ui_snap(2.0 * g_ui_m.dot_r) + ui_snap(g_ui_m.pad * g_ui_style.tip_head_gap) + hw);
    }
}

inline void ui_tooltip_draw() {
    int64 now = ui_sys_ms();
    if (g_ui_tip.text.size() == 0) {
        g_ui_tip.since = 0;
        return;
    }
    if (g_ui_tip.since == 0 || now - g_ui_tip.seen_at > UI_TIP_GAP_MS)
        g_ui_tip.since = now;
    g_ui_tip.seen_at = now;
    int64 shown = now - g_ui_tip.since - UI_TIP_DELAY_MS;
    if (shown < 0)
        return;
    float64 f = ui_clamp(static_cast<float64>(shown) / static_cast<float64>(UI_TIP_FADE_MS), 0.0, 1.0);
    if (g_ui_tip.wrapped != g_ui_tip.text || g_ui_tip.font != g_ui_font)
        ui_tip_layout();
    float64 pad = ui_snap(g_ui_m.pad);
    float64 py = ui_snap(pad * g_ui_style.tip_pad_y);
    float64 lh = ui_snap(g_ui_font_h + g_ui_m.line_gap);
    float64 gap = ui_snap(pad * 0.5);
    bool head = g_ui_tip.kind >= 0;
    uint64 n = g_ui_tip.lines.size();
    float64 head_h = head ? py + lh + (n != 0 ? gap : py) : 0.0;
    float64 body_h = n != 0 ? static_cast<float64>(n) * lh + 2.0 * py - (head ? py - gap : 0.0) : 0.0;
    float64 w = ui_snap(g_ui_tip.w + 2.0 * pad);
    float64 h = head_h + (head && n != 0 ? 1.0 : 0.0) + body_h;
    vec2 view = ui_gfx_view();
    float64 m = ui_snap(pad * 0.5);
    float64 x = g_ui_mouse.x + pad * g_ui_style.tip_off_x;
    if (x + w > view.x - m)
        x = g_ui_mouse.x - pad - w;
    float64 y = g_ui_mouse.y + pad * g_ui_style.tip_off_y;
    if (y + h > view.y - m)
        y = g_ui_mouse.y - pad - h;
    x = ui_snap(ui_clamp(x, m, max(view.x - w - m, m)));
    y = ui_snap(ui_clamp(y, m, max(view.y - h - m, m)));

    float64 r = ui_snap(g_ui_m.round * g_ui_style.tip_round);
    // The shadow drops sd under the panel; the spread one also reaches ss past its sides and bottom.
    float64 sd = g_ui_style.tip_shadow_drop;
    float64 ss = g_ui_style.tip_shadow_spread;
    color spread = ui_fade(g_ui_theme.shadow, f * g_ui_theme.tip_shadow_a);
    ui_gfx_fill(vec2(x - ss, y + sd), vec2(w + 2.0 * ss, h + 2.0 * sd + ss), spread, r + 2.0 * ss);
    ui_gfx_fill(vec2(x, y + sd), vec2(w, h + sd), ui_fade(g_ui_theme.shadow, f), r);
    ui_gfx_fill(vec2(x, y), vec2(w, h), ui_fade(g_ui_theme.bg2, f), r);
    float64 ly = y;
    if (head) {
        color hc = g_ui_cfg.tip_cols[static_cast<uint64>(g_ui_tip.kind)];
        // Top corners only, so the band meets the hairline square.
        color band = ui_fade(hc, f * g_ui_theme.tip_band_a);
        ui_gfx_fill_corners(vec2(x + 1.0, y + 1.0), vec2(w - 2.0, head_h - 1.0), band, r, UI_CORNER_TL | UI_CORNER_TR);
        float64 dr = g_ui_m.dot_r;
        float64 cy = ly + py + lh * 0.5;
        ui_gfx_disc(vec2(x + pad + dr, cy), dr, ui_fade(hc, f));
        const std::string& name = g_ui_cfg.tip_heads[static_cast<uint64>(g_ui_tip.kind)];
        float64 tx = x + pad + ui_snap(2.0 * dr) + ui_snap(pad * g_ui_style.tip_head_gap);
        ui_gfx_text(name, vec2(tx, ui_snap(cy - g_ui_font_h * 0.5)), ui_fade(hc, f), g_ui_font_bold);
        ly = y + head_h;
        if (n != 0) {
            ui_gfx_fill(vec2(x + 1.0, ly), vec2(w - 2.0, 1.0), ui_fade(g_ui_theme.border, f), 0.0);
            ly = ly + 1.0 + gap;
        }
    } else {
        ly = y + py;
    }
    for (const std::string& l : g_ui_tip.lines) {
        ui_gfx_text(l, vec2(x + pad, ly + ui_snap((lh - g_ui_font_h) * 0.5)), ui_fade(g_ui_theme.text, f), g_ui_font);
        ly = ly + lh;
    }
    ui_gfx_frame(vec2(x + 0.5, y + 0.5), vec2(w - 1.0, h - 1.0), ui_fade(g_ui_theme.border_hi, f), 1.0, r);
}
