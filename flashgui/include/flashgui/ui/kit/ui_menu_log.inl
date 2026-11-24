inline std::string ui_clock_or(const std::string& fallback) {
    return ui_sys_clock_or(fallback);
}

inline std::string ui_clock() {
    return ui_sys_clock_or("--:--:--");
}

inline void ui_perf_setup(const std::string& log_path, const std::string& fault_path) {
    g_ui_perf.log_path = log_path;
    g_ui_perf.fault_path = fault_path;
    g_ui_perf.bytes = log_path.size() != 0 && ui_sys_file_write(log_path, "") ? 0 : UI_PERF_LOG_MAX;
}

inline int64 ui_perf_ms(int64 ticks) {
    int64 f = ui_sys_perf_freq();
    return f > 0 ? ticks * 1000 / f : 0;
}

inline int64 ui_perf_frames() {
    return g_ui_perf.frames;
}

inline std::string ui_perf_stage_name() {
    return g_ui_perf.stage;
}

inline void ui_perf_log(const std::string& line) {
    ui_perf_t& p = g_ui_perf;
    int64 now = ui_sys_ms();
    if (p.bytes >= UI_PERF_LOG_MAX)
        return;
    if (now - p.at >= UI_PERF_EVERY_MS) {
        p.at = now;
        p.n = 0;
    }
    if (p.n >= UI_PERF_BURST) {
        p.skipped = p.skipped + 1;
        return;
    }
    p.n = p.n + 1;
    std::string s = ui_clock() + "  " + line + (p.skipped != 0 ? std::format(" (+{} not logged)", p.skipped)
        : std::string("")) + "\n";
    p.skipped = 0;
    if (ui_sys_file_append(p.log_path, s))
        p.bytes = p.bytes + static_cast<int64>(s.size());
    else
        p.bytes = UI_PERF_LOG_MAX;
}

inline void ui_fault_log(const std::string& line) {
    if (g_ui_perf.fault_path.size() != 0 && !ui_sys_file_append(g_ui_perf.fault_path, line))
        println("[menu] cannot write " + g_ui_perf.fault_path);
}

inline void ui_perf_stage_end(int64 t) {
    ui_perf_t& p = g_ui_perf;
    if (p.stage_t != 0 && t - p.stage_t > p.slow_t) {
        p.slow_t = t - p.stage_t;
        p.slow_stage = p.stage;
    }
    p.stage_t = t;
}

inline void ui_perf_stage(const std::string& stage) {
    ui_perf_stage_end(ui_sys_perf_time());
    g_ui_perf.stage = stage;
}

inline void ui_perf_fault() {
    g_ui_perf.faulted = true;
    g_ui_perf.fault_streak = true;
    g_ui_perf.faults = g_ui_perf.faults + 1;
}

inline void ui_perf_frame_begin() {
    ui_perf_t& p = g_ui_perf;
    int64 t = ui_sys_perf_time();
    if (p.frame_end_t != 0 && ui_perf_ms(t - p.frame_end_t) >= UI_PERF_LATE_MS)
        ui_perf_log(std::format("frame {}: started {} ms after the last one ended", p.frames,
            ui_perf_ms(t - p.frame_end_t)));
    p.frame_t = t;
    p.stage_t = t;
    p.stage = "frame start";
    p.slow_t = 0;
    p.slow_stage = "";
    p.faulted = false;
}

inline void ui_perf_frame_end() {
    ui_perf_t& p = g_ui_perf;
    if (!p.faulted && p.fault_streak) {
        p.fault_streak = false;
        ui_fault_log(std::format("{} frame {}: clean again after {} caught exception(s)\n", ui_clock(), p.frames,
            p.faults));
        p.faults = 0;
    }
    int64 t = ui_sys_perf_time();
    ui_perf_stage_end(t);
    p.frame_end_t = t;
    int64 ms = ui_perf_ms(t - p.frame_t);
    if (ms >= UI_PERF_SLOW_MS)
        ui_perf_log(std::format("frame {}: {} ms, slowest stage '{}' {} ms", p.frames, ms, p.slow_stage,
            ui_perf_ms(p.slow_t)));
    p.frames = p.frames + 1;
}

inline void ui_watchdog_setup(UiHeartbeatFn heartbeat_fn, UiInputReleaseFn release_fn, UiStallFn stall_fn) {
    g_ui_watchdog.heartbeat_fn = heartbeat_fn;
    g_ui_watchdog.release_fn = release_fn;
    g_ui_watchdog.stall_fn = stall_fn;
}

inline void ui_watchdog_tick() {
    ui_watchdog_t& w = g_ui_watchdog;
    int64 now = ui_sys_ms();
    if (w.heartbeat_fn != nullptr && w.release_fn != nullptr) {
        int64 hb = w.heartbeat_fn();
        if (hb != w.seen_heartbeat || w.heartbeat_at == 0) {
            w.seen_heartbeat = hb;
            w.heartbeat_at = now;
            w.input_released = false;
        } else if (!w.input_released && now - w.heartbeat_at >= UI_WATCHDOG_INPUT_MS
            && w.release_fn(now - w.heartbeat_at >= UI_WATCHDOG_STALL_MS)) {
            w.input_released = true;
            println(std::format("[menu] frame() stalled in stage '{}': game input released", g_ui_perf.stage));
        }
    }
    int64 frames = g_ui_perf.frames;
    if (frames != w.seen_frames || w.seen_at == 0) {
        w.seen_frames = frames;
        w.seen_at = now;
        w.reported = false;
        return;
    }
    if (w.reported || now - w.seen_at < UI_WATCHDOG_STALL_MS)
        return;
    w.reported = true;
    std::string stage = g_ui_perf.stage;
    std::string line = std::format("{} frame() stopped after frame {} in stage '{}'\n", ui_clock(), frames, stage);
    println("[menu] " + line);
    ui_fault_log(line);
    if (w.stall_fn == nullptr)
        return;
    // The watchdog routine must outlive a handler that throws.
    try {
        w.stall_fn(stage);
    } catch (std::exception& e) {
        println("[menu] stall handler threw: " + std::string(e.what()));
    } catch (...) {
        println("[menu] stall handler threw: non-std exception");
    }
}

inline void ui_watch_value(const std::string& name, UiWatchFn fn) {
    for (ui_watch_t& w : g_ui_session.watch) {
        if (w.name == name) {
            w.fn = fn;
            return;
        }
    }
    ui_watch_t w;
    w.name = name;
    w.fn = fn;
    g_ui_session.watch.push_back(w);
}

inline int32 ui_watch_count() {
    return static_cast<int32>(g_ui_session.watch.size());
}

inline std::string ui_fmt_onoff(bool v) {
    return v ? std::string("on") : std::string("off");
}

inline std::string ui_fmt_f1(float64 v) {
    return std::format("{:.1f}", v);
}

inline void ui_session_set_sinks(UiFeatSinkFn feat_fn, UiStatesSinkFn states_fn, UiValuesSinkFn values_fn) {
    g_ui_session.feat_fn = feat_fn;
    g_ui_session.states_fn = states_fn;
    g_ui_session.values_fn = values_fn;
}

inline void ui_session_begin() {
    if (g_ui_session.feat_fn == nullptr)
        return;
    for (const ui_feat_t& f : g_ui_feats.list)
        g_ui_session.feat_fn(f.name, UI_FEAT_RISK_TAGS[static_cast<uint64>(f.risk)], f.risk);
}

inline void ui_session_tick(int64 now, bool attached, bool writes) {
    ui_session_t& s = g_ui_session;
    if (s.states_fn != nullptr) {
        s.states.resize(g_ui_feats.list.size());
        for (uint64 i = 0; i < g_ui_feats.list.size(); i++)
            s.states[i] = ui_feat_state(g_ui_feats.list[i], attached, writes);
        s.states_fn(s.states, now);
    }
    if (s.values_fn == nullptr || now - s.values_at < UI_WATCH_EVERY_MS)
        return;
    s.values_at = now;
    s.names.resize(s.watch.size());
    s.values.resize(s.watch.size());
    for (uint64 i = 0; i < s.watch.size(); i++) {
        s.names[i] = s.watch[i].name;
        s.values[i] = s.watch[i].fn != nullptr ? s.watch[i].fn() : std::string("");
    }
    s.values_fn(s.names, s.values);
}
