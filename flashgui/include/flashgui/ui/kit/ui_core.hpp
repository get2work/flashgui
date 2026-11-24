// Shared immediate-mode state: ids, the mouse and its edges, hit testing, the row cursor, text fitting and the
// primitives every widget draws with.
// --- ids ---

// g_ui_active while the button is held after a press that hit no widget, so dragging onto a row does nothing.
const int32 UI_ID_NOWHERE = -1;
// g_ui_active while the watermark is dragged; it lives outside the window, so it has no per-frame id.
const int32 UI_ID_WATERMARK = -2;
// g_ui_active while a press on the open popup is held.
const int32 UI_ID_POPUP = -3;
// g_ui_pop.press while the popup's scrollbar is held.
const int32 UI_POP_BAR = -2;
// g_ui_active while the body's scrollbar is dragged; taken after a variable number of rows, so it has no per-frame id.
const int32 UI_ID_SCROLLBAR = -4;
// g_ui_active while a press on the colour picker that hit none of its parts is held.
const int32 UI_ID_PICKER = -5;
// g_ui_active while floating panel slot s (ui_float_drag_at) is dragged is UI_ID_FLOAT - s; drawn after the menu, like
// the watermark.
const int32 UI_ID_FLOAT = -6;
// The picker's parts take ids from here, drawn after a variable number of rows, so a drag keeps its id.
const int32 UI_ID_PICKER_BASE = 1000000;
// Body scroll per wheel notch, in rows. Only the sign of the delta is used: its units are not confirmed live.
const float64 UI_SCROLL_ROWS = 2.0;
// Vector glyphs of the square buttons (ui_head_button, ui_tilemap_button), drawn by ui_draw_glyph.
const int32 UI_GLYPH_NONE = 0;
const int32 UI_GLYPH_MAP = 1;
const int32 UI_GLYPH_BACK = 2;
const int32 UI_GLYPH_PLUS = 3;
const int32 UI_GLYPH_MINUS = 4;
const int32 UI_GLYPH_TARGET = 5;
const int32 UI_GLYPH_PANEL = 6;
const int32 UI_GLYPH_CLOSE = 7;

// --- types ---

// The icon slot the next row's label starts with (ui_icon_next); key is an icon key (ui_gfx_icon_key), "" for none.
// path, when set (ui_icon_next_path), is a PNG drawn instead of key.
struct ui_icon_t {
    bool on;
    std::string key;
    std::string path;
};

// A draggable rect drawn over everything outside the window (the watermark, a floating panel); ui_begin hit-tests last
// frame's x, y, w, h while live. The watermark follows the top-right corner until the first drag sets placed.
struct ui_float_rect_t {
    bool placed;
    float64 x;
    float64 y;
    float64 w;
    float64 h;
    bool live;
};

// The last row drawn, for ui_tip: where its label ends and how far a marker may reach before the widget.
struct ui_row_t {
    float64 x;
    float64 y;
    float64 h;
    float64 label_end;
    float64 limit;
};

// The wrap of one ui_text_colored call, by call order in the frame, redone only when text, width or font changes.
struct ui_wrap_t {
    std::string text;
    float64 w;
    int64 font;
    std::vector<std::string> lines;
};

// --- state ---

// Widest row content (label plus widget, unclipped) measured this frame, and the widest group column needed by any
// page seen at the applied scale, per 1.0 of scale. ui_min_w_at keeps UI_MIN_COLS such columns; reset when the scale
// changes.
inline float64 g_ui_need_w;
inline float64 g_ui_need_col_1;
inline float64 g_ui_cursor;
// Left edge and width rows lay out in: the body's, or the open group's padded inside.
inline float64 g_ui_area_x;
inline float64 g_ui_area_w;
// The open group's clip rect; rows outside it are neither drawn nor hot.
inline bool g_ui_in_group;
inline float64 g_ui_clip_x;
inline float64 g_ui_clip_y;
inline float64 g_ui_clip_w;
inline float64 g_ui_clip_h;
inline int32 g_ui_next_id;
// Widget that took the last press (window drag, slider drag, a button waiting for its release), 0 for none.
inline int32 g_ui_active;
inline float64 g_ui_drag_dx;
inline float64 g_ui_drag_dy;
inline vec2 g_ui_mouse;
inline bool g_ui_down;
// Edges of this frame, derived in ui_begin from the caller's button state. The first widget under the mouse takes
// the press.
inline bool g_ui_pressed;
inline bool g_ui_released;
inline bool g_ui_prev_down;
// Right button edge of this frame (opens a keybind's mode popup).
inline bool g_ui_rpressed;
inline bool g_ui_prev_rdown;
// This frame's wheel delta; a widget that uses it zeroes it so the body does not scroll as well.
inline float64 g_ui_scroll;
inline ui_row_t g_ui_row;
inline std::vector<ui_wrap_t> g_ui_wraps;
inline uint64 g_ui_wrap_n;
// Passed as the marks of a single-select popup, and as the icons of a popup without them.
inline std::vector<bool> g_ui_no_marks;
inline std::vector<std::string> g_ui_no_icons;
inline ui_icon_t g_ui_icon;

// --- hit testing and presses ---







// Never true under the open popup or colour picker, which are drawn over every row, nor outside the body or the open
// group.



// ui_hovered, also bounded to the chrome region (rx, ry, rw, rh) the widget belongs to.



// A hovered widget is lit unless another one holds the press.



// Takes this frame's press if the mouse is over the widget. True on the frame it was taken.



// Press-inside then release-inside, as ImGui buttons do. A press and release in one frame (a click shorter than a
// tick) counts too.



// Takes the buttons as they are now as the previous frame's, and drops any widget's press, so a button already held
// (the click that brought the game back, one made while the menu was hidden) is no press on the menu.



// --- layout ---




// Rounded to a whole pixel: a half-pixel origin smears a 1 px border.












// Records that the open group's row needs w pixels of content width to show without cutting anything.






// Records the row just drawn for ui_tip: x, y, h, where its label ends and how far a marker may reach.



// --- text ---










// The longest prefix of text that fits max_w pixels in font, mark ending a cut.



// ".." marks the cut: the default glyph range is not known to hold U+2026.






// Word-wraps text to w pixels into out (cleared first). " | " and "\n" also break the line, so a status string
// reads one part per line.



// One line of text (no "\n") word-wrapped to w pixels, appended to out; an empty line stays one empty line.



// --- primitives ---

// A filled, slightly rounded box with a 1 px border. The stroke is centred on the edge, so the half-pixel-inset rect
// lands it on whole pixels.



// Input frames (combo, key box, value box): darker when idle, lighter while hovered.



// On: an accent box with a dark check, ringed while hovered. Off: an empty field box, lighter while hovered.



// The faint band behind a hovered clickable row, reaching half a pad past its sides.



// A small solid triangle pointing up or down, centred on (cx, cy).



// The X of the close button and of a tag, as two lines: the default glyph range is not known to hold U+00D7.



// One panel of the map glyph: a convex quad, top-left, top-right, bottom-right, bottom-left.



// Glyph g (UI_GLYPH_*) centred on (cx, cy), s its half size, in c.



// The corner radius of the window's border (ui_gradient_border) and of the surfaces filled under it.



// A fill in the shape ui_gradient_border(x, y, w, h) strokes: the same rounded path half a pixel inside the edge, so
// the border's inner half covers its rim and nothing of it reaches past the border's outer edge. A fill of the whole
// rect rounds about other centres and pokes out at the corners. corners: the UI_CORNER_* mask, 0 for all.



// A 1 px gradient frame with the menu's corner rounding. ui_gfx_frame has no gradient stroke, so it is drawn in
// segments: the rounded outline clipped to the left column (edge and both corners) in the gradient's first stop, to the
// right column in its last, and the straight top and bottom edges between them running through the whole gradient.



// A scrollbar thumb centred in the gutter at gx, gw wide; thumb_grow wider while lit. Shared by the body, ui_multi_list
// and the popup.



// --- row icons ---

// The next ui_toggle / ui_checkbox_color / ui_text_button / ui_status_colored row starts with an icon slot holding key's
// icon. The slot is kept while there is no icon (or it is still loading), so the labels of one list stay aligned.



// ui_icon_next with a PNG path (relative to the scripts folder) rather than an icon key.



// An icon fitted to a combo field or popup item h high, so it grows with the row.



// Width the pending icon slot takes before the label, 0 when none is pending.



// An icon given as a PNG path (ends in .png) or an icon key, fitted into size x size at (x, y). False while there is
// nothing to draw (missing or still loading).



// Draws the pending icon in the row at x and clears it; returns where the label starts.
