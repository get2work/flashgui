// Menu layer, part 1: the feature registry and everything that reads it. The host registers each feature once: id,
// name, risk tag, what of it is visible, its switch (a bool or a predicate) and its key. The kit then draws its rows and
// tooltips, checks key clashes, applies "Start with risky features off", lists it in the keys HUD (ui_menu_shell.em)
// and feeds the session log (ui_menu_log.em). Registration order is display order everywhere.
// --- constants ---

// Visibility of a feature's effects, drawn as a pill after its label and keyed into its tooltip's coloured header.
// READ is SAFE (no pill), CLIENT is LOW, SERVER is MED, SERVER_RISKY is RISKY: server-visible combat, movement or
// pickups beyond normal play, which "Start with risky features off" leaves off at load.
const int32 UI_FEAT_READ = 0;
const int32 UI_FEAT_CLIENT = 1;
const int32 UI_FEAT_SERVER = 2;
const int32 UI_FEAT_SERVER_RISKY = 3;
const std::vector<std::string> UI_FEAT_RISK_TAGS = { "SAFE", "LOW", "MED", "RISKY" };
// Tooltip prefixes by risk; the default tooltip kinds (ui_style.em) give each a coloured header.
const std::vector<std::string> UI_FEAT_PREFIXES = { "[READ]", "[CLIENT]", "[SERVER]", "[SERVER]" };
// A feature's session state (ui_feat_state): off, switched on with its key idle, acting now.
const int32 UI_FEAT_OFF = 0;
const int32 UI_FEAT_ARMED = 1;
const int32 UI_FEAT_ACTIVE = 2;

// --- types ---

using UiFeatEnabledFn = std::function<bool()>;
using UiFeatActiveFn = std::function<bool()>;
using UiFeatStatusFn = std::function<std::string()>;
// One registered feature. The host owns the switch and the bind; the kit reads and edits them through the pointers.
struct ui_feat_t {
    int32 id;
    std::string name;
    // The keys HUD's label when it differs from name ("" for name).
    std::string hud_name;
    // What of it is visible, after the risk tag in its tooltip ("only your sky changes").
    std::string what;
    int32 risk;
    // The switch: enabled_fn when set (a computed one, never cleared), else *enabled_p, else always on.
    bool* enabled_p = nullptr;
    UiFeatEnabledFn enabled_fn = nullptr;
    // nullptr for a feature without a key.
    ui_keybind_t* bind_p = nullptr;
    // Acting now, for a feature whose activity is not its key (a flight in progress); else the key decides.
    UiFeatActiveFn active_fn = nullptr;
    UiFeatStatusFn status_fn = nullptr;
};

struct ui_feats_t {
    std::vector<ui_feat_t> list;
    // Added to the tooltip of every feature that writes (not READ), e.g. " Needs Write memory.".
    std::string write_note;
    // Keys no bind may take (the host's own) and binds outside the registry that still count for clashes.
    std::vector<int64> reserved;
    std::vector<ui_keybind_t*> extra_binds;
    // Start with risky features off, and how many the last config load turned off.
    bool safe_start = true;
    int32 safe_off;
};

// --- state ---

inline ui_feats_t g_ui_feats;

// --- registry ---

// Adds f, or replaces the entry with its id. Call once per feature at setup, in display order.






// A switch without a key.



// A switch and its key.



// A computed switch ("any of four picks"), with or without a key.



// A switch whose activity is active_fn's rather than its key's.






// By registration order, nullptr out of range.



// By id, nullptr for an unknown one.















// --- keybind state ---

// A key box's accent border: Always on while enabled, Toggle while switched on, On hold while its key is held (the bits
// kb_active read this frame, so a held key shows even while the open menu keeps it from the feature).



// A fixed-mode key's box (no mode popup: a menu key, a click key) is lit while its key is physically held. Read here
// for display only, so it takes no press edge from the bind's owner.



// The bind's state as kb_active left it this frame, without polling it again.



// Acting now: active_fn's answer, else the key's state; a feature without either is acting whenever it is on.



// UI_FEAT_OFF / ARMED / ACTIVE. Acting counts only while attached, and for a writing feature only while writes are
// allowed, so a log's on-time is time the feature could act.



// --- rows and tooltips ---

// A row's tooltip with the live status under it, in place of an always-visible status line.



// The risk pill after the last row's label, before its tooltip's "?". SAFE has none.



// Feature id's pill, then its tooltip: the risk's prefix (its coloured header), tag and what is visible, tip, the write
// note for a writing feature, and status.



// Label, key box (mode popup on right-click with modes), checkbox; the box lit by ui_kb_lit. True when anything changed.



// A label with a key box at the right end. With modes the box lights as on a switch row, by switch_on (the bind alone
// when the row follows no switch); a fixed-mode key's while its key is held.






// A fixed-mode label row whose box the caller lights.



// A feature's row from its entry under label: checkbox with its key box when it has a key, else a toggle, then its
// tooltip, the status shown only while it is on. True when anything changed.



// ui_feat_row_as under the feature's own name.



// --- key checks ---




// A bind that is not a feature (a click key, a modifier) but still may not share a key with one.



// Every registered feature's key, then the extra binds'.









// --- start with risky features off ---










// Switches off every RISKY feature with a bool switch, Always on keys included; computed switches have nothing to
// clear. Returns how many were on.
