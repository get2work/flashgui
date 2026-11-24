inline uint8 ui_channel(float64 x) {
    return static_cast<uint8>(ui_clamp(x, 0.0, 1.0) * 255.0 + 0.5);
}

inline color ui_hsv_color(float64 h, float64 s, float64 v, uint8 a) {
    float64 hh = ui_clamp(h, 0.0, 1.0) * 6.0;
    if (hh >= 6.0)
        hh = 0.0;
    int32 i = static_cast<int32>(hh);
    float64 f = hh - static_cast<float64>(i);
    float64 p = v * (1.0 - s);
    float64 q = v * (1.0 - s * f);
    float64 t = v * (1.0 - s * (1.0 - f));
    if (i == 1)
        return color(ui_channel(q), ui_channel(v), ui_channel(p), a);
    if (i == 2)
        return color(ui_channel(p), ui_channel(v), ui_channel(t), a);
    if (i == 3)
        return color(ui_channel(p), ui_channel(q), ui_channel(v), a);
    if (i == 4)
        return color(ui_channel(t), ui_channel(p), ui_channel(v), a);
    if (i == 5)
        return color(ui_channel(v), ui_channel(p), ui_channel(q), a);
    return color(ui_channel(v), ui_channel(t), ui_channel(p), a);
}

inline float64 ui_hue_of(color c) {
    float64 r = static_cast<float64>(c.r) / 255.0;
    float64 g = static_cast<float64>(c.g) / 255.0;
    float64 b = static_cast<float64>(c.b) / 255.0;
    float64 mx = max(r, max(g, b));
    float64 d = mx - min(r, min(g, b));
    if (d <= 0.0)
        return 0.0;
    float64 h = mx == r ? (g - b) / d : (mx == g ? 2.0 + (b - r) / d : 4.0 + (r - g) / d);
    h = h / 6.0;
    return h < 0.0 ? h + 1.0 : h;
}

inline void ui_sat_val_of(color c, float64& s, float64& v) {
    float64 r = static_cast<float64>(c.r) / 255.0;
    float64 g = static_cast<float64>(c.g) / 255.0;
    float64 b = static_cast<float64>(c.b) / 255.0;
    v = max(r, max(g, b));
    s = v > 0.0 ? (v - min(r, min(g, b))) / v : 0.0;
}

inline void ui_sync_accent() {
    color a = g_ui_theme.accent;
    float64 s = 0.0;
    float64 v = 0.0;
    float64 base_s = 0.0;
    float64 base_v = 0.0;
    ui_sat_val_of(a, s, v);
    ui_sat_val_of(UI_GRAD_BASE, base_s, base_v);
    if (g_ui_theme.gradient.size() != 2)
        g_ui_theme.gradient.resize(2);
    g_ui_theme.gradient[0] = color(a.r, a.g, a.b, 255);
    g_ui_theme.gradient[1] = ui_hsv_color(ui_hue_of(a), UI_GRAD_DARK_S * min(s / base_s, 1.0),
        UI_GRAD_DARK_V * min(v / base_v, 1.0), 255);
    g_ui_theme.wm_dot = color(a.r, a.g, a.b, 255);
}

inline std::string ui_hex_of(color c) {
    return std::format("#{:02X}{:02X}{:02X}{:02X}", static_cast<int32>(c.r), static_cast<int32>(c.g),
        static_cast<int32>(c.b), static_cast<int32>(c.a));
}

inline bool ui_parse_hex(const std::string& text, color& c) {
    std::string t = text.size() != 0 && text.substr(0, 1) == "#" ? text.substr(1) : text;
    if (t.size() != 6 && t.size() != 8)
        return false;
    std::vector<int64> nib;
    for (uint64 i = 0; i < t.size(); i++) {
        int64 k = static_cast<int64>(t[i]);
        if (k >= UI_CHAR_0 && k < UI_CHAR_0 + 10)
            nib.push_back(k - UI_CHAR_0);
        else if (k >= UI_CHAR_UPPER_A && k < UI_CHAR_UPPER_A + 6)
            nib.push_back(k - UI_CHAR_UPPER_A + 10);
        else if (k >= UI_CHAR_LOWER_A && k < UI_CHAR_LOWER_A + 6)
            nib.push_back(k - UI_CHAR_LOWER_A + 10);
        else
            return false;
    }
    c.r = static_cast<uint8>(nib[0] * 16 + nib[1]);
    c.g = static_cast<uint8>(nib[2] * 16 + nib[3]);
    c.b = static_cast<uint8>(nib[4] * 16 + nib[5]);
    if (nib.size() == 8)
        c.a = static_cast<uint8>(nib[6] * 16 + nib[7]);
    return true;
}

inline bool ui_picker_is_open(const std::string& label) {
    return g_ui_pk.open && g_ui_pk.label == label;
}

inline void ui_picker_set(color c) {
    g_ui_pk.col = c;
    float64 r = static_cast<float64>(c.r) / 255.0;
    float64 g = static_cast<float64>(c.g) / 255.0;
    float64 b = static_cast<float64>(c.b) / 255.0;
    float64 mx = max(r, max(g, b));
    float64 d = mx - min(r, min(g, b));
    g_ui_pk.val = mx;
    if (mx > 0.0)
        g_ui_pk.sat = d / mx;
    if (d <= 0.0)
        return;
    float64 h = mx == r ? (g - b) / d : (mx == g ? 2.0 + (b - r) / d : 4.0 + (r - g) / d);
    h = h / 6.0;
    g_ui_pk.hue = h < 0.0 ? h + 1.0 : h;
}

inline void ui_draw_checker(ui_checker_t& ck, float64 x, float64 y, float64 w, float64 h, float64 cell) {
    if (ck.x != x || ck.y != y || ck.w != w || ck.h != h || ck.cell != cell) {
        ck.x = x;
        ck.y = y;
        ck.w = w;
        ck.h = h;
        ck.cell = cell;
        ck.cells.clear();
        int32 cols = static_cast<int32>((w + cell - 1.0) / cell);
        int32 rows = static_cast<int32>((h + cell - 1.0) / cell);
        for (int32 j = 0; j < rows; j++) {
            for (int32 i = (j % 2 == 0) ? 1 : 0; i < cols; i = i + 2) {
                float64 cx = x + static_cast<float64>(i) * cell;
                float64 cy = y + static_cast<float64>(j) * cell;
                ck.cells.push_back(static_cast<float32>(cx));
                ck.cells.push_back(static_cast<float32>(cy));
                ck.cells.push_back(static_cast<float32>(min(cell, x + w - cx)));
                ck.cells.push_back(static_cast<float32>(min(cell, y + h - cy)));
            }
        }
    }
    ui_gfx_fill(vec2(x, y), vec2(w, h), g_ui_theme.check_lo, 0.0);
    if (ck.cells.size() != 0)
        ui_gfx_fills(ck.cells, g_ui_theme.check_hi);
}

inline void ui_draw_swatch(float64 x, float64 y, float64 w, float64 h, color c, bool lit, bool open) {
    color border = open ? ui_col_accent() : (lit ? g_ui_theme.text : g_ui_theme.border_hi);
    ui_draw_box(x, y, w, h, c.with_alpha(255), border);
}

inline void ui_picker_open(const std::string& label, color c) {
    g_ui_pk.label = label;
    g_ui_pk.open = true;
    g_ui_pk.dirty = false;
    g_ui_pk.old = c;
    ui_picker_set(c);
    // Its list would sit under the picker.
    g_ui_pop.label = "";
}

inline bool ui_picker_bind(const std::string& label, color& c, float64 x, float64 y, float64 w, float64 h) {
    if (g_ui_pk.label != label)
        return false;
    bool changed = false;
    if (g_ui_pk.dirty) {
        changed = !(c == g_ui_pk.col);
        c = g_ui_pk.col;
        g_ui_pk.dirty = false;
    } else if (!(c == g_ui_pk.col)) {
        ui_picker_set(c);
    }
    float64 vis_top = g_ui_in_group ? g_ui_clip_y : g_ui_body.y;
    float64 vis_bottom = g_ui_in_group ? g_ui_clip_y + g_ui_clip_h : g_ui_body.y + g_ui_body.h;
    float64 mid = y + h * 0.5;
    if (g_ui_body.live && (mid < vis_top || mid > vis_bottom))
        g_ui_pk.open = false;
    // Closed by this frame's press it is still drawn once, so a value being typed commits.
    if (!g_ui_pk.open && !g_ui_pk.live) {
        g_ui_pk.label = "";
        return changed;
    }
    g_ui_pk.seen = true;
    g_ui_pk.ax = x;
    g_ui_pk.ay = y;
    g_ui_pk.aw = w;
    g_ui_pk.ah = h;
    return changed;
}

inline void ui_picker_input() {
    uint8 bits = 0;
    bool esc = (ui_in_key_bits(vk::escape, bits) && (bits & UI_KEY_DOWN) != 0) || ui_in_key_down(vk::escape);
    bool esc_press = esc && !g_ui_pk.esc_down;
    g_ui_pk.esc_down = esc;
    g_ui_pk.press = false;
    g_ui_pk.live = g_ui_pk.open;
    if (!g_ui_pk.live)
        return;
    bool over = ui_mouse_in(g_ui_pk.x, g_ui_pk.y, g_ui_pk.w, g_ui_pk.h);
    if (over)
        g_ui_scroll = 0.0;
    if (g_ui_pressed && over) {
        g_ui_pressed = false;
        g_ui_pk.press = true;
    } else if (g_ui_pressed) {
        g_ui_pressed = false;
        g_ui_active = UI_ID_NOWHERE;
        g_ui_pk.open = false;
    }
    if (g_ui_rpressed && over)
        g_ui_rpressed = false;
    if (esc_press && !ui_editing() && !ui_capturing() && g_ui_cap.mute_vk != vk::escape)
        g_ui_pk.open = false;
}

inline void ui_draw_bar_marker(float64 x, float64 y, float64 w) {
    float64 my = ui_snap(y);
    float64 mh = g_ui_style.marker_h;
    ui_gfx_frame(vec2(x - 1.5, my - mh * 0.5 - 1.0), vec2(w + 3.0, mh + 2.0), g_ui_theme.bg, 1.0, 0.0);
    ui_gfx_frame(vec2(x - 0.5, my - mh * 0.5), vec2(w + 1.0, mh), g_ui_theme.text, 1.0, 0.0);
}

inline bool ui_picker_drag(int32 id, float64 x, float64 y, float64 w, float64 h) {
    ui_take_press(id, ui_hovered(x, y, w, h));
    return g_ui_active == id && (g_ui_down || g_ui_released);
}

inline void ui_picker_draw() {
    if (g_ui_pk.label.size() != 0 && !g_ui_pk.seen) {
        g_ui_pk.open = false;
        g_ui_pk.label = "";
    }
    g_ui_pk.seen = false;
    if (g_ui_pk.label.size() == 0)
        return;
    float64 pad = ui_snap(g_ui_m.pad);
    float64 gap = ui_snap(pad * 0.5);
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 q = ui_snap(g_ui_m.row_h * g_ui_style.picker_square);
    float64 h = pad + q + pad + fh + gap + fh + pad;
    float64 w = h;
    float64 bw = ui_snap((w - q - 4.0 * pad) * 0.5);
    vec2 view = ui_gfx_view();
    float64 x = g_ui_pk.ax + g_ui_pk.aw + pad;
    if (x + w > view.x)
        x = g_ui_pk.ax - pad - w;
    x = ui_snap(ui_clamp(x, 0.0, max(view.x - w, 0.0)));
    float64 y = ui_snap(ui_clamp(g_ui_pk.ay, 0.0, max(view.y - h, 0.0)));
    g_ui_pk.x = x;
    g_ui_pk.y = y;
    g_ui_pk.w = w;
    g_ui_pk.h = h;

    int32 row_ids = g_ui_next_id;
    g_ui_next_id = UI_ID_PICKER_BASE;
    g_ui_pk.drawing = true;
    if (g_ui_pk.press)
        g_ui_pressed = true;
    float64 sx = x + pad;
    float64 sy = y + pad;
    float64 hx = sx + q + pad;
    float64 alx = hx + bw + pad;
    bool hsv_edit = false;
    if (ui_picker_drag(ui_id(), sx, sy, q, q)) {
        g_ui_pk.sat = ui_clamp((g_ui_mouse.x - sx) / q, 0.0, 1.0);
        g_ui_pk.val = 1.0 - ui_clamp((g_ui_mouse.y - sy) / q, 0.0, 1.0);
        hsv_edit = true;
    }
    if (ui_picker_drag(ui_id(), hx, sy, bw, q)) {
        g_ui_pk.hue = ui_clamp((g_ui_mouse.y - sy) / q, 0.0, 1.0);
        hsv_edit = true;
    }
    if (hsv_edit) {
        g_ui_pk.col = ui_hsv_color(g_ui_pk.hue, g_ui_pk.sat, g_ui_pk.val, g_ui_pk.col.a);
        g_ui_pk.dirty = true;
    }
    if (ui_picker_drag(ui_id(), alx, sy, bw, q)) {
        g_ui_pk.col.a = ui_channel(1.0 - ui_clamp((g_ui_mouse.y - sy) / q, 0.0, 1.0));
        g_ui_pk.dirty = true;
    }
    color col = g_ui_pk.col;

    ui_border_fill(x, y, w, h, g_ui_theme.bg, 0);
    ui_gradient_border(x, y, w, h);
    color pure = ui_hsv_color(g_ui_pk.hue, 1.0, 1.0, 255);
    ui_gfx_corners(vec2(sx, sy), vec2(q, q), UI_WHITE, pure, UI_BLACK, UI_BLACK, 0.0);
    ui_gfx_frame(vec2(sx + 0.5, sy + 0.5), vec2(q - 1.0, q - 1.0), g_ui_theme.border_hi, 1.0, 0.0);
    vec2 knob = vec2(ui_snap(sx + g_ui_pk.sat * q), ui_snap(sy + (1.0 - g_ui_pk.val) * q));
    float64 kr = max(ui_snap(g_ui_m.knob_r), g_ui_style.picker_knob_min);
    ui_gfx_ring(knob, kr + 1.0, g_ui_theme.bg, 1.0);
    ui_gfx_ring(knob, kr, UI_WHITE, ui_stroke_w());
    ui_gfx_gradient(vec2(hx, sy), vec2(bw, q), UI_HUE_STOPS, 0.0, false);
    ui_gfx_frame(vec2(hx + 0.5, sy + 0.5), vec2(bw - 1.0, q - 1.0), g_ui_theme.border_hi, 1.0, 0.0);
    ui_draw_bar_marker(hx, sy + g_ui_pk.hue * q, bw);
    float64 cell = max(ui_snap(bw * 0.5), g_ui_style.checker_min);
    ui_draw_checker(g_ui_pk.alpha_ck, alx, sy, bw, q, cell);
    std::vector<color> fade = { col.with_alpha(255), col.with_alpha(0) };
    ui_gfx_gradient(vec2(alx, sy), vec2(bw, q), fade, 0.0, false);
    ui_gfx_frame(vec2(alx + 0.5, sy + 0.5), vec2(bw - 1.0, q - 1.0), g_ui_theme.border_hi, 1.0, 0.0);
    ui_draw_bar_marker(alx, sy + (1.0 - static_cast<float64>(col.a) / 255.0) * q, bw);

    // Old and new colour over a checkerboard; a click on the old half reverts.
    float64 ry = sy + q + pad;
    float64 pw = ui_snap(q * g_ui_style.picker_preview);
    float64 half = ui_snap(pw * 0.5);
    int32 old_id = ui_id();
    bool old_hot = ui_hovered(sx, ry, half, fh);
    if (ui_button_behavior(old_id, old_hot)) {
        ui_picker_set(g_ui_pk.old);
        g_ui_pk.dirty = true;
        col = g_ui_pk.col;
    }
    ui_draw_checker(g_ui_pk.preview_ck, sx, ry, pw, fh, ui_snap(fh * g_ui_style.preview_cell));
    ui_gfx_fill(vec2(sx, ry), vec2(half, fh), g_ui_pk.old, 0.0);
    ui_gfx_fill(vec2(sx + half, ry), vec2(pw - half, fh), col, 0.0);
    color edge = ui_lit(old_hot, old_id) ? g_ui_theme.text : g_ui_theme.border_hi;
    ui_gfx_frame(vec2(sx + 0.5, ry + 0.5), vec2(pw - 1.0, fh - 1.0), edge, 1.0, 0.0);
    if (ui_lit(old_hot, old_id) && g_ui_active == 0)
        g_ui_tip.text = "Revert to the colour it had when opened";

    if (!(g_ui_pk.hex_of == col) && g_ui_edit.label != UI_PICKER_HEX) {
        g_ui_pk.hex = ui_hex_of(col);
        g_ui_pk.hex_of = col;
    }
    std::string hex = g_ui_pk.hex;
    float64 hxx = sx + pw + gap;
    ui_text_input(UI_PICKER_HEX, hex, "#RRGGBBAA", hxx, ry, x + w - pad - hxx, fh);
    if (hex != g_ui_pk.hex) {
        color nc = col;
        if (ui_parse_hex(hex, nc)) {
            ui_picker_set(nc);
            g_ui_pk.dirty = true;
            col = nc;
        }
        g_ui_pk.hex = ui_hex_of(col);
        g_ui_pk.hex_of = col;
    }

    float64 cy = ry + fh + gap;
    float64 nf = static_cast<float64>(UI_PICKER_FIELDS.size());
    float64 cw = ui_snap((w - 2.0 * pad - (nf - 1.0) * gap) / nf);
    float64 lw = ui_snap(ui_gfx_text_size(g_ui_font, "G").x) + gap;
    for (uint64 i = 0; i < UI_PICKER_FIELDS.size(); i++) {
        float64 cx = sx + static_cast<float64>(i) * (cw + gap);
        uint8 ch = i == 0 ? col.r : (i == 1 ? col.g : (i == 2 ? col.b : col.a));
        float64 v = static_cast<float64>(ch);
        ui_text_at(UI_CHANNEL_NAMES[i], cx, cy, fh, g_ui_theme.dim, g_ui_font);
        if (ui_value_field(UI_PICKER_FIELDS[i], v, 0.0, 255.0, 1.0, cx + lw, cy, cw - lw, fh, true)) {
            uint8 nv = static_cast<uint8>(v);
            if (i == 0)
                col.r = nv;
            else if (i == 1)
                col.g = nv;
            else if (i == 2)
                col.b = nv;
            else
                col.a = nv;
            ui_picker_set(col);
            g_ui_pk.dirty = true;
        }
    }

    g_ui_pk.drawing = false;
    g_ui_next_id = row_ids;
    // A press on the picker's background is used up here.
    if (g_ui_pk.press && g_ui_pressed) {
        g_ui_pressed = false;
        g_ui_active = UI_ID_PICKER;
    }
    g_ui_pk.press = false;
}

inline bool ui_color_picker(const std::string& label, color& c) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    int32 id = ui_id();
    bool hot = ui_hovered(x, y, w, rh);
    if (ui_button_behavior(id, hot))
        ui_picker_open(label, c);
    float64 sw = ui_snap(g_ui_m.swatch_w);
    float64 sh = ui_snap(g_ui_m.toggle_h);
    float64 sx = x + w - sw;
    float64 sy = ui_snap(y + (rh - sh) * 0.5);
    bool changed = ui_picker_bind(label, c, sx, sy, sw, sh);
    std::string hex = std::format("#{:02X}{:02X}{:02X}", static_cast<int32>(c.r), static_cast<int32>(c.g),
        static_cast<int32>(c.b));
    float64 hw = ui_gfx_text_size(g_ui_font, hex).x;
    float64 hx = sx - g_ui_m.pad - hw;
    ui_need(ui_gfx_text_size(g_ui_font, label).x + 2.0 * g_ui_m.pad + hw + sw);
    ui_text_at(hex, hx, y, rh, g_ui_theme.dim, g_ui_font);
    ui_label(label, x, y, rh, hx - g_ui_m.pad - x);
    ui_draw_swatch(sx, sy, sw, sh, c, ui_lit(hot, id), ui_picker_is_open(label));
    return changed;
}

inline bool ui_checkbox_color_button(const std::string& label, bool& v, color& c, const std::string& button, bool& clicked) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 pad = g_ui_m.pad;
    float64 bs = ui_snap(g_ui_m.box);
    float64 th = ui_snap(g_ui_m.toggle_h);
    float64 bw = button.size() != 0 ? ui_snap(ui_gfx_text_size(g_ui_font, button).x + 2.0 * pad) : 0.0;
    float64 right = button.size() != 0 ? x + w - bw - ui_snap(pad * 0.5) : x + w;
    float64 tx = right - bs;
    float64 sw = ui_snap(g_ui_m.swatch_w);
    float64 sx = tx - ui_snap(pad) - sw;
    float64 ty = ui_snap(y + (rh - th) * 0.5);
    // With a button the label is a list key, cut with a tooltip as in ui_text_button.
    float64 lw = ui_gfx_text_size(g_ui_font, label).x;
    if (button.size() != 0)
        lw = min(lw, g_ui_m.stub_w);
    ui_need(ui_icon_w() + lw + 2.0 * pad + sw + bs + (x + w - right));
    clicked = false;
    if (button.size() != 0) {
        float64 fh = ui_snap(g_ui_m.frame_h);
        clicked = ui_small_button(button, x + w - bw, ui_snap(y + (rh - fh) * 0.5), bw, fh, true);
    }
    // The swatch first, so it wins a press over the row.
    int32 sid = ui_id();
    bool shot = ui_hovered(sx - pad * 0.5, y, sw + pad, rh);
    if (ui_button_behavior(sid, shot))
        ui_picker_open(label, c);
    int32 id = ui_id();
    bool hot = ui_hovered(x, y, right - x, rh) && !shot;
    bool changed = ui_button_behavior(id, hot);
    if (changed)
        v = !v;
    changed = ui_picker_bind(label, c, sx, ty, sw, th) || changed;
    ui_label(label, x, y, rh, sx - pad - x);
    ui_draw_swatch(sx, ty, sw, th, c, ui_lit(shot, sid), ui_picker_is_open(label));
    ui_draw_check(tx, ui_snap(y + (rh - bs) * 0.5), bs, v, ui_lit(hot, id));
    return changed;
}

inline bool ui_checkbox_color(const std::string& label, bool& v, color& c) {
    bool clicked = false;
    return ui_checkbox_color_button(label, v, c, "", clicked);
}
