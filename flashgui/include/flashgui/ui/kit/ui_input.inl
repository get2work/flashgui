inline bool ui_editing() {
    return g_ui_edit.label.size() != 0;
}

inline std::string ui_edit_label() {
    return g_ui_edit.label;
}

inline void ui_edit_end() {
    g_ui_edit.label = "";
    g_ui_cap.repeat_vk = -1;
    g_ui_edit.commit = false;
    g_ui_edit.cancel = false;
}

inline void ui_edit_start(const std::string& label, const std::string& text) {
    ui_capture_cancel();
    g_ui_edit.label = label;
    g_ui_edit.text = text;
    g_ui_edit.fresh = true;
    g_ui_edit.free = false;
    g_ui_edit.enter = false;
    g_ui_edit.commit = false;
    g_ui_edit.cancel = false;
    g_ui_edit.seen = true;
    g_ui_edit.started_at = ui_sys_ms();
    g_ui_cap.repeat_vk = -1;
    for (uint64 i = 0; i < g_ui_cap.bits_seen.size(); i++)
        g_ui_cap.bits_seen[i] = false;
    ui_capture_read(true);
}

inline bool ui_shift_down() {
    for (int32 vk : UI_VK_SHIFTS) {
        if (static_cast<uint64>(vk) < g_ui_cap.down.size() && g_ui_cap.down[static_cast<uint64>(vk)])
            return true;
    }
    return false;
}

inline std::string ui_text_char(int64 k, bool shift) {
    if (k >= vk::a && k <= vk::z)
        return std::string(1, static_cast<char>((shift ? UI_CHAR_UPPER_A : UI_CHAR_LOWER_A) + (k - vk::a)));
    if (k >= vk::k0 && k <= vk::k9)
        return std::string(1, static_cast<char>(UI_CHAR_0 + (k - vk::k0)));
    if (k >= vk::numpad0 && k <= vk::numpad9)
        return std::string(1, static_cast<char>(UI_CHAR_0 + (k - vk::numpad0)));
    if (k == vk::space)
        return " ";
    if (k == UI_VK_OEM_MINUS || k == vk::subtract)
        return shift && k == UI_VK_OEM_MINUS ? "_" : "-";
    if (k == UI_VK_OEM_PERIOD || k == vk::decimal)
        return ".";
    if (k == UI_VK_OEM_COMMA)
        return ",";
    if (k == UI_VK_OEM_QUOTE)
        return "'";
    return "";
}

inline void ui_edit_key(int64 k) {
    if (k == vk::escape || k == vk::enter) {
        if (k == vk::escape)
            g_ui_edit.cancel = true;
        else
            g_ui_edit.commit = true;
        g_ui_edit.enter = k == vk::enter;
        g_ui_cap.mute_vk = k;
        return;
    }
    if (k == vk::backspace) {
        uint64 n = g_ui_edit.text.size();
        g_ui_edit.text = g_ui_edit.fresh || n == 0 ? "" : g_ui_edit.text.substr(0, n - 1);
        g_ui_edit.fresh = false;
        return;
    }
    if (g_ui_edit.free) {
        std::string t = ui_text_char(k, ui_shift_down());
        g_ui_edit.fresh = false;
        if (t.size() != 0 && g_ui_edit.text.size() < g_ui_edit.max_chars)
            g_ui_edit.text = g_ui_edit.text + t;
        return;
    }
    std::string c = "";
    if (k >= vk::k0 && k <= vk::k9)
        c = std::string(1, static_cast<char>(UI_CHAR_0 + (k - vk::k0)));
    else if (k >= vk::numpad0 && k <= vk::numpad9)
        c = std::string(1, static_cast<char>(UI_CHAR_0 + (k - vk::numpad0)));
    else if (k == UI_VK_OEM_PERIOD || k == vk::decimal)
        c = ".";
    else if (k == UI_VK_OEM_MINUS || k == vk::subtract)
        c = "-";
    if (c.size() == 0)
        return;
    if (g_ui_edit.fresh)
        g_ui_edit.text = "";
    g_ui_edit.fresh = false;
    if ((c == "-" && g_ui_edit.text.size() != 0) || (c == "." && g_ui_edit.text.find(".") != std::string::npos))
        return;
    if (g_ui_edit.text.size() < UI_EDIT_MAX_CHARS)
        g_ui_edit.text = g_ui_edit.text + c;
}

inline bool ui_key_repeats(int64 k) {
    return k == vk::backspace || ui_text_char(k, false).size() != 0;
}

inline void ui_edit_repeat(int64 now) {
    int64 k = g_ui_cap.repeat_vk;
    if (k < 0)
        return;
    uint64 i = static_cast<uint64>(k);
    // A failed kernel read leaves the last bits in down; only the API state is current then.
    if (i >= g_ui_cap.down.size() || !g_ui_cap.down[i] || (!g_ui_cap.bits_seen[i] && !g_ui_cap.api[i])) {
        g_ui_cap.repeat_vk = -1;
        return;
    }
    if (g_ui_edit.commit || g_ui_edit.cancel || now < g_ui_cap.repeat_at)
        return;
    ui_edit_key(k);
    // A slow frame fires once, never a burst.
    g_ui_cap.repeat_at = g_ui_cap.repeat_at + UI_REPEAT_MS;
    if (g_ui_cap.repeat_at <= now)
        g_ui_cap.repeat_at = now + UI_REPEAT_MS;
}

inline bool ui_edit_apply(const std::string& label, float64& v, float64 lo, float64 hi, float64 step) {
    if (g_ui_edit.label != label)
        return false;
    g_ui_edit.seen = true;
    if (g_ui_edit.cancel) {
        ui_edit_end();
        return false;
    }
    if (!g_ui_edit.commit)
        return false;
    float64 nv = v;
    bool ok = ui_sys_parse_float(g_ui_edit.text, nv);
    ui_edit_end();
    if (!ok)
        return false;
    nv = ui_on_grid(nv, lo, hi, step);
    bool changed = nv != v;
    v = nv;
    return changed;
}

inline bool ui_value_field(const std::string& label, float64& v, float64 lo, float64 hi, float64 step, float64 x, float64 y, float64 w, float64 h, bool framed) {
    bool changed = ui_edit_apply(label, v, lo, hi, step);
    int32 id = ui_id();
    bool hot = ui_hovered(x, y, w, h);
    if (ui_button_behavior(id, hot) && !ui_editing())
        ui_edit_start(label, ui_format_value(v, step));
    bool editing = g_ui_edit.label == label;
    bool lit = ui_lit(hot, id);
    if (editing) {
        g_ui_edit.x = x;
        g_ui_edit.y = y;
        g_ui_edit.w = w;
        g_ui_edit.h = h;
        ui_draw_box(x, y, w, h, g_ui_theme.pressed, ui_col_accent());
    } else if (framed) {
        ui_draw_field(x, y, w, h, lit, false);
    } else if (lit) {
        ui_gfx_fill(vec2(x, y + 1.0), vec2(w, h - 2.0), g_ui_theme.hover, 0.0);
    }
    if (!editing) {
        ui_text_center(ui_format_value(v, step), x, y, w, h, g_ui_theme.text, g_ui_font);
        return changed;
    }
    std::string text = ui_fit_text(g_ui_edit.text, w - g_ui_m.pad);
    float64 tw = ui_gfx_text_size(g_ui_font, text).x;
    float64 tx = ui_snap(x + (w - tw) * 0.5);
    float64 in = g_ui_style.caret_inset;
    if (g_ui_edit.fresh && tw > 0.0)
        ui_gfx_fill(vec2(tx - 1.0, y + in), vec2(tw + 2.0, h - 2.0 * in), g_ui_theme.border_hi, 0.0);
    ui_gfx_text(text, vec2(tx, y + (h - g_ui_font_h) * 0.5), g_ui_theme.text, g_ui_font);
    ui_draw_caret(tx + tw + 1.0, y, h);
    return changed;
}

inline void ui_draw_caret(float64 x, float64 y, float64 h) {
    if (((ui_sys_ms() - g_ui_edit.started_at) / UI_CARET_MS) % 2 != 0)
        return;
    float64 in = g_ui_style.caret_inset;
    ui_gfx_fill(vec2(x, y + in), vec2(1.0, h - 2.0 * in), g_ui_theme.text, 0.0);
}

inline void ui_text_edit_start(const std::string& id, const std::string& text, uint64 max_chars) {
    ui_edit_start(id, text);
    g_ui_edit.free = true;
    g_ui_edit.fresh = false;
    g_ui_edit.max_chars = max_chars;
}

inline bool ui_text_input(const std::string& id, std::string& text, const std::string& hint, float64 x, float64 y, float64 w, float64 h) {
    return ui_text_input_max(id, text, hint, x, y, w, h, UI_TEXT_MAX_CHARS);
}

inline bool ui_text_input_max(const std::string& id, std::string& text, const std::string& hint, float64 x, float64 y, float64 w, float64 h, uint64 max_chars) {
    bool submit = false;
    if (g_ui_edit.label == id) {
        g_ui_edit.seen = true;
        if (g_ui_edit.cancel) {
            ui_edit_end();
        } else if (g_ui_edit.commit) {
            text = g_ui_edit.text;
            submit = g_ui_edit.enter;
            ui_edit_end();
        }
    }
    int32 wid = ui_id();
    bool hot = ui_hovered(x, y, w, h);
    if (ui_button_behavior(wid, hot) && !ui_editing())
        ui_text_edit_start(id, text, max_chars);
    bool editing = g_ui_edit.label == id;
    float64 pad = ui_snap(g_ui_m.pad * 0.5);
    float64 avail = max(w - 2.0 * pad - g_ui_style.caret_room, 1.0);
    float64 ty = y + (h - g_ui_font_h) * 0.5;
    if (!editing) {
        ui_draw_field(x, y, w, h, ui_lit(hot, wid), false);
        bool empty = text.size() == 0;
        color tc = empty ? g_ui_theme.muted : g_ui_theme.text;
        ui_gfx_text(ui_fit_text(empty ? hint : text, avail), vec2(x + pad, ty), tc, g_ui_font);
        return submit;
    }
    g_ui_edit.x = x;
    g_ui_edit.y = y;
    g_ui_edit.w = w;
    g_ui_edit.h = h;
    ui_draw_box(x, y, w, h, g_ui_theme.pressed, ui_col_accent());
    // Cut from the front, so the end with the caret stays in view.
    std::string shown = g_ui_edit.text;
    if (ui_gfx_text_size(g_ui_font, shown).x > avail) {
        float64 room = avail - ui_gfx_text_size(g_ui_font, "..").x;
        while (shown.size() != 0 && ui_gfx_text_size(g_ui_font, shown).x > room)
            shown = shown.substr(1);
        shown = ".." + shown;
    }
    float64 tw = ui_gfx_text_size(g_ui_font, shown).x;
    ui_gfx_text(shown, vec2(x + pad, ty), g_ui_theme.text, g_ui_font);
    ui_draw_caret(ui_snap(x + pad + tw + 1.0), y, h);
    return submit;
}

inline bool ui_toggle_text(const std::string& label, bool& v, std::string& text, const std::string& hint, uint64 max_chars) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 pad = g_ui_m.pad;
    float64 bs = ui_snap(g_ui_m.box);
    float64 bx = x + w - bs;
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 lw = ui_gfx_text_size(g_ui_font, label).x;
    float64 iw = ui_snap(ui_gfx_text_size(g_ui_font, std::string(max_chars, '0')).x + 2.0 * ui_snap(pad * 0.5)
        + g_ui_style.caret_room);
    float64 ir = bx - ui_snap(pad * g_ui_style.key_gap);
    float64 ix = ui_snap(max(ir - iw, x + lw + pad));
    ui_need(lw + pad + iw + pad * g_ui_style.key_gap + bs);
    int32 id = ui_id();
    float64 hit = pad * g_ui_style.check_hit;
    bool hot = ui_hovered(x, y, ix - pad * 0.5 - x, rh) || ui_hovered(bx - hit, y, bs + hit, rh);
    bool changed = ui_button_behavior(id, hot);
    if (changed)
        v = !v;
    ui_row_hover(x, y, w, rh, ui_lit(hot, id));
    ui_label(label, x, y, rh, ix - pad - x);
    ui_draw_check(bx, ui_snap(y + (rh - bs) * 0.5), bs, v, ui_lit(hot, id));
    std::string was = text;
    ui_text_input_max(label, text, hint, ix, ui_snap(y + (rh - fh) * 0.5), max(ir - ix, fh), fh, max_chars);
    return changed || text != was;
}

inline int32 ui_input_row(const std::string& id, std::string& text, const std::string& hint, const std::vector<std::string>& buttons) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 pad = ui_snap(g_ui_m.pad);
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (rh - fh) * 0.5);
    std::vector<float64> bws;
    float64 all = 0.0;
    for (const std::string& b : buttons) {
        float64 bw = ui_snap(ui_gfx_text_size(g_ui_font, b).x + 2.0 * pad);
        bws.push_back(bw);
        all = all + bw + ui_snap(pad * 0.5);
    }
    float64 iw = max(w - all, fh);
    int32 hit = ui_text_input(id, text, hint, x, fy, iw, fh) ? 0 : -1;
    float64 bx = x + iw;
    for (uint64 i = 0; i < buttons.size(); i++) {
        bx = bx + ui_snap(pad * 0.5);
        if (ui_small_button(buttons[i], bx, fy, bws[i], fh, true) && hit < 0)
            hit = static_cast<int32>(i);
        bx = bx + bws[i];
    }
    ui_need(g_ui_m.stub_w + all);
    ui_row_mark(x, y, rh, x, x);
    return hit;
}

inline std::string ui_search_row(const std::string& id, std::string& text) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 gap = ui_snap(g_ui_m.pad * 0.5);
    float64 fh = ui_snap(g_ui_m.frame_h);
    float64 fy = ui_snap(y + (rh - fh) * 0.5);
    float64 iw = max(w - fh - gap, fh);
    ui_text_input(id, text, "search", x, fy, iw, fh);
    bool typing = g_ui_edit.label == id && !g_ui_edit.cancel;
    std::string query = typing ? g_ui_edit.text : text;
    if (ui_small_button(UI_SEARCH_CLEAR, x + iw + gap, fy, fh, fh, query.size() != 0)) {
        text = "";
        query = "";
        if (g_ui_edit.label == id)
            ui_edit_end();
    }
    ui_need(g_ui_m.stub_w + fh + gap);
    ui_row_mark(x, y, rh, x, x);
    return query;
}

inline std::string ui_key_name(int64 vk) {
    if (vk == 0)
        return "none";
    if (vk == vk::lbutton)
        return "Mouse1";
    if (vk == vk::rbutton)
        return "Mouse2";
    if (vk == vk::mbutton)
        return "Middle";
    if (vk == vk::xbutton1)
        return "Mouse4";
    if (vk == vk::xbutton2)
        return "Mouse5";
    std::string n = ui_in_key_name(vk);
    return n.size() != 0 ? n : std::format("VK 0x{:x}", vk);
}

inline bool ui_capturing() {
    return g_ui_cap.label.size() != 0;
}

inline std::string ui_capture_label() {
    return g_ui_cap.label;
}

inline bool ui_keys_muted() {
    return ui_capturing() || ui_editing() || g_ui_cap.mute_vk >= 0;
}

inline void ui_keys_mute_tick() {
    if (g_ui_cap.mute_vk < 0)
        return;
    uint8 bits = 0;
    bool down = ui_in_key_bits(g_ui_cap.mute_vk, bits) && (bits & UI_KEY_DOWN) != 0;
    if (!down && !ui_in_key_down(g_ui_cap.mute_vk))
        g_ui_cap.mute_vk = -1;
}

inline int64 ui_capture_read(bool snapshot) {
    uint64 n = static_cast<uint64>(UI_VK_COUNT);
    if (g_ui_cap.bits.size() < n) {
        g_ui_cap.bits.resize(n);
        g_ui_cap.bits_seen.resize(n);
        g_ui_cap.down.resize(n);
        g_ui_cap.api.resize(n);
    }
    for (uint64 i = 0; i < n; i++)
        g_ui_cap.api[i] = false;
    g_ui_cap.hits.clear();
    for (int32 k : ui_in_keys_down()) {
        if (k > 0 && k < UI_VK_COUNT)
            g_ui_cap.api[k] = true;
    }
    int64 hit = 0;
    for (int32 vk = 1; vk < UI_VK_COUNT; vk++) {
        bool down = false;
        bool press = false;
        uint8 bits = 0;
        if (ui_in_key_bits(vk, bits)) {
            down = (bits & UI_KEY_DOWN) != 0;
            // The toggle bit flips on every press, so a tap between two polls counts too.
            press = g_ui_cap.bits_seen[vk] && (down || (bits & UI_KEY_TOGGLE) != (g_ui_cap.bits[vk] & UI_KEY_TOGGLE));
            g_ui_cap.bits[vk] = bits;
            g_ui_cap.bits_seen[vk] = true;
        } else {
            down = (g_ui_cap.bits[vk] & UI_KEY_DOWN) != 0;
            g_ui_cap.bits_seen[vk] = false;
        }
        press = !snapshot && !g_ui_cap.down[vk] && (press || g_ui_cap.api[vk]);
        g_ui_cap.down[vk] = down || g_ui_cap.api[vk];
        if (press && vk != vk::lbutton) {
            if (hit == 0)
                hit = static_cast<int64>(vk);
            g_ui_cap.hits.push_back(static_cast<int64>(vk));
        }
    }
    return hit;
}

inline void ui_capture_start(const std::string& label) {
    ui_edit_end();
    g_ui_cap.label = label;
    g_ui_cap.started_at = ui_sys_ms();
    g_ui_cap.key = 0;
    g_ui_cap.flash = "";
    for (uint64 i = 0; i < g_ui_cap.bits_seen.size(); i++)
        g_ui_cap.bits_seen[i] = false;
    ui_capture_read(true);
}

inline void ui_capture_cancel() {
    g_ui_cap.label = "";
    g_ui_cap.key = 0;
}

inline void ui_capture_poll(bool live) {
    ui_keys_mute_tick();
    if (ui_editing()) {
        if (!live) {
            ui_edit_end();
            return;
        }
        ui_capture_read(false);
        int64 now = ui_sys_ms();
        for (int64 k : g_ui_cap.hits) {
            if (!g_ui_edit.commit && !g_ui_edit.cancel)
                ui_edit_key(k);
            if (ui_key_repeats(k)) {
                g_ui_cap.repeat_vk = k;
                g_ui_cap.repeat_at = now + UI_REPEAT_DELAY_MS;
            }
        }
        ui_edit_repeat(now);
        return;
    }
    if (!ui_capturing())
        return;
    int64 k = ui_capture_read(false);
    if (live && g_ui_cap.key == 0)
        g_ui_cap.key = k;
}

inline ui_keybind_t ui_kb_make(int64 vk, int32 mode) {
    ui_keybind_t kb;
    kb.vk = vk;
    kb.mode = mode;
    return kb;
}

inline bool kb_poll(ui_keybind_t& kb) {
    uint8 bits = 0;
    if (kb.vk == 0) {
        kb.bits = 0;
        kb.seen = false;
        return false;
    }
    if (!ui_in_key_bits(kb.vk, bits)) {
        kb.seen = false;
        // Kept through a transient failure so KB_HOLD does not drop for one frame; cleared once the reader is gone.
        if (!ui_in_key_reader())
            kb.bits = 0;
        return false;
    }
    bool down = (bits & UI_KEY_DOWN) != 0;
    bool was = (kb.bits & UI_KEY_DOWN) != 0;
    bool press = kb.seen && !kb.swallow && !ui_keys_muted()
        && ((down && !was) || ((bits & UI_KEY_TOGGLE) != (kb.bits & UI_KEY_TOGGLE) && !down && !was));
    kb.bits = bits;
    kb.seen = true;
    if (!down)
        kb.swallow = false;
    return press;
}

inline bool kb_held(const ui_keybind_t& kb) {
    return g_kb_live && (kb.bits & UI_KEY_DOWN) != 0 && !kb.swallow;
}

inline bool kb_active(ui_keybind_t& kb, bool enabled) {
    bool press = kb_poll(kb);
    if (!enabled) {
        kb.on = false;
        kb.active = false;
        return false;
    }
    if (kb.mode == UI_KB_TOGGLE && press && g_kb_live)
        kb.on = !kb.on;
    kb.active = kb.mode == UI_KB_ALWAYS_ON || (kb.mode == UI_KB_TOGGLE ? kb.on : kb_held(kb));
    return kb.active;
}

inline void ui_bind_flash(const std::string& label, const std::string& text) {
    g_ui_cap.flash_label = label;
    g_ui_cap.flash = text;
    g_ui_cap.flash_at = ui_sys_ms();
}

inline bool ui_bind_flashing(const std::string& label) {
    return g_ui_cap.flash_label == label && g_ui_cap.flash.size() != 0
        && ui_sys_ms() - g_ui_cap.flash_at < UI_BIND_FLASH_MS;
}

inline void kb_rebind(ui_keybind_t& kb, int64 vk, bool swallow) {
    kb.vk = vk;
    kb.on = false;
    kb.seen = false;
    kb.swallow = swallow;
}

inline bool ui_key_allowed(int64 k, int64 current) {
    if (k == vk::lbutton)
        return false;
    if (k == current)
        return true;
    for (int64 t : g_ui_keys_taken) {
        if (t == k)
            return false;
    }
    return true;
}

inline float64 ui_keybind_width(const std::string& label, const ui_keybind_t& kb, bool modes) {
    bool flash = ui_bind_flashing(label);
    if (g_ui_key_text.count(label) == 0) {
        ui_key_text_t fresh;
        g_ui_key_text[label] = fresh;
    }
    ui_key_text_t& kt = g_ui_key_text[label];
    if (kt.vk != kb.vk || kt.flash != flash || kt.font != g_ui_font) {
        std::string text = flash ? g_ui_cap.flash : ui_key_name(kb.vk);
        kt.vk = kb.vk;
        kt.flash = flash;
        kt.font = g_ui_font;
        kt.w = ui_gfx_text_size(g_ui_font, text).x;
        kt.text = text;
    }
    float64 tw = max(kt.w, g_ui_dots_w);
    return ui_snap(max(tw + 2.0 * g_ui_m.pad, g_ui_m.key_w));
}

inline bool ui_keybind(const std::string& label, ui_keybind_t& kb, float64 row_y, float64 right_x, bool modes, bool clearable) {
    int32 id = ui_id();
    int64 now = ui_sys_ms();
    bool flash = ui_bind_flashing(label);
    float64 bw = ui_keybind_width(label, kb, modes);
    float64 bh = ui_snap(g_ui_m.frame_h);
    float64 bx = ui_snap(right_x) - bw;
    float64 by = ui_snap(row_y + (g_ui_m.row_h - g_ui_m.frame_h) * 0.5);
    bool hot = ui_hovered(bx, by, bw, bh);
    bool changed = false;

    if (modes && hot && g_ui_rpressed && !ui_capturing()) {
        g_ui_rpressed = false;
        // ui_popup_input already closed it if it was this box's: a second right click closes.
        g_ui_pop.label = g_ui_pop.prev == label ? "" : label;
        g_ui_pop.press = -1;
    }
    if (g_ui_pop.label == label && g_ui_pop.pick >= 0) {
        changed = kb.mode != g_ui_pop.pick;
        kb.mode = g_ui_pop.pick;
        kb.on = false;
        g_ui_pop.label = "";
    }

    if (ui_button_behavior(id, hot)) {
        if (g_ui_cap.label == label) {
            ui_capture_cancel();
        } else {
            ui_capture_start(label);
        }
    } else if (g_ui_cap.label == label && g_ui_cap.key != 0) {
        int64 k = g_ui_cap.key;
        g_ui_cap.key = 0;
        if (k == vk::escape) {
            if (clearable) {
                changed = kb.vk != 0;
                kb_rebind(kb, 0, false);
            }
            g_ui_cap.label = "";
            g_ui_cap.mute_vk = k;
        } else if (!ui_key_allowed(k, kb.vk)) {
            ui_bind_flash(label, "key not allowed");
        } else {
            changed = k != kb.vk;
            kb_rebind(kb, k, true);
            g_ui_cap.label = "";
            g_ui_cap.mute_vk = k;
        }
    }

    bool capturing = g_ui_cap.label == label;
    if (capturing)
        g_ui_cap.seen = true;
    bool open = g_ui_pop.label == label;
    if (open)
        ui_popup_place(UI_KB_MODE_NAMES, g_ui_no_icons, false, g_ui_no_marks, kb.mode,
            ui_snap(ui_max_text_w(UI_KB_MODE_NAMES) + 2.0 * g_ui_m.pad + 2.0), bx + bw, by, by + bh);
    bool lit = open || ui_lit(hot, id);
    color border = capturing ? g_ui_theme.purple
        : (open || kb.active ? ui_col_accent() : (lit ? g_ui_theme.border_hi : g_ui_theme.border));
    ui_draw_box(bx, by, bw, bh, capturing ? g_ui_theme.pressed : (lit ? g_ui_theme.hover : g_ui_theme.bg2), border);
    if (capturing && !flash) {
        int64 n = ((now - g_ui_cap.started_at) / UI_DOTS_MS) % UI_DOTS_N + 1;
        std::string dots = std::string(static_cast<uint64>(n), '.');
        // Placed by the width of "...", so the dots grow in place instead of re-centring.
        ui_text_at(dots, bx + (bw - g_ui_dots_w) * 0.5, by, bh, g_ui_theme.purple, g_ui_font_bold);
    } else {
        // Measured by ui_keybind_width above; flash is the same this frame, so the cached text is the refusal or the key.
        color c = flash ? g_ui_theme.deny : (kb.vk == 0 ? g_ui_theme.muted : g_ui_theme.text);
        vec2 at = vec2(ui_snap(bx + (bw - g_ui_key_text[label].w) * 0.5), by + (bh - g_ui_font_h) * 0.5);
        ui_gfx_text(g_ui_key_text[label].text, at, c, g_ui_font);
    }
    return changed;
}

inline bool ui_checkbox_bind(const std::string& label, bool& enabled, ui_keybind_t& kb) {
    return ui_checkbox_key(label, enabled, kb, true);
}

inline bool ui_checkbox_key(const std::string& label, bool& enabled, ui_keybind_t& kb, bool modes) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 pad = g_ui_m.pad;
    float64 bs = ui_snap(g_ui_m.box);
    float64 bx = x + w - bs;
    float64 key_right = bx - ui_snap(pad * g_ui_style.key_gap);
    float64 kw = ui_keybind_width(label, kb, modes);
    float64 kx = key_right - kw;
    ui_need(ui_gfx_text_size(g_ui_font, label).x + pad + kw + pad * g_ui_style.key_gap + bs);
    int32 id = ui_id();
    float64 hit = pad * g_ui_style.check_hit;
    bool hot = ui_hovered(x, y, kx - pad * 0.5 - x, rh) || ui_hovered(bx - hit, y, bs + hit, rh);
    bool changed = ui_button_behavior(id, hot);
    if (changed)
        enabled = !enabled;
    ui_row_hover(x, y, w, rh, ui_lit(hot, id));
    ui_label(label, x, y, rh, kx - pad - x);
    ui_draw_check(bx, ui_snap(y + (rh - bs) * 0.5), bs, enabled, ui_lit(hot, id));
    float64 lx = g_ui_row.label_end;
    float64 limit = g_ui_row.limit;
    changed = ui_keybind(label, kb, y, key_right, modes, true) || changed;
    g_ui_row.label_end = lx;
    g_ui_row.limit = limit;
    return changed;
}

inline bool ui_checkbox_key_pair(const std::string& label, bool& enabled, ui_keybind_t& kb, bool modes, const std::string& tip, const std::string& label2, bool& v2, const std::string& tip2) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 rh = g_ui_m.row_h;
    float64 y = ui_row_begin(rh);
    float64 pad = g_ui_m.pad;
    float64 bs = ui_snap(g_ui_m.box);
    float64 l2w = ui_gfx_text_size(g_ui_font, label2).x;
    float64 w2 = ui_snap(l2w + pad + bs);
    float64 x2 = x + w - w2;
    float64 bx = x2 - ui_snap(pad * g_ui_style.pair_gap) - bs;
    float64 key_right = bx - ui_snap(pad * g_ui_style.key_gap);
    float64 kw = ui_keybind_width(label, kb, modes);
    float64 kx = key_right - kw;
    ui_need(ui_gfx_text_size(g_ui_font, label).x + pad + kw + pad * g_ui_style.key_gap + bs + pad * g_ui_style.pair_gap
        + l2w + pad + bs);
    int32 id = ui_id();
    float64 hit = pad * g_ui_style.check_hit;
    bool hot = ui_hovered(x, y, kx - pad * 0.5 - x, rh) || ui_hovered(bx - hit, y, bs + hit, rh);
    bool changed = ui_button_behavior(id, hot);
    if (changed)
        enabled = !enabled;
    ui_label(label, x, y, rh, kx - pad - x);
    ui_draw_check(bx, ui_snap(y + (rh - bs) * 0.5), bs, enabled, ui_lit(hot, id));
    float64 lx = g_ui_row.label_end;
    float64 limit = g_ui_row.limit;
    changed = ui_keybind(label, kb, y, key_right, modes, true) || changed;
    g_ui_row.label_end = lx;
    g_ui_row.limit = limit;
    ui_tip(tip);
    int32 id2 = ui_id();
    bool hot2 = ui_hovered(x2, y, w2, rh);
    if (ui_button_behavior(id2, hot2)) {
        v2 = !v2;
        changed = true;
    }
    ui_label(label2, x2, y, rh, w2 - pad - bs);
    ui_draw_check(x + w - bs, ui_snap(y + (rh - bs) * 0.5), bs, v2, ui_lit(hot2, id2));
    ui_tip(tip2);
    return changed;
}

inline bool ui_label_bind(const std::string& label, ui_keybind_t& kb, bool modes, bool clearable) {
    float64 x = ui_snap(ui_content_x());
    float64 w = ui_snap(ui_content_w());
    float64 y = ui_row_begin(g_ui_m.row_h);
    float64 kw = ui_keybind_width(label, kb, modes);
    ui_need(ui_gfx_text_size(g_ui_font, label).x + g_ui_m.pad + kw);
    ui_label(label, x, y, g_ui_m.row_h, w - kw - g_ui_m.pad);
    return ui_keybind(label, kb, y, x + w, modes, clearable);
}
