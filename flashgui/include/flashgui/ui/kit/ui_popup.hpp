// Popups: the shared list popup (one open at a time), the single and multi select combos that open it, the popup menu
// opened at a point (ui_popup_menu), and the inline multi-select list.
// --- constants ---

// Most items a popup shows before it scrolls.
const int32 UI_POPUP_ROWS = 12;

// --- types ---

// The open popup of a keybind (mode list), a combo, a multi combo or a popup menu. Its rect and items are set by its
// owner each frame and hit-tested by the next ui_begin; seen drops it when the owner did not draw it (tab switch,
// collapsed group, closed view).
struct ui_popup_t {
    // Label of the owning row, "" for none.
    std::string label;
    std::vector<std::string> items;
    // Per-item icon (ui_icon_any), empty for none.
    std::vector<std::string> icons;
    // Multi combo: every item has a check box (marks) and a pick toggles it, leaving the popup open.
    bool multi;
    std::vector<bool> marks;
    // label at the start of this frame's ui_popup_input, so a second click on the owner closes it.
    std::string prev;
    bool live;
    bool seen;
    float64 x;
    float64 y;
    float64 w;
    float64 h;
    // Height of one item.
    float64 ih;
    // Highlighted item (the current mode or value), -1 for none.
    int32 mode;
    // Item under the press that is held, and the item picked this frame (-1 none) for the owning row to apply.
    int32 press;
    int32 pick;
    // First shown item, items shown, and the scrollbar gutter width (0 when every item fits).
    int32 top;
    int32 shown;
    float64 bar_w;
    // The label top was set up for on opening; "" while closed.
    std::string placed;
    // Per-item tooltips, shown only while tips_label owns the popup (ui_multi_tips).
    std::vector<std::string> tips;
    std::string tips_label;
    // A popup menu: placed at the point ax, ay (ui_popup_menu_open) rather than under an owner's box.
    bool at_point;
    float64 ax;
    float64 ay;
    // Esc as last read, so only its press closes.
    bool esc_down;
};

// --- state ---

inline ui_popup_t g_ui_pop;
// Scroll top of each popup, by owner label.
inline std::map<std::string, int32> g_ui_pop_tops;
// First shown item of each ui_multi_list, by label.
inline std::map<std::string, int32> g_ui_list_top;

// --- popup ---

// Close the open popup, keybind-mode or combo (host-side resets, e.g. after a config reload).



// Popup item under the mouse, clamped to the shown items.



// The popup's scroll range, thumb height and thumb top for its current top.









// The popup is drawn over the window, so it takes presses before any widget: an item is picked on press then
// release on it, and a press anywhere else only closes it. Esc closes it too, unless it ends a typed edit or a key
// capture (as the colour picker's).



// The open popup's items, icons, marks and current item for this frame, copied only where they differ from last
// frame's.



// The first shown item: on opening the label's last top, moved so the current item (or first marked one) shows; then
// kept in range.



// Called by the row owning the open popup, every frame it is drawn: one item_h-high item per name, right-aligned to
// right_x, just under the owner's box (top .. bottom), or just over it when the items do not fit below and there is
// more room above. At most UI_POPUP_ROWS items or what fits on screen are shown; more scroll, with a bar in a pad-wide
// gutter added to w (ui_popup_fix_top). Closes it instead once the box has scrolled out of the visible body (or group).



// --- popup menu ---

// Opens the popup menu label at the screen point (x, y), usually ui_mouse(), in place of any open popup. Its owner
// then calls ui_popup_menu every frame it should stay open. label only keys it: make it unique ("Options##map").






// Moves the open popup menu label to the screen point (x, y), keeping it open: for a menu on a point that moves (a map
// point under a pan or zoom). Call before ui_popup_menu; nothing when label is not open.



// Options a popup menu of n shows at once, item_h (ih) high each: at most UI_POPUP_ROWS and what fits the screen's
// height; the rest scroll.



// The size ui_popup_menu gives a menu of these options, so a host can open it with its far corner at a point (above a
// button, say).



// The open popup menu label: one item_h-high button per option, its top-left at the opening point, moved left of it
// or above it where it would leave the screen, and kept on screen. Drawn over everything by ui_end, in the theme with
// the hovered option marked in the accent. Returns the option clicked this frame (press and release on it), which also
// closes it, else -1. It also closes on a press outside it (used up, so nothing under it is clicked), on Esc, and on
// any frame its owner does not call this. Call between ui_begin and ui_end.



// ui_popup_menu with option current (-1 for none) marked as the one in use: the accent bar and its text in the accent,
// and scrolled into view when the menu opens.



// The open popup, over everything else in the window and not clipped to it. Dropped when its row was not drawn this
// frame.



// The chevron at the right end of a combo field (fx, fy, fw, fh), centred in its arrow room cw, as large as on a
// frame_h field.



// A combo row's field line: the combo_h field with the margins a frame_h field has in a row_h row.



// --- combo ---

// Row width a combo needs: its label, and a field holding the arrow (cw) and a stub of the names (longer ones are cut).



// Single-select combo: the label, then the current name in a full-width combo_h field under it; a click opens the list
// under it and a pick sets index. True when index changed.



// ui_combo with an icon per item (ui_icon_any: a PNG path or an icon key, "" for none) in the popup and before the
// current name in the field.



// --- multi combo ---

// Like ui_combo: the label, then a full-width combo_h field with the picked names joined with ", ", cut with "..."
// before the arrow. A click opens a popup of check boxes, one per name, that stays open while items are toggled. picked has one
// entry per name.
// True when an item was toggled.



// The field's content when every picked item has a loaded icon and they fit: one icon per picked item, is wide.



// ui_multi_combo with an icon per item (ui_icon_any: a PNG path or an icon key, "" for none) in the popup, and in the
// field instead of the names while every picked icon fits.



// Right after ui_multi_combo(label, ...): tips[i], when not empty, is item i's tooltip in its open popup. Copied only
// where it differs from last frame's.



// --- inline multi list ---

// An inline multi-select box for lists too long for a popup (which never scrolls): rows items high (item_h, as a
// popup's), each a check box, icon (ui_icon_any, asked for only while its row is shown), name and a dim note at the
// right end. The wheel over it scrolls the list instead of the body; pressing the bar at its right edge drags it. A
// click flips picked[i]; true then.
