inline void ui_feat_add(const ui_feat_t& f) {
    ui_feat_t n = f;
    if (n.risk < UI_FEAT_READ || n.risk > UI_FEAT_SERVER_RISKY)
        n.risk = UI_FEAT_READ;
    for (uint64 i = 0; i < g_ui_feats.list.size(); i++) {
        if (g_ui_feats.list[i].id == n.id) {
            g_ui_feats.list[i] = n;
            return;
        }
    }
    g_ui_feats.list.push_back(n);
}

inline ui_feat_t ui_feat_make(int32 id, const std::string& name, int32 risk, const std::string& what) {
    ui_feat_t f;
    f.id = id;
    f.name = name;
    f.risk = risk;
    f.what = what;
    return f;
}

inline void ui_feat_add_toggle(int32 id, const std::string& name, int32 risk, const std::string& what, bool* enabled_p, UiFeatStatusFn status_fn) {
    ui_feat_t f = ui_feat_make(id, name, risk, what);
    f.enabled_p = enabled_p;
    f.status_fn = status_fn;
    ui_feat_add(f);
}

inline void ui_feat_add_keyed(int32 id, const std::string& name, int32 risk, const std::string& what, bool* enabled_p, ui_keybind_t* bind_p, UiFeatStatusFn status_fn) {
    ui_feat_t f = ui_feat_make(id, name, risk, what);
    f.enabled_p = enabled_p;
    f.bind_p = bind_p;
    f.status_fn = status_fn;
    ui_feat_add(f);
}

inline void ui_feat_add_computed(int32 id, const std::string& name, int32 risk, const std::string& what, UiFeatEnabledFn enabled_fn, ui_keybind_t* bind_p, UiFeatStatusFn status_fn) {
    ui_feat_t f = ui_feat_make(id, name, risk, what);
    f.enabled_fn = enabled_fn;
    f.bind_p = bind_p;
    f.status_fn = status_fn;
    ui_feat_add(f);
}

inline void ui_feat_add_with_active(int32 id, const std::string& name, int32 risk, const std::string& what, bool* enabled_p, ui_keybind_t* bind_p, UiFeatActiveFn active_fn, UiFeatStatusFn status_fn) {
    ui_feat_t f = ui_feat_make(id, name, risk, what);
    f.enabled_p = enabled_p;
    f.bind_p = bind_p;
    f.active_fn = active_fn;
    f.status_fn = status_fn;
    ui_feat_add(f);
}

inline int32 ui_feat_count() {
    return static_cast<int32>(g_ui_feats.list.size());
}

inline ui_feat_t* ui_feat_at(int32 index) {
    if (index < 0 || static_cast<uint64>(index) >= g_ui_feats.list.size())
        return nullptr;
    return &g_ui_feats.list[static_cast<uint64>(index)];
}

inline ui_feat_t* ui_feat_get(int32 id) {
    for (uint64 i = 0; i < g_ui_feats.list.size(); i++) {
        if (g_ui_feats.list[i].id == id)
            return &g_ui_feats.list[i];
    }
    return nullptr;
}

inline void ui_feat_set_hud_name(int32 id, const std::string& name) {
    ui_feat_t* f = ui_feat_get(id);
    if (f != nullptr)
        f->hud_name = name;
}

inline void ui_feat_set_write_note(const std::string& note) {
    g_ui_feats.write_note = note;
}

inline int32 ui_feat_risk(int32 id) {
    ui_feat_t* f = ui_feat_get(id);
    return f != nullptr ? f->risk : UI_FEAT_READ;
}

inline bool ui_feat_enabled(const ui_feat_t& f) {
    if (f.enabled_fn != nullptr)
        return f.enabled_fn();
    if (f.enabled_p != nullptr)
        return *f.enabled_p;
    return true;
}

inline bool ui_kb_lit(const ui_keybind_t& kb, bool enabled) {
    if (!enabled)
        return false;
    if (kb.mode == UI_KB_TOGGLE)
        return kb.on;
    if (kb.mode == UI_KB_HOLD)
        return (kb.bits & UI_KEY_DOWN) != 0;
    return true;
}

inline bool ui_kb_held(const ui_keybind_t& kb) {
    uint8 bits = 0;
    return kb.vk != 0 && ui_in_key_bits(kb.vk, bits) && (bits & UI_KEY_DOWN) != 0;
}

inline bool ui_kb_on(const ui_keybind_t& kb) {
    return kb.mode == UI_KB_ALWAYS_ON || (kb.mode == UI_KB_TOGGLE && kb.on)
        || (kb.mode == UI_KB_HOLD && g_kb_live && (kb.bits & UI_KEY_DOWN) != 0);
}

inline bool ui_feat_active(const ui_feat_t& f) {
    if (f.active_fn != nullptr)
        return f.active_fn();
    return f.bind_p == nullptr || ui_kb_on(*f.bind_p);
}

inline int32 ui_feat_state(const ui_feat_t& f, bool attached, bool writes) {
    if (!ui_feat_enabled(f))
        return UI_FEAT_OFF;
    if (!ui_feat_active(f) || !attached || (f.risk != UI_FEAT_READ && !writes))
        return UI_FEAT_ARMED;
    return UI_FEAT_ACTIVE;
}

inline void ui_tip_status(const std::string& tip, const std::string& status) {
    ui_tip(status.size() != 0 ? tip + "\n\nStatus: " + status : tip);
}

inline void ui_feat_risk_badge(int32 risk) {
    if (risk == UI_FEAT_READ)
        return;
    color c = risk == UI_FEAT_SERVER_RISKY ? g_ui_theme.deny : risk == UI_FEAT_SERVER ? g_ui_theme.warn : g_ui_theme.dim;
    ui_badge(UI_FEAT_RISK_TAGS[static_cast<uint64>(risk)], c);
}

inline void ui_feat_tip(int32 id, const std::string& tip, const std::string& status) {
    ui_feat_t* f = ui_feat_get(id);
    if (f == nullptr) {
        ui_tip_status(tip, status);
        return;
    }
    ui_feat_risk_badge(f->risk);
    uint64 r = static_cast<uint64>(f->risk);
    std::string note = f->risk == UI_FEAT_READ ? std::string("") : g_ui_feats.write_note;
    ui_tip_status(UI_FEAT_PREFIXES[r] + " " + UI_FEAT_RISK_TAGS[r] + ": " + f->what + ".\n\n" + tip + note, status);
}

inline bool ui_check_key(const std::string& label, bool& enabled, ui_keybind_t& kb, bool modes) {
    kb.active = ui_kb_lit(kb, enabled);
    return ui_checkbox_key(label, enabled, kb, modes);
}

inline bool ui_label_key_switch(const std::string& label, ui_keybind_t& kb, bool modes, bool clearable, bool switch_on) {
    kb.active = modes ? ui_kb_lit(kb, switch_on) : ui_kb_held(kb);
    return ui_label_bind(label, kb, modes, clearable);
}

inline bool ui_label_key(const std::string& label, ui_keybind_t& kb, bool modes, bool clearable) {
    return ui_label_key_switch(label, kb, modes, clearable, true);
}

inline bool ui_label_key_lit(const std::string& label, ui_keybind_t& kb, bool clearable, bool lit) {
    kb.active = lit;
    return ui_label_bind(label, kb, false, clearable);
}

inline bool ui_feat_row_as(int32 id, const std::string& label, const std::string& tip, const std::string& status) {
    ui_feat_t* f = ui_feat_get(id);
    if (f == nullptr || f->enabled_p == nullptr)
        return false;
    bool changed = f->bind_p != nullptr ? ui_check_key(label, *f->enabled_p, *f->bind_p, true)
        : ui_toggle(label, *f->enabled_p);
    ui_feat_tip(id, tip, *f->enabled_p ? status : std::string(""));
    return changed;
}

inline bool ui_feat_row(int32 id, const std::string& tip, const std::string& status) {
    ui_feat_t* f = ui_feat_get(id);
    return f != nullptr && ui_feat_row_as(id, f->name, tip, status);
}

inline void ui_menu_set_reserved_keys(const std::vector<int64>& keys) {
    g_ui_feats.reserved = keys;
}

inline void ui_menu_add_bind(ui_keybind_t* kb) {
    if (kb != nullptr)
        g_ui_feats.extra_binds.push_back(kb);
}

inline std::vector<int64> ui_menu_bind_keys() {
    std::vector<int64> ks;
    for (const ui_feat_t& f : g_ui_feats.list) {
        if (f.bind_p != nullptr)
            ks.push_back(f.bind_p->vk);
    }
    for (ui_keybind_t* kb : g_ui_feats.extra_binds)
        ks.push_back(kb->vk);
    return ks;
}

inline bool ui_menu_key_reserved(int64 vk) {
    for (int64 k : g_ui_feats.reserved) {
        if (k == vk)
            return true;
    }
    return false;
}

inline bool ui_menu_key_clashes(int64 vk) {
    for (int64 k : ui_menu_bind_keys()) {
        if (k == vk)
            return true;
    }
    return false;
}

inline bool ui_feat_safe_start() {
    return g_ui_feats.safe_start;
}

inline void ui_feat_set_safe_start(bool on) {
    g_ui_feats.safe_start = on;
}

inline int32 ui_feat_safe_off() {
    return g_ui_feats.safe_off;
}

inline int32 ui_feat_apply_safe_start() {
    int32 n = 0;
    for (ui_feat_t& f : g_ui_feats.list) {
        if (f.risk == UI_FEAT_SERVER_RISKY && f.enabled_p != nullptr && *f.enabled_p) {
            *f.enabled_p = false;
            n++;
        }
    }
    return n;
}

inline void ui_feat_safe_start_row(const std::string& label) {
    ui_toggle(label, g_ui_feats.safe_start);
    ui_tip("Loading the config leaves every feature tagged RISKY (server-visible beyond normal play) switched off, "
        "Always on keys included, so each is turned on by hand. Its other settings and its key still load."
        + (g_ui_feats.safe_off > 0 ? std::format("\n\nThe last load turned off {}.", g_ui_feats.safe_off)
        : std::string("")));
}
