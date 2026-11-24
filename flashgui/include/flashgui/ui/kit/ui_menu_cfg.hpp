// Menu layer, part 4: the menu's own settings (window, tab and pages, scale, accent, watermark and keys HUD, safe
// start, the toggle key, auto-save) saved with the host's. The kit keeps a key=value file of its own; a host with its
// own settings store plugs it in with ui_cfg_set_io, so one file holds both. ui_cfg_setup names the file and the host's
// store / apply steps; then ui_cfg_load at start, ui_menu_tick auto-saves, ui_cfg_rows draws Save / Load / Auto-save.
// --- constants ---

// Auto-saved at most every UI_CFG_SAVE_MS, compared with the last save every UI_CFG_CHECK_MS.
const int64 UI_CFG_SAVE_MS = 2000;
const int64 UI_CFG_CHECK_MS = 250;
const std::string UI_CFG_HEADER = "# ui config - key=value per line, '#' starts a comment\n";
// A saved toggle key outside 1 .. UI_CFG_VK_MAX, or the left button, is malformed.
const int64 UI_CFG_VK_MAX = 0xFE;

// --- types ---

using UiCfgClearFn = std::function<void()>;
using UiCfgSetFn = std::function<void(const std::string& key, const std::string& value)>;
using UiCfgGetFn = std::function<std::string(const std::string& key)>;
using UiCfgTextFn = std::function<std::string()>;
using UiCfgFileFn = std::function<bool(const std::string& path)>;
using UiCfgCountFn = std::function<int64()>;
using UiCfgHookFn = std::function<void()>;
// A host's settings store. get_fn returns "" for a missing key; text_fn is the whole store as saved, key=value lines in
// store order ('#' lines skipped), to tell a change and to read the key order back (ui_cfg_pairs); set_fn appends a new
// key last; load_fn reads the file into the store in its line order (and may apply the host's own settings from it).
struct ui_cfg_io_t {
    UiCfgClearFn clear_fn = nullptr;
    UiCfgSetFn set_fn = nullptr;
    UiCfgGetFn get_fn = nullptr;
    UiCfgTextFn text_fn = nullptr;
    UiCfgFileFn save_fn = nullptr;
    UiCfgFileFn load_fn = nullptr;
    UiCfgTextFn error_fn = nullptr;
    UiCfgCountFn count_fn = nullptr;
};

// The kit's own store, used while no host store is set.
struct ui_kv_t {
    std::vector<std::string> keys;
    std::vector<std::string> vals;
    std::string error;
};

struct ui_cfgs_t {
    std::string path;
    ui_cfg_io_t io;
    bool has_io;
    // The host's keys into the store (after a clear, before the menu's), and its settings from it after a load.
    UiCfgHookFn store_fn = nullptr;
    UiCfgHookFn apply_fn = nullptr;
    bool autosave = true;
    // The store's text as last saved or loaded, to tell whether anything changed since.
    std::string text;
    int64 saved_at;
    int64 checked_at;
    std::string last_save = "never";
    // Outcome of the last load or failed save, "" after a successful save.
    std::string status;
    bool failed;
};

// --- state ---

inline ui_kv_t g_ui_kv;
inline ui_cfgs_t g_ui_cfgs;

// --- the kit's store ---



















// --- store access ---










// "" for a missing key.


















// --- key order ---

// The store's keys and values in store order, read back from ui_cfg_text: "key=value" lines, '#' lines skipped. Split
// at the first '=' and not trimmed, so a pair set back is the one read.



// Puts the store's keys in order's order, then the ones order lacks in their store order (new keys last); values are
// kept. A key of order no longer in the store stays dropped. Leaves the store alone when it is in that order already.



// --- typed values ---

// Optional sign, digits, optional '.' and digits (no exponent); false leaves out as it was.





















// Out of [lo, hi] counts as malformed: def is kept.









// A missing channel keeps c's own (configs from before the alpha picker have no .a).






// fixed_mode >= 0 ignores the saved mode; clearable accepts vk 0 (unbound). A changed bind is remade, so its toggle
// and edge state start fresh.



// Every registered feature's switch and key under prefix (prefix.<id>, key.prefix.<id>), for a host with no settings
// layer of its own. Computed switches are the host's to save.






// --- the menu's keys ---




// Positions and sizes are clamped to the view where they are drawn.



// --- save and load ---

// path: the settings file; store_fn / apply_fn: the host's own keys into the store and its settings out of it (either
// may be nullptr).












// Every key into the store: rebuilt from empty (the host's keys, then the menu's), so a key nothing writes any more - a
// cleared outfit or weapon skin - is dropped, then put back in the order the store had (the file's line order after a
// load) with new keys last, so a save diffs cleanly. The committed values, not the slider edits, so a drag in progress
// is not saved half-way.



// The outcome of a save the host wrote itself (what names it in the console); ok also counts as the last save.









// After the host's settings: the menu's, the accent everywhere, Start with risky features off, the toggle key off
// the feature keys, and every edit, capture and popup reset to what was loaded.



// Settings are left as they are when there is no file. Normalised through the store, so a load does not count as a
// change to auto-save.



// The auto-save check, from ui_menu_tick.
