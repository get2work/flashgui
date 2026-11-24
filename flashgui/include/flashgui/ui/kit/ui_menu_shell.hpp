// Menu layer, part 3: the menu app around the widgets. The host sets it up once (ui_menu_setup, ui_menu_add_tab per
// sidebar tab with its pages and draw delegate, footer indicators, the watermark's parts) and then, every frame:
// shown = ui_menu_poll(fg) after the features polled their keys, ui_menu_draw(now) while shown, ui_menu_hud() for the
// watermark and the keys HUD, ui_menu_tick(now) for the accent, the picture sweep, tab ticks and config auto-save.
// --- constants ---

// Scroll offsets are kept per page key: tab * UI_MENU_PAGE_KEYS + page.
const int32 UI_MENU_PAGE_KEYS = 8;
// One name per UI_SCALE_PRESETS entry.
const std::vector<std::string> UI_SCALE_NAMES = { "1.25x", "1.5x", "2x" };
// The keys HUD: its title and floating panel slot (ui_float_drag_at).
const std::string UI_MENU_KEYS_TITLE = "Keybinds";
const int32 UI_MENU_KEYS_SLOT = 0;
// ui_menu_t.sized_for after a config load: no view or tab matches it, so the next draw fits the window again.
const int32 UI_MENU_UNSIZED = -2;

// --- types ---

using UiTabDrawFn = std::function<void(int32 page, int64 now)>;
using UiTabFn = std::function<void()>;
using UiIndicatorFn = std::function<bool()>;
using UiWmPartsFn = std::function<void(std::vector<std::string>& parts)>;
// Returns the reading, or ui_wheel_none(): a float64& parameter never reaches a function bound to a delegate (README,
// "Delegates").
using UiWheelReadFn = std::function<float64()>;
using UiWheelFrameFn = std::function<int64()>;
using UiMenuHookFn = std::function<void()>;
// A view's canvas (ui_menu_add_view): the rect it owns this frame, by value.
using UiViewCanvasFn = std::function<void(float64 x, float64 y, float64 w, float64 h, int64 now)>;
// One sidebar tab. No pages: its name is its only page. cols[page] is the body's column count (the last entry for later
// pages, 2 without any).
struct ui_tab_t {
    std::string name;
    std::vector<std::string> pages;
    std::vector<int32> cols;
    // The hitch log's stage while it draws.
    std::string stage;
    int32 page;
    UiTabDrawFn draw_fn = nullptr;
    // Slider edits: reset to the settings, committed to them (not while a drag is held).
    UiTabFn reset_fn = nullptr;
    UiTabFn commit_fn = nullptr;
    // Every frame, menu open or not.
    UiTabFn tick_fn = nullptr;
    UiTabFn shutdown_fn = nullptr;
};

struct ui_indicator_t {
    std::string on_text;
    std::string off_text;
    color on_col;
    UiIndicatorFn fn = nullptr;
};

// A top-bar button (ui_head_button): click_fn runs on a click, lit_fn lights it.
struct ui_menu_head_t {
    std::string icon;
    int32 glyph;
    std::string tip;
    UiMenuHookFn click_fn = nullptr;
    UiIndicatorFn lit_fn = nullptr;
};

// A view: a page outside the sidebar that takes the body's place while open, usually opened by a head button. With a
// canvas_fn the body's left part is its canvas and draw_fn's groups take a side_w column at the right (side_w 0: all
// canvas). While open the window is at least min_w x min_h (at 1.0 scale, within the screen); closing gives back the
// size it had, unless it was resized meanwhile.
struct ui_view_t {
    std::string name;
    std::string stage;
    float64 side_w;
    float64 min_w;
    float64 min_h;
    UiViewCanvasFn canvas_fn = nullptr;
    UiTabDrawFn draw_fn = nullptr;
};

// A mouse button as read at the previous frame.
struct ui_menu_button_t {
    // The reader's toggle bit, to catch a click that starts and ends between frames.
    bool toggle;
    bool down;
    // From a good read; false after a failed one, so the next good read is a baseline, not a tap.
    bool valid;
};

struct ui_menu_t {
    std::string title;
    bool open;
    // Open and the host in front: only then is the menu drawn and fed input.
    bool shown;
    bool fg;
    // The toggle key, always Toggle, and the keys a clash on load falls back to.
    ui_keybind_t key;
    std::vector<int64> key_fallbacks;
    // The text box and key box that had the keys at the end of the last drawn frame.
    std::string edit;
    std::string capture;
    std::vector<ui_tab_t> tabs;
    int32 tab;
    std::vector<std::string> page_one = { "" };
    std::vector<ui_menu_head_t> heads;
    std::vector<ui_view_t> views;
    // The open view, -1 for the tab's own page; the view the window was last sized for, the size before a view grew it
    // and the size it grew to.
    int32 view = -1;
    int32 sized_for = -1;
    bool grown;
    float64 grow_from_w;
    float64 grow_from_h;
    float64 grown_w;
    float64 grown_h;
    // ui_perf_frames when last drawn, so a menu shown again takes a fresh mouse baseline.
    int64 drawn_frame = -2;
    std::vector<ui_indicator_t> indicators;
    std::vector<std::string> foot_texts;
    std::vector<color> foot_cols;
    ui_menu_button_t lb;
    ui_menu_button_t rb;
    // A wheel the Input API cannot see (a game's own scroll value) and the frame it belongs to, -1 for unknown.
    UiWheelReadFn wheel_fn = nullptr;
    UiWheelFrameFn wheel_frame_fn = nullptr;
    float64 wheel_last;
    int64 wheel_frame = -1;
    UiMenuHookFn close_fn = nullptr;
    UiWmPartsFn wm_fn = nullptr;
    std::vector<std::string> wm_parts;
    bool watermark_on = true;
    bool keys_hud_on = true;
    ui_float_rect_t keys_rect;
    std::vector<std::string> hud_names;
    std::vector<std::string> hud_keys;
    // Keys-HUD cache: an integer signature of the active keyed set (and the scale) and the panel width measured for it,
    // so the per-frame HUD rebuilds its rows and re-measures its width only when that set changes.
    uint64 hud_sig;
    bool hud_ready;
    float64 hud_w;
};

// --- state ---

inline ui_menu_t g_ui_menu;

// --- setup ---

// title: the window title (its part after " - " is the sidebar subtitle); key_vk: the default toggle key, before the
// config loads; fallbacks: the keys tried in order when a loaded toggle key clashes with a feature key.



// The next sidebar tab, in display order; returns its index.


















// The tab shown until the config says otherwise.



// A top-bar button, right to left in registration order: icon (PNG path or icon key, through the icon provider) or
// else glyph (UI_GLYPH_*), its tooltip, what a click runs and what lights it (nullptr: never lit).



// A view (ui_view_t); returns its index. side_w and the minimum size are at 1.0 scale.









// The groups' column of a view with a canvas, at 1.0 scale; 0 gives the whole body to the canvas.






// The open view, -1 while the tab's page shows.



// Opens view (-1 closes it); the window follows at the next draw (ui_menu_view_fit).



// The window's own size: the one the config saves and a closing view gives back. While a view holds the window at the
// size it grew it to, the size from before the view; once the size changes during the view (the grip, a config load),
// that size is the window's own from then on.



// Before ui_begin, when the open view changed: the window's own size comes back (ui_menu_window_size), then the newly
// open view grows it to its minimum within the screen.






// A footer indicator, in display order: on_text in on_col while fn says on, else off_text muted.









// What a wheel source returns when it has no reading (NaN).



// Runs whenever the menu closes, after its edits are committed.









// Shown, and a text box or key box has the keys: the host keeps the typing keys from its target.



// --- mouse ---

// A mouse button, read once per frame. The key reader is preferred: it reports the live state whether or not the host
// has the focus, and its toggle bit flips on every press, so a click shorter than a frame still becomes a tap. Without
// it the Input API's down state is used (no taps).






// A fresh baseline, so a click made while the menu was hidden or in another app (or the one that brought the host back)
// does not land on it as a press or a tap.



// The wheel since the last frame, positive up: the Input API's, else the host's source. That one holds a frame's
// notches until its owner clears them, so it is taken once per frame when the source knows its frame, else per change.



// --- open and close ---




// Not while held: a feature sees one value per release rather than every step of a drag.






// The toggle key, unmuted (the feature-key mute is on while the menu is open) and with no swallow. A failed read
// clears seen, so the next good one is a baseline rather than an edge.



// Every frame, after the features polled their keys. fg: the host is the foreground window; the menu shows only then,
// and stays open across a switch away. The key is read every frame (the reader sees presses made in any app) but
// toggles only in front and not while a key box or text box has the keys. Returns shown.



// --- draw ---

// The keys the capture refuses: the host's reserved ones, and the toggle key, or with menu_key_row every bind (the
// toggle key keeps off the feature keys and they off it).



// The top bar: the tab's pages, or its name as its only page. Returns the page shown.









// The open tab's page bar and body.



// The open view: a back arrow and its name in the top bar, its canvas left of a side column of groups (or the whole
// body as one or the other). Its groups scroll under their own page key, -1 - view.



// While shown. The stages (menu_overlay/head, the tab's or view's, menu_overlay/end) name which part of a slow draw
// was slow.



// --- HUD ---

// One row per keyed feature switched on whose key is active now: Toggle switched on, On hold held. Always on binds are
// not listed.



// The keybind list: a see-through panel, a title, then a row per active key, its feature left and its key right. With
// nothing active it hides while the menu is closed and shows its title while open, so it can be dragged. Sits under
// the watermark until the first drag places it; dragged through a float slot, whose press the next ui_begin takes.



// After ui_menu_draw, so both sit over the menu; only with the host in front. Dragged while the menu shows.



// Every frame, menu open or not: the accent's derived colours (HUDs draw with the menu closed too), the tab ticks,
// then the config auto-save.



// --- rows ---




// The toggle key's row: lit while drawn, never cleared, kept off every feature key. extra ends its tooltip.









// The toggle key keeps off the feature keys: a hand-edited clash falls back to the first free fallback.



// On unload: the tabs' shutdowns, then every card picture and its loader, so a reload starts clean.
