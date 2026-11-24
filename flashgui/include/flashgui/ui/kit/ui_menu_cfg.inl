inline std::string ui_kv_trim(const std::string& s) {
    uint64 a = 0;
    uint64 b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r'))
        a = a + 1;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r'))
        b = b - 1;
    return s.substr(a, b - a);
}

inline int64 ui_kv_index(const std::string& key) {
    for (uint64 i = 0; i < g_ui_kv.keys.size(); i++) {
        if (g_ui_kv.keys[i] == key)
            return static_cast<int64>(i);
    }
    return -1;
}

inline void ui_kv_set(const std::string& key, const std::string& value) {
    int64 i = ui_kv_index(key);
    if (i >= 0) {
        g_ui_kv.vals[static_cast<uint64>(i)] = value;
        return;
    }
    g_ui_kv.keys.push_back(key);
    g_ui_kv.vals.push_back(value);
}

inline std::string ui_kv_text() {
    std::string s = UI_CFG_HEADER;
    for (uint64 i = 0; i < g_ui_kv.keys.size(); i++)
        s += g_ui_kv.keys[i] + "=" + g_ui_kv.vals[i] + "\n";
    return s;
}

inline bool ui_kv_load(const std::string& path) {
    g_ui_kv.keys.clear();
    g_ui_kv.vals.clear();
    std::string text = ui_sys_file_exists(path) ? ui_sys_file_read(path) : std::string("");
    uint64 at = 0;
    while (at < text.size()) {
        uint64 nl = text.find("\n", at);
        if (nl == std::string::npos)
            nl = text.size();
        std::string line = ui_kv_trim(text.substr(at, nl - at));
        at = nl + 1;
        uint64 eq = line.find("=");
        if (line.size() == 0 || line[0] == '#' || eq == std::string::npos)
            continue;
        std::string key = ui_kv_trim(line.substr(0, eq));
        if (key.size() != 0)
            ui_kv_set(key, ui_kv_trim(line.substr(eq + 1)));
    }
    if (g_ui_kv.keys.size() == 0) {
        g_ui_kv.error = text.size() == 0 ? "no config file" : "no settings in " + path;
        return false;
    }
    g_ui_kv.error = "";
    return true;
}

inline bool ui_kv_save(const std::string& path) {
    if (!ui_sys_file_write(path, ui_kv_text())) {
        g_ui_kv.error = "cannot write " + path;
        return false;
    }
    g_ui_kv.error = "";
    return true;
}

inline void ui_cfg_set_io(const ui_cfg_io_t& io) {
    g_ui_cfgs.io = io;
    g_ui_cfgs.has_io = true;
}

inline void ui_cfg_clear() {
    if (!g_ui_cfgs.has_io) {
        g_ui_kv.keys.clear();
        g_ui_kv.vals.clear();
    } else if (g_ui_cfgs.io.clear_fn != nullptr) {
        g_ui_cfgs.io.clear_fn();
    }
}

inline void ui_cfg_set(const std::string& key, const std::string& value) {
    if (!g_ui_cfgs.has_io)
        ui_kv_set(key, value);
    else if (g_ui_cfgs.io.set_fn != nullptr)
        g_ui_cfgs.io.set_fn(key, value);
}

inline std::string ui_cfg_get(const std::string& key) {
    if (!g_ui_cfgs.has_io) {
        int64 i = ui_kv_index(key);
        return i >= 0 ? g_ui_kv.vals[static_cast<uint64>(i)] : std::string("");
    }
    return g_ui_cfgs.io.get_fn != nullptr ? g_ui_cfgs.io.get_fn(key) : std::string("");
}

inline std::string ui_cfg_text() {
    if (!g_ui_cfgs.has_io)
        return ui_kv_text();
    return g_ui_cfgs.io.text_fn != nullptr ? g_ui_cfgs.io.text_fn() : std::string("");
}

inline std::string ui_cfg_error() {
    if (!g_ui_cfgs.has_io)
        return g_ui_kv.error;
    return g_ui_cfgs.io.error_fn != nullptr ? g_ui_cfgs.io.error_fn() : std::string("");
}

inline int64 ui_cfg_count() {
    if (!g_ui_cfgs.has_io)
        return static_cast<int64>(g_ui_kv.keys.size());
    return g_ui_cfgs.io.count_fn != nullptr ? g_ui_cfgs.io.count_fn() : 0;
}

inline bool ui_cfg_file_save() {
    if (!g_ui_cfgs.has_io)
        return ui_kv_save(g_ui_cfgs.path);
    return g_ui_cfgs.io.save_fn != nullptr && g_ui_cfgs.io.save_fn(g_ui_cfgs.path);
}

inline bool ui_cfg_file_load() {
    if (!g_ui_cfgs.has_io)
        return ui_kv_load(g_ui_cfgs.path);
    return g_ui_cfgs.io.load_fn != nullptr && g_ui_cfgs.io.load_fn(g_ui_cfgs.path);
}

inline void ui_cfg_pairs(std::vector<std::string>& keys, std::vector<std::string>& vals) {
    keys.clear();
    vals.clear();
    std::string text = ui_cfg_text();
    uint64 at = 0;
    while (at < text.size()) {
        uint64 nl = text.find("\n", at);
        if (nl == std::string::npos)
            nl = text.size();
        uint64 eq = text.find("=", at);
        if (text[at] != '#' && eq != std::string::npos && eq > at && eq < nl) {
            keys.push_back(text.substr(at, eq - at));
            vals.push_back(text.substr(eq + 1, nl - eq - 1));
        }
        at = nl + 1;
    }
}

inline void ui_cfg_keep_order(const std::vector<std::string>& order) {
    std::vector<std::string> keys;
    std::vector<std::string> vals;
    ui_cfg_pairs(keys, vals);
    std::map<std::string, uint64> pos;
    for (uint64 i = 0; i < keys.size(); i++)
        pos[keys[i]] = i;
    std::vector<bool> used;
    used.assign(keys.size(), false);
    std::vector<uint64> idx;
    for (const std::string& k : order) {
        if (pos.count(k) == 0)
            continue;
        uint64 i = pos[k];
        if (!used[i]) {
            used[i] = true;
            idx.push_back(i);
        }
    }
    for (uint64 i = 0; i < keys.size(); i++) {
        if (!used[i])
            idx.push_back(i);
    }
    bool same = true;
    for (uint64 i = 0; i < idx.size() && same; i++)
        same = idx[i] == i;
    if (same)
        return;
    ui_cfg_clear();
    for (uint64 i : idx)
        ui_cfg_set(keys[i], vals[i]);
}

inline bool ui_parse_float(const std::string& s, float64& out) {
    uint64 i = 0;
    bool neg = false;
    if (s.size() != 0 && (s[0] == '-' || s[0] == '+')) {
        neg = s[0] == '-';
        i = 1;
    }
    float64 v = 0.0;
    bool digits = false;
    while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
        v = v * 10.0 + static_cast<float64>(s[i] - '0');
        digits = true;
        i = i + 1;
    }
    if (i < s.size() && s[i] == '.') {
        i = i + 1;
        float64 frac = 0.0;
        float64 div = 1.0;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9') {
            frac = frac * 10.0 + static_cast<float64>(s[i] - '0');
            div = div * 10.0;
            digits = true;
            i = i + 1;
        }
        v = v + frac / div;
    }
    if (!digits || i != s.size())
        return false;
    out = neg ? -v : v;
    return true;
}

inline void ui_cfg_set_bool(const std::string& key, bool v) {
    ui_cfg_set(key, v ? "true" : "false");
}

inline void ui_cfg_set_int(const std::string& key, int64 v) {
    ui_cfg_set(key, std::format("{}", v));
}

inline void ui_cfg_set_float(const std::string& key, float64 v) {
    ui_cfg_set(key, std::format("{:.6f}", v));
}

inline bool ui_cfg_bool(const std::string& key, bool def) {
    std::string v = ui_cfg_get(key);
    if (v == "true" || v == "1")
        return true;
    if (v == "false" || v == "0")
        return false;
    return def;
}

inline int64 ui_cfg_int(const std::string& key, int64 def) {
    int64 v = 0;
    return ui_sys_parse_int(ui_cfg_get(key), v) ? v : def;
}

inline float64 ui_cfg_float(const std::string& key, float64 def) {
    float64 v = 0.0;
    return ui_parse_float(ui_cfg_get(key), v) ? v : def;
}

inline int64 ui_cfg_int_in(const std::string& key, int64 def, int64 lo, int64 hi) {
    int64 v = ui_cfg_int(key, def);
    return v >= lo && v <= hi ? v : def;
}

inline float64 ui_cfg_float_in(const std::string& key, float64 def, float64 lo, float64 hi) {
    float64 v = ui_cfg_float(key, def);
    return v >= lo && v <= hi ? v : def;
}

inline void ui_cfg_set_color(const std::string& key, const color& c) {
    ui_cfg_set_int(key + ".r", static_cast<int64>(c.r));
    ui_cfg_set_int(key + ".g", static_cast<int64>(c.g));
    ui_cfg_set_int(key + ".b", static_cast<int64>(c.b));
    ui_cfg_set_int(key + ".a", static_cast<int64>(c.a));
}

inline void ui_cfg_get_color(const std::string& key, color& c) {
    c.r = static_cast<uint8>(ui_cfg_int_in(key + ".r", static_cast<int64>(c.r), 0, 255));
    c.g = static_cast<uint8>(ui_cfg_int_in(key + ".g", static_cast<int64>(c.g), 0, 255));
    c.b = static_cast<uint8>(ui_cfg_int_in(key + ".b", static_cast<int64>(c.b), 0, 255));
    c.a = static_cast<uint8>(ui_cfg_int_in(key + ".a", static_cast<int64>(c.a), 0, 255));
}

inline void ui_cfg_set_bind(const std::string& name, const ui_keybind_t& kb) {
    ui_cfg_set_int("key." + name + ".vk", kb.vk);
    ui_cfg_set_int("key." + name + ".mode", static_cast<int64>(kb.mode));
}

inline void ui_cfg_get_bind(const std::string& name, ui_keybind_t& kb, int32 fixed_mode, bool clearable) {
    int64 vk = ui_cfg_int("key." + name + ".vk", kb.vk);
    if (vk < 0 || (vk == 0 && !clearable) || vk > UI_CFG_VK_MAX || vk == vk::lbutton)
        vk = kb.vk;
    int64 mode = ui_cfg_int("key." + name + ".mode", static_cast<int64>(kb.mode));
    if (mode < UI_KB_ALWAYS_ON || mode > UI_KB_HOLD)
        mode = kb.mode;
    if (fixed_mode >= 0)
        mode = fixed_mode;
    if (vk != kb.vk || mode != kb.mode)
        kb = ui_kb_make(vk, static_cast<int32>(mode));
}

inline void ui_cfg_store_feats(const std::string& prefix) {
    for (const ui_feat_t& f : g_ui_feats.list) {
        std::string k = std::format("{}.{}", prefix, f.id);
        if (f.enabled_p != nullptr)
            ui_cfg_set_bool(k, *f.enabled_p);
        if (f.bind_p != nullptr)
            ui_cfg_set_bind(k, *f.bind_p);
    }
}

inline void ui_cfg_apply_feats(const std::string& prefix) {
    for (ui_feat_t& f : g_ui_feats.list) {
        std::string k = std::format("{}.{}", prefix, f.id);
        if (f.enabled_p != nullptr)
            *f.enabled_p = ui_cfg_bool(k, *f.enabled_p);
        if (f.bind_p != nullptr)
            ui_cfg_get_bind(k, *f.bind_p, -1, true);
    }
}

inline void ui_cfg_store_menu() {
    ui_menu_t& m = g_ui_menu;
    ui_cfg_set_bool("watermark.enabled", m.watermark_on);
    ui_cfg_set_bool("watermark.placed", g_ui_wm.placed);
    ui_cfg_set_float("watermark.x", g_ui_wm.x);
    ui_cfg_set_float("watermark.y", g_ui_wm.y);
    ui_cfg_set_bool("hud.keys", m.keys_hud_on);
    ui_cfg_set_bool("hud.keys.placed", m.keys_rect.placed);
    ui_cfg_set_float("hud.keys.x", m.keys_rect.x);
    ui_cfg_set_float("hud.keys.y", m.keys_rect.y);
    ui_cfg_set_bool("game.safe_start", g_ui_feats.safe_start);
    ui_cfg_set_float("ui.scale", g_ui_scale);
    ui_cfg_set_color("ui.accent", g_ui_theme.accent);
    ui_cfg_set_float("window.x", g_ui_x);
    ui_cfg_set_float("window.y", g_ui_y);
    // Not an open view's temporary size.
    vec2 own = ui_menu_window_size();
    ui_cfg_set_float("window.w", own.x);
    ui_cfg_set_float("window.h", own.y);
    ui_cfg_set_bool("config.autosave", g_ui_cfgs.autosave);
    ui_cfg_set_bind("menu", m.key);
    ui_cfg_set_int("menu.tab", static_cast<int64>(m.tab));
    for (uint64 i = 0; i < m.tabs.size(); i++)
        ui_cfg_set_int(std::format("menu.page.{}", i), static_cast<int64>(m.tabs[i].page));
}

inline void ui_cfg_apply_menu() {
    ui_menu_t& m = g_ui_menu;
    m.watermark_on = ui_cfg_bool("watermark.enabled", m.watermark_on);
    g_ui_wm.placed = ui_cfg_bool("watermark.placed", g_ui_wm.placed);
    g_ui_wm.x = ui_cfg_float("watermark.x", g_ui_wm.x);
    g_ui_wm.y = ui_cfg_float("watermark.y", g_ui_wm.y);
    m.keys_hud_on = ui_cfg_bool("hud.keys", m.keys_hud_on);
    m.keys_rect.placed = ui_cfg_bool("hud.keys.placed", m.keys_rect.placed);
    m.keys_rect.x = ui_cfg_float("hud.keys.x", m.keys_rect.x);
    m.keys_rect.y = ui_cfg_float("hud.keys.y", m.keys_rect.y);
    g_ui_feats.safe_start = ui_cfg_bool("game.safe_start", g_ui_feats.safe_start);
    float64 lo = UI_SCALE_PRESETS[0];
    float64 hi = UI_SCALE_PRESETS[UI_SCALE_PRESETS.size() - 1];
    g_ui_scale = ui_scale_preset(ui_cfg_float_in("ui.scale", g_ui_scale, lo, hi));
    ui_cfg_get_color("ui.accent", g_ui_theme.accent);
    g_ui_x = ui_cfg_float("window.x", g_ui_x);
    g_ui_y = ui_cfg_float("window.y", g_ui_y);
    g_ui_w = max(ui_cfg_float("window.w", g_ui_w), ui_min_w_at(g_ui_scale));
    g_ui_h = max(ui_cfg_float("window.h", g_ui_h), ui_min_h_at(g_ui_scale));
    // The loaded size is the window's own; an open view grows it again at the next draw.
    m.grown = false;
    m.sized_for = UI_MENU_UNSIZED;
    g_ui_cfgs.autosave = ui_cfg_bool("config.autosave", g_ui_cfgs.autosave);
    ui_cfg_get_bind("menu", m.key, UI_KB_TOGGLE, false);
    int64 n = static_cast<int64>(m.tabs.size());
    m.tab = static_cast<int32>(ui_cfg_int_in("menu.tab", static_cast<int64>(m.tab), 0, n > 0 ? n - 1 : 0));
    for (uint64 i = 0; i < m.tabs.size(); i++) {
        ui_tab_t& t = m.tabs[i];
        int64 pages = t.pages.size() != 0 ? static_cast<int64>(t.pages.size()) : 1;
        t.page = static_cast<int32>(ui_cfg_int_in(std::format("menu.page.{}", i), static_cast<int64>(t.page), 0,
            pages - 1));
    }
}

inline void ui_cfg_setup(const std::string& path, UiCfgHookFn store_fn, UiCfgHookFn apply_fn) {
    g_ui_cfgs.path = path;
    g_ui_cfgs.store_fn = store_fn;
    g_ui_cfgs.apply_fn = apply_fn;
}

inline std::string ui_cfg_path() {
    return g_ui_cfgs.path;
}

inline bool ui_cfg_autosave() {
    return g_ui_cfgs.autosave;
}

inline void ui_cfg_set_status(const std::string& status) {
    g_ui_cfgs.status = status;
}

inline void ui_cfg_store() {
    std::vector<std::string> order;
    std::vector<std::string> vals;
    ui_cfg_pairs(order, vals);
    ui_cfg_clear();
    if (g_ui_cfgs.store_fn != nullptr)
        g_ui_cfgs.store_fn();
    ui_cfg_store_menu();
    ui_cfg_keep_order(order);
}

inline void ui_cfg_note_save(bool ok, const std::string& what) {
    ui_cfgs_t& c = g_ui_cfgs;
    c.saved_at = ui_sys_ms();
    if (ok) {
        c.last_save = ui_clock();
        c.status = "";
        c.failed = false;
        return;
    }
    // Printed once per failure run: auto-save retries every UI_CFG_SAVE_MS.
    if (!c.failed)
        println("[menu] " + what + " save failed: " + ui_cfg_error());
    c.status = "Save failed: " + ui_cfg_error();
    c.failed = true;
}

inline bool ui_cfg_save() {
    ui_cfg_store();
    bool ok = ui_cfg_file_save();
    ui_cfg_note_save(ok, "config");
    if (ok)
        g_ui_cfgs.text = ui_cfg_text();
    return ok;
}

inline void ui_cfg_save_if_changed() {
    ui_cfg_store();
    if (ui_cfg_text() != g_ui_cfgs.text)
        ui_cfg_save();
}

inline void ui_cfg_apply() {
    if (g_ui_cfgs.apply_fn != nullptr)
        g_ui_cfgs.apply_fn();
    ui_cfg_apply_menu();
    ui_sync_accent();
    g_ui_feats.safe_off = g_ui_feats.safe_start ? ui_feat_apply_safe_start() : 0;
    ui_menu_fix_key();
    ui_capture_cancel();
    ui_popup_close();
    ui_edit_end();
    ui_menu_reset_edits();
}

inline bool ui_cfg_load() {
    ui_cfgs_t& c = g_ui_cfgs;
    bool ok = ui_cfg_file_load();
    int64 n = ui_cfg_count();
    if (ok)
        ui_cfg_apply();
    ui_cfg_store();
    c.text = ui_cfg_text();
    c.failed = false;
    c.status = ok ? std::format("Loaded {} settings at {}", n, ui_clock())
        + (g_ui_feats.safe_off > 0 ? std::format(", {} risky features left off", g_ui_feats.safe_off) : std::string(""))
        : "Not loaded: " + ui_cfg_error();
    return ok;
}

inline void ui_cfg_tick(int64 now) {
    ui_cfgs_t& c = g_ui_cfgs;
    if (!c.autosave || now - c.checked_at < UI_CFG_CHECK_MS || now - c.saved_at < UI_CFG_SAVE_MS)
        return;
    c.checked_at = now;
    ui_cfg_save_if_changed();
}

inline void ui_cfg_rows() {
    ui_cfgs_t& c = g_ui_cfgs;
    if (ui_button("Save config"))
        ui_cfg_save();
    ui_tip("Last saved: " + c.last_save + ". File: " + c.path);
    if (ui_button("Load config"))
        ui_cfg_load();
    if (!c.failed && c.status.size() != 0)
        ui_tip(c.status);
    // Saved at once, or turning auto-save off would never reach the file.
    if (ui_toggle("Auto-save on change", c.autosave))
        ui_cfg_save();
    if (c.failed)
        ui_status_colored(c.status, g_ui_theme.deny);
}
