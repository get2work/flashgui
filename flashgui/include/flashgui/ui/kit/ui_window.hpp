// The window: ui_begin / ui_end, the sidebar nav, the top tabs, the scrolling body of group boxes, the footer,
// the resize grip and the tooltip.
// --- constants ---

// The window's size until the app sets g_ui_w / g_ui_h (say, from its saved config).
const float64 UI_DEFAULT_W = 560.0;
const float64 UI_DEFAULT_H = 700.0;
// A tooltip shows once the mouse rests UI_TIP_DELAY_MS on its row and fades in over UI_TIP_FADE_MS. Moving straight on
// to another tipped row (no frame without a tip, or a gap under UI_TIP_GAP_MS for callers outside the menu) shows the
// next one at once.
const int64 UI_TIP_DELAY_MS = 350;
const int64 UI_TIP_FADE_MS = 90;
const int64 UI_TIP_GAP_MS = 120;

// --- types ---

// The footer logo. Its texture is asked of the icon provider on every draw and never kept: a provider may free one that
// has not been asked for in a while. No texture (no g_ui_cfg.logo_path, still loading, or a failed load) draws a plain
// emblem instead.
struct ui_logo_t {
    // The first load has settled (a failure is logged once).
    bool tried;
    // The square ui_footer last drew it in, and where its first indicator starts.
    float64 foot_x;
    float64 foot_y;
    float64 foot_s;
    float64 foot_next_x;
};

// The scrolling area under the top bar. Groups flow down cols columns; col_y is where the next group of each starts.
struct ui_body_t {
    bool live;
    int64 page;
    float64 x;
    float64 y;
    float64 w;
    float64 h;
    int32 cols;
    float64 col_w;
    std::vector<float64> col_y;
    float64 off;
};

// The open group box. Its background is drawn before its rows, so it uses last frame's height (g_ui_group_h).
struct ui_group_t {
    std::string title;
    bool open;
    int32 col;
    float64 x;
    float64 y;
    float64 w;
};

// The hovered tooltip of this frame, drawn once by ui_end. lines and w hold the wrap of wrapped at font, redone only
// when either changes.
struct ui_tooltip_t {
    std::string text;
    std::string wrapped;
    int64 font;
    float64 w;
    std::vector<std::string> lines;
    // g_ui_cfg.tip_prefixes index of the header, -1 for none; lines then hold the body only.
    int32 kind = -1;
    // When the current run of hovered frames started and the last frame with a tip, 0 while none.
    int64 since;
    int64 seen_at;
};

// --- state ---

// Top-left of the window in overlay pixels; negative until the first ui_begin() centres it.
inline float64 g_ui_x = -1.0;
inline float64 g_ui_y = -1.0;
// Current window size; kept across hide/show for the life of the script.
inline float64 g_ui_w = UI_DEFAULT_W;
inline float64 g_ui_h = UI_DEFAULT_H;
// Sidebar height the nav needs (head, last frame's items, footer), at the applied scale and per 1.0 of scale.
inline float64 g_ui_side_need;
inline float64 g_ui_side_need_1;
// Ids taken first in ui_begin, so they are the same every frame.
inline int32 g_ui_win_id;
inline int32 g_ui_grip_id;
// Reset to false each frame and no longer set (the close X was removed); ui_close_clicked() returns it, so callers that
// check it compile and simply never see a close.
inline bool g_ui_closed;
// Where the next ui_nav item goes, and the regions nav items and tabs are drawn and hit-tested in.
inline float64 g_ui_nav_y;
inline float64 g_ui_nav_top;
inline float64 g_ui_nav_bottom;
inline int32 g_ui_nav_n;
inline int32 g_ui_nav_last;
inline float64 g_ui_tabs_left;
inline float64 g_ui_tabs_right;
// Left edge of the last top-bar button placed (ui_head_button), the window's right edge before the first.
inline float64 g_ui_head_right;
inline std::vector<float64> g_ui_tab_w;
// Collapsed group boxes and each group's height last frame, by title.
inline std::map<std::string, bool> g_ui_group_closed;
inline std::map<std::string, float64> g_ui_group_h;
// Body scroll offset per page key (ui_body_begin).
inline std::map<int64, float64> g_ui_page_off;
inline ui_body_t g_ui_body;
inline ui_group_t g_ui_grp;
inline ui_tooltip_t g_ui_tip;
// The footer's platform logo (g_ui_cfg.logo_path).
inline ui_logo_t g_ui_logo;

// --- logo ---

// Polled until settled, to log a failed load once: the decode runs off the frame (ui_gfx_texture), since a texture load
// can fault.



// The logo fitted into a size x size square under top, centred on cx, unmodified (white tint).



// --- RT logo ---

// The outline of the RT logo drawn h pixels high.



// The T's pen offset from the R's at logo font size p with outline o: rt_tuck of the line height, or further right
// where that would leave fewer than o + 1 clear pixels between the two outlines (the watermark's small sizes).



// Width of the RT logo drawn h pixels high: the T's offset plus the T's ink right of its pen (rt_ink_w - rt_tuck).



// The red team mark as text: black glyphs (g_ui_cfg.font_logo_path) outlined in the accent, the R raised and the T
// dropped to its right with a small gap between them, the ink h pixels high with its top-left at (x, y). The sidebar
// head and the watermark both draw it, so they match and follow the accent picker.



// The sidebar brand: the RT logo side_rt of the box_h high area, at most box_w wide, centred on cx.



// --- frame ---

// down: the left button as read once this frame. tap: a whole click happened since the previous frame without the
// button being seen down (from the kernel toggle bit). rdown, rtap: the same for the right button. wheel: the wheel
// since the last frame, positive up (the caller picks a source the game does not hide). title's part after " - " is the
// dim subtitle under the sidebar's RT logo (g_ui_cfg.subtitle overrides it).



// A main tab in the sidebar, one under the other. The selected one gets the gradient bar on its left edge. True on
// the frame it was selected. Drawn and hit-tested only above the footer.



// A square top-bar button at (bx, by), side s: hover fill, the glyph (or icon) dim, text while hovered, accent and the
// page tab's underline while lit. True when clicked.



// A square button in the top bar, placed right to left from the window's right end in call order. Call after ui_begin
// and before ui_tabs, which then fit left of the buttons. icon: a PNG path or icon key drawn through the icon provider,
// else glyph (UI_GLYPH_*); lit: what it opens is open. True when clicked.



// A back arrow at the left end of the top bar, before the page tabs (call before ui_tabs). True when clicked.



// The page tabs in the top bar, left to right, label i selecting page i: text only, the active one bold with the
// gradient underline. They fit between the sidebar and the head buttons: the padding shrinks first, then the longest
// labels are cut to a shared width (water-filled) and show their full name as a tooltip. True on the frame one was
// selected.



// Gives a page a free canvas left of a narrower body: the body keeps its right side_w (for groups as usual) and the
// rest is returned as the canvas rect, which the caller draws and hit-tests itself. Call between ui_tabs and
// ui_body_begin.



// Starts the scrolling body for page (any key, one scroll offset each): cols columns, fewer when a column would be
// narrower than col_min_w.



// The body offset change for this frame's wheel notch: up (positive) scrolls towards the top.



// The wheel over the body scrolls it unless a widget used the notch. When the content overflows, a thin bar shows in
// the gap right of the body; pressing anywhere in that gap drags it (a press off the thumb centres the thumb there).



// The shown part of a group title: text after "##" only keys the group (collapse state, last height), so two pages can
// each have an "Advanced##x" group.



// A group box at the bottom of column col (taken modulo the body's columns): a header with an accent bar, the bold
// title and a chevron over the card-style fade; a click on the header collapses it (kept per title). True when open:
// the caller draws its rows then, and calls ui_group_end() either way.






// The bottom bar: the platform logo (g_ui_cfg.logo_path) in the bottom-left corner with equal gaps on all four sides
// (window left, window bottom, footer top line, first indicator), then one indicator per text, left to right, each a
// dot and its text in its colour (bold unless muted).



// The resize grip: a triangle of square dots (the lower-right half of a grip_n x grip_n grid) in the corner, whole
// pixels at every scale. Muted, dim while hovered, the accent while dragged. Its hit area is the grip square taken in
// ui_begin.






// Kept for callers (the close X was removed): g_ui_closed is never set now, so this is always false.



// --- tooltip ---

// Splits the header prefix (g_ui_cfg.tip_prefixes) off g_ui_tip.text and wraps the rest to tip_max_w; w is the widest
// of the body lines and the header (dot, gap, bold name).



// The hovered row's tooltip, below and right of the mouse, flipped left or above at a screen edge and kept on screen,
// after UI_TIP_DELAY_MS of hover. Rounded panel with a soft shadow; a prefixed tip (g_ui_cfg.tip_prefixes) starts with
// its coloured header over a tinted band and a hairline. Drawn after every hit test and never tested itself, so it takes
// no clicks. Relaid out only when the text or the font changes.
