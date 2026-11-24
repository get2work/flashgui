// Menu layer, part 2: the frame trace and hitch log, the fault log, the watchdog, and the session's watched values and
// feature states. The host names its log files once (ui_perf_setup), brackets each frame with ui_perf_frame_begin /
// ui_perf_frame_end and names stages with ui_perf_stage; a second routine calls ui_watchdog_tick.
// --- constants ---

// Hitch log, emptied by ui_perf_setup: frames slower than UI_PERF_SLOW_MS (with their slowest stage) and frames that
// started UI_PERF_LATE_MS or more after the last one ended. At most UI_PERF_BURST lines per UI_PERF_EVERY_MS and
// UI_PERF_LOG_MAX bytes in all.
const int64 UI_PERF_SLOW_MS = 40;
const int64 UI_PERF_LATE_MS = 100;
const int64 UI_PERF_EVERY_MS = 1000;
const int64 UI_PERF_BURST = 3;
const int64 UI_PERF_LOG_MAX = 262144;
// A stalled frame no longer hands the game its input back, so the watchdog releases it after UI_WATCHDOG_INPUT_MS;
// UI_WATCHDOG_STALL_MS is longer than any legitimate frame, and a stall that long also runs the host's stall handler.
const int64 UI_WATCHDOG_INPUT_MS = 1500;
const int64 UI_WATCHDOG_STALL_MS = 8000;
// Watched values are compared at most this often.
const int64 UI_WATCH_EVERY_MS = 500;

// --- types ---

using UiHeartbeatFn = std::function<int64()>;
using UiInputReleaseFn = std::function<bool(bool force)>;
using UiStallFn = std::function<void(const std::string& stage)>;
using UiWatchFn = std::function<std::string()>;
using UiFeatSinkFn = std::function<void(const std::string& name, const std::string& tag, int32 rank)>;
using UiStatesSinkFn = std::function<void(const std::vector<int32>& states, int64 now)>;
using UiValuesSinkFn = std::function<void(const std::vector<std::string>& names, const std::vector<std::string>& values)>;
struct ui_perf_t {
    std::string log_path;
    std::string fault_path;
    // Frames ended so far; the watchdog watches it advance.
    int64 frames;
    std::string stage;
    // ui_sys_perf_time at this frame's start, the last frame's end and the current stage's start; the slowest stage.
    int64 frame_t;
    int64 frame_end_t;
    int64 stage_t;
    int64 slow_t;
    std::string slow_stage;
    // A fault was caught this frame / since the last clean frame, and how many.
    bool faulted;
    bool fault_streak;
    int64 faults;
    // The current UI_PERF_EVERY_MS window, lines written in it, lines dropped since the last one, bytes written.
    int64 at;
    int64 n;
    int64 skipped;
    int64 bytes;
};

struct ui_watchdog_t {
    UiHeartbeatFn heartbeat_fn = nullptr;
    UiInputReleaseFn release_fn = nullptr;
    UiStallFn stall_fn = nullptr;
    // The heartbeat as last seen and when it changed; the input was released since then.
    int64 seen_heartbeat;
    int64 heartbeat_at;
    bool input_released;
    int64 seen_frames;
    int64 seen_at;
    bool reported;
};

struct ui_watch_t {
    std::string name;
    UiWatchFn fn = nullptr;
};

struct ui_session_t {
    std::vector<ui_watch_t> watch;
    UiFeatSinkFn feat_fn = nullptr;
    UiStatesSinkFn states_fn = nullptr;
    UiValuesSinkFn values_fn = nullptr;
    int64 values_at;
    // Reused every tick.
    std::vector<int32> states;
    std::vector<std::string> names;
    std::vector<std::string> values;
};

// --- state ---

inline ui_perf_t g_ui_perf;
inline ui_watchdog_t g_ui_watchdog;
inline ui_session_t g_ui_session;

// --- clock ---




// HH:MM:SS local time.



// --- frame trace and hitch log ---

// Names both logs and empties the hitch log; once, at load. An empty path turns that log off.












// One line in the hitch log, within its rate and size limits (lines over the rate are counted into the next one).






// The stage running since stage_t ends at t; the frame's slowest so far is kept for ui_perf_frame_end.



// Names the stage about to run: the hitch log's slowest stage and the stage a stalled frame stopped in.



// A caught exception this frame; the first clean frame after a run of them is noted in the fault log.



// Frame start: a frame that began UI_PERF_LATE_MS or more after the last one ended waited outside the frame.



// Frame end: the end of a fault run, a slow frame with its slowest stage, then the frame counts as done.



// --- watchdog ---

// heartbeat_fn advances while the host's input owner runs, release_fn(force) hands the game its input back (force once
// the frame counts as dead), stall_fn runs once per stall with the stage the frame stopped in. Any may be nullptr.



// From a second routine, which outlives a stopped frame. Reads the frame's fields only once they stop advancing, so it
// never races their writes.



// --- watched values ---

// A named setting the session log follows, as text; registered once, in log order. A name already watched is replaced.












// --- session ---

// Where the registry's features and watched values go: feat_fn once per feature (ui_session_begin), states_fn every
// tick with one UI_FEAT_* per feature in registration order, values_fn every UI_WATCH_EVERY_MS. Any may be nullptr.



// After every feature is registered.



// Every frame, after the features ran. attached: the target is there; writes: writing features may act.
