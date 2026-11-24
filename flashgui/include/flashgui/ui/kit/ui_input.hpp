// Keyboard: typed number edits, text inputs, key capture and keybinds (ui_keybind_t). Keys come from
// ui_in_key_bits (ui_backend.em) because the game hides the keyboard from the Input API.
// --- constants ---

// A capturing keybind cycles ".", "..", "..." (up to UI_DOTS_N dots) at this period.
const int64 UI_DOTS_MS = 300;
const int64 UI_DOTS_N = 3;
// How long a keybind box shows a refusal ("key not allowed").
const int64 UI_BIND_FLASH_MS = 1200;
// ui_keybind_t modes, in popup order (same values as most hosts' KB_* constants).
const int32 UI_KB_ALWAYS_ON = 0;
const int32 UI_KB_TOGGLE = 1;
const int32 UI_KB_HOLD = 2;
const std::vector<std::string> UI_KB_MODE_NAMES = { "Always on", "Toggle", "On hold" };
// Virtual-key codes ui_capture_read scans, 1 .. UI_VK_COUNT - 1.
const int32 UI_VK_COUNT = 256;
// The '-' and '.' keys of the main block; vk:: has no names for them.
const int64 UI_VK_OEM_MINUS = 0xBD;
const int64 UI_VK_OEM_PERIOD = 0xBE;
const int64 UI_VK_OEM_COMMA = 0xBC;
const int64 UI_VK_OEM_QUOTE = 0xDE;
// Generic Shift and both sides: the kernel state is not known to set the generic one on every path.
const std::vector<int32> UI_VK_SHIFTS = { 0x10, 0xA0, 0xA1 };
// A typed number edit holds at most this many characters, a text input UI_TEXT_MAX_CHARS unless its row sets fewer
// (ui_text_input_max); the caret blinks at this period.
const uint64 UI_EDIT_MAX_CHARS = 12;
const uint64 UI_TEXT_MAX_CHARS = 48;
const int64 UI_CARET_MS = 500;
// A held key in a typed edit repeats after UI_REPEAT_DELAY_MS, then every UI_REPEAT_MS.
const int64 UI_REPEAT_DELAY_MS = 400;
const int64 UI_REPEAT_MS = 35;
const std::string UI_SEARCH_CLEAR = "x";
// ASCII codes of '0', 'A' and 'a': the digit and letter key codes map onto them in order.
const int64 UI_CHAR_0 = 48;
const int64 UI_CHAR_UPPER_A = 65;
const int64 UI_CHAR_LOWER_A = 97;

// --- types ---

// A rebindable key and when it counts as active (kb_active). The fields after mode are kb_poll's.
struct ui_keybind_t {
    // 0: unbound, never pressed.
    int64 vk;
    int32 mode;
    // KB_TOGGLE's state.
    bool on;
    // The key's kernel bits at the last poll, valid while seen.
    uint8 bits;
    bool seen;
    // Set on bind: the key is ignored until released, so the press that bound it does not also act.
    bool swallow;
    // The bind counts as active: kb_active's last result, or what the app sets for a bind it polls itself. Its key box
    // shows it as an accent border.
    bool active;
};

// The keybind waiting for a key (one at a time) and ui_capture_poll's state, shared with a typed number edit (ui_edit_t).
struct ui_capture_t {
    // Label of the capturing keybind, "" for none.
    std::string label;
    int64 started_at;
    // The capturing keybind was drawn this frame; ui_begin drops a capture whose row went away (tab switch).
    bool seen;
    // Per VK: the last good kernel bits (valid while bits_seen) and whether either source had the key down at the
    // previous poll. Taken whole when the capture starts, so a key held then waits for its release.
    std::vector<uint8> bits;
    std::vector<bool> bits_seen;
    std::vector<bool> down;
    // ui_capture_read's per-VK Input API state, reused every poll.
    std::vector<bool> api;
    // Every key ui_capture_read saw pressed in its last poll, in VK order, for a typed edit.
    std::vector<int64> hits;
    // First key ui_capture_poll took for the capture, 0 while waiting; the capturing row applies it.
    int64 key;
    // The key that ended the last capture: no keybind sees a press until it is released, -1 for none.
    int64 mute_vk = -1;
    // The last repeatable key pressed in a typed edit while it stays down (-1 for none), and when it fires next.
    int64 repeat_vk = -1;
    int64 repeat_at;
    // A refusal ("key not allowed") shown in one keybind box for UI_BIND_FLASH_MS.
    std::string flash_label;
    std::string flash;
    int64 flash_at;
};

// The number box or text input being typed into, one at a time. ui_capture_poll feeds it keys (kernel key state: the
// Input API sees no keyboard while the game is focused); the owning row applies or drops it (ui_edit_apply,
// ui_text_input). While it runs, keybinds see no press (ui_keys_muted) and the host keeps the typing keys from its
// target (ui_menu_typing).
struct ui_edit_t {
    // Label of the owning row, "" for none.
    std::string label;
    std::string text;
    // A text input (ui_text_input): letters, digits and a few marks, appended to the text shown, at most max_chars.
    bool free;
    uint64 max_chars = UI_TEXT_MAX_CHARS;
    // The commit came from Enter, not a press outside.
    bool enter;
    // Nothing typed yet: the first character replaces the value shown.
    bool fresh;
    // Enter, or a press outside the box: the row applies text. Esc: the row drops it.
    bool commit;
    bool cancel;
    // The owning row was drawn this frame; ui_begin drops an edit whose row went away.
    bool seen;
    int64 started_at;
    // The box last frame, for ui_begin's outside-press test.
    float64 x;
    float64 y;
    float64 w;
    float64 h;
};

// A key box's text and its width at font, redone only when the key, the refusal flash or the font changes.
struct ui_key_text_t {
    int64 vk = -1;
    bool flash;
    int64 font;
    std::string text;
    float64 w;
};

// --- state ---

inline ui_capture_t g_ui_cap;
inline ui_edit_t g_ui_edit;
inline std::map<std::string, ui_key_text_t> g_ui_key_text;
// Keys no keybind may take, other than the one it already holds. The caller fills it before drawing the rows.
inline std::vector<int64> g_ui_keys_taken;
// The game is foreground and the overlay menu is closed; the entry script sets it each frame before kb_active.
inline bool g_kb_live;

// --- typed number edits ---




// The label of the row being typed into right now, "" for none. The host uses it to keep the typing keys from the game
// (so they go to the text box instead) while a text input is focused.






// Starts typing into label's box (any key capture ends), with every key held now waiting for its release.



// From ui_capture_read's per-VK state, so it matches the keys being taken this poll.



// The character a text input takes for key k (US layout), "" for none. Every key here is in lib/game_input.em
// GI_TYPE_VKS, so none of them reaches the game while typing.



// One key pressed while typing: digits (main block or keypad, Shift ignored), '.', a leading '-', Backspace, Enter
// (commit) and Esc (cancel); a text input takes ui_text_char instead. The key that ends the edit is muted until
// released, like a bound key.



// Backspace and every key that types a character repeat while held; there is no caret, so arrows would do nothing.



// The kernel key state gives edges only, so a held key is replayed here from ui_capture_read's per-VK down state.



// Called by label's row every frame it is drawn: applies a committed edit to v, kept in [lo, hi] on the step grid,
// or drops a cancelled one. Text that is not a number leaves v as it was. True when v changed.



// The value of label's row in the box (x, y, w, h); a click on it starts typing a new one (not while another edit is
// still waiting to commit). framed draws the box itself; otherwise only while typing (the stepper frames its own).
// While typing: the accent border, the text with a blinking caret, and a highlight while the first key would replace it.



// The typed edit's blinking caret, 1 px wide at x in the box at y, h high.



// --- text inputs ---




// A one-line text box at (x, y, w, h) holding text. A click focuses it; keys come through ui_capture_poll (ui_text_char,
// Shift for capitals), Backspace deletes, Enter keeps the text and submits, a press outside keeps it, Esc drops it.
// text only changes when the edit ends. hint shows muted while it is empty. True on the Enter frame.



// ui_text_input taking at most max_chars characters.



// A toggle with a one-line text input before its check box: a click on the label or the box flips v, a click on the
// input types into text (at most max_chars characters, taken when the edit ends, as ui_text_input). The input is as wide
// as max_chars digits, so the label keeps room for its ui_tip. True when v or text changed.



// A text input filling the row left of its buttons, each sized to its caption. The index of the button clicked, 0 on
// Enter in the input (the first button submits), -1 for none.



// A search box with a clear button. Returns the query to filter by: the text being typed while the box has focus (so
// the list follows each key), else text. The button clears text and is muted while there is nothing to clear.



// --- key capture ---

// ui_in_key_name has no names for the mouse buttons.






// The label of the key box capturing a key right now, "" for none. The host uses it to keep every key from the game
// while a bind is being set.



// While a capture or a typed edit runs, and after it until the key that ended it is released, keybinds see no press
// EDGE (kb_poll). A held KB_HOLD key is not gated by this - that is the per-bind swallow's job (kb_held).



// Clears the post-capture mute once its key is released. Must run every frame, menu shown or not - ui_menu_poll and
// ui_capture_poll both call it - so a bind made while holding its key does not leave every feature key muted after the
// menu closes before the release.



// Kernel bits through ui_in_key_bits (a failed read is skipped and makes the next good one a baseline) plus the Input
// API's down keys, which it reports while the kernel reader is unresolved or unconfirmed. snapshot only takes the state.
// The left button is never a bind: a click on the box cancels and a click elsewhere does nothing.









// Call once per frame before any keybind is polled. live: the menu is shown, so a key pressed in another app is
// tracked (it stays blocked while held) but not taken. A typed edit takes every key pressed since the last poll, and
// ends when the menu is hidden or the game loses the foreground.



// --- keybinds ---

// The kit's keybind widgets act on ui_keybind_t through kb_poll / kb_held / kb_active below and kb_rebind. A host that
// has its own keybind type (a wider struct with more fields) keeps it; the overlap of (vk, mode, on, bits, seen) carries
// across cleanly through a view / write-back bridge on the host side.



// Reads the key once; true on a press since the previous poll, including one that started and ended between two
// polls (the kernel toggle bit flips on every press). Kept per keybind rather than in one shared per-VK state, so
// two keybinds on one key both see the press. No press while ui_keys_muted; the state still follows the key.
// A failed read is no reading: it clears seen, so the next good one is a fresh baseline rather than an edge.



// KB_HOLD's state from the bits the last kb_poll left; never polls, so it also serves readers after kb_active. Follows
// the physical key while g_kb_live; the per-bind swallow (set on rebind, cleared on release) keeps the keystroke that
// bound it from counting, so it is NOT gated by the global post-capture mute - a stuck mute never freezes a held key.



// Call once per frame per keybind. KB_ALWAYS_ON follows enabled alone; KB_TOGGLE flips on each press made while
// g_kb_live and keeps its state outside it (callers gate their own output); KB_HOLD follows the key only while g_kb_live.
// The result is kept in kb.active for its key box.









// A changed key starts with no toggle state and no edge baseline.



// The left button clicks the menu, so it is never a bind.



// Width of label's key box this frame: the key name (in every mode) or the refusal shown, at least key_w and the
// capture dots. The text and its width are kept in g_ui_key_text[label] for ui_keybind to draw.



// The key box of the row at row_y, its right edge at right_x; label names the row (one capture or popup per label).
// A click starts a capture and the next key or mouse button pressed is bound; another click on the box cancels. Esc
// unbinds (vk 0) when clearable, else cancels. With modes, a right click opens the mode popup under the box. The border
// is the accent while kb.active (Always on and enabled, Toggle on, On hold held). True when the key or the mode changed.



// Hotkey plus an independent checkbox on one row: label, key box (with the mode popup), checkbox at the right end. A
// click on the label or the checkbox flips enabled.



// ui_checkbox_bind; modes false hides the mode popup, for a key that only acts on its press (kb_poll).



// ui_checkbox_key in the left part of the row and a plain label2 check box in the right part; tip / tip2 are the halves'
// tooltips (after this row, a ui_tip would land on the right half). True when anything changed.



// A label with a key box at the right end; modes false hides the mode popup (a fixed mode).
