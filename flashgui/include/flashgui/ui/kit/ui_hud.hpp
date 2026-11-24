// Outside the window: draggable floating panels and the watermark bar.
// --- constants ---

// Default watermark gap from the top and right screen edges, in overlay pixels.
const float64 UI_WM_MARGIN = 10.0;
// The watermark's accent sweep crosses its bottom edge in UI_WM_SWEEP_MS, once every UI_WM_SWEEP_EVERY_MS.
const int64 UI_WM_SWEEP_MS = 1400;
const int64 UI_WM_SWEEP_EVERY_MS = 5000;
// Floating panel slots, numbered by the app. A later slot is drawn on top, so it takes a press first.
const int32 UI_FLOAT_SLOTS = 2;
// tan 22.5 degrees (sqrt 2 - 1): insetting a 45 degree cut by d moves its ends this times d along the straight edges.
const float64 UI_TAN_22_5 = 0.41421356;

// --- types ---

// The watermark's draw scratch, kept between frames so drawing it allocates nothing once warmed up: a shape's points
// (ui_gfx_polygon), the border's outer and inner octagons (ring) and the sweep's three gradient stops. The parts' sizes
// (pw, ph) and width w are kept while parts, font and metrics match the keys; the shapes and text spots while the bar's
// rect and cut match gx .. gcut (gw -1 forces a rebuild).
struct ui_wm_scratch_t {
    std::vector<float64> pw;
    std::vector<float64> ph;
    std::vector<float32> pts;
    std::vector<float32> ring;
    std::vector<color> sweep;
    std::vector<std::string> parts;
    int64 font = 0;
    float64 pad = -1.0;
    float64 mh;
    float64 msep;
    float64 mdot;
    float64 w;
    float64 gx;
    float64 gy;
    float64 gw = -1.0;
    float64 gh;
    float64 gcut;
    std::vector<float32> fill;
    std::vector<float32> q1;
    std::vector<float32> q3;
    std::vector<float32> q5;
    std::vector<float32> q7;
    vec2 top_p;
    vec2 top_s;
    vec2 bot_p;
    vec2 bot_s;
    std::vector<float32> sides;
    std::vector<color> side_cols;
    float64 dot_x;
    std::vector<float32> seps;
    std::vector<float64> tx;
    std::vector<float64> ty;
};

// --- state ---

inline ui_float_rect_t g_ui_wm;
inline ui_wm_scratch_t g_ui_wm_s;
// One rect per UI_FLOAT_SLOTS slot, grown by ui_float_drag_at; same use as g_ui_wm's (placed unused).
inline std::vector<ui_float_rect_t> g_ui_floats;
// ui_hud_panel_faded's gradient while it fades.
inline std::vector<color> g_ui_hud_grad;
// ui_watermark_card's part widths and the part strings / scale they were measured at, so a part is re-measured only
// when it or the scale changes (the fps and clock change about once a second).
inline std::vector<float64> g_ui_wm_card_pw;
inline std::vector<std::string> g_ui_wm_card_parts;
inline float64 g_ui_wm_card_scale = -1.0;
// Where the card watermark last drew its diamond (left edge) and each part, for layout checks.
inline float64 g_ui_wm_card_mark_x;
inline std::vector<float64> g_ui_wm_card_tx;

// --- floating panels ---

// Makes the w x h panel at (x, y) draggable with the left button while interactive (the menu is open and this frame's
// ui_begin/ui_end already ran), the watermark's way: the press is taken in the next ui_begin, before any widget, so it
// never reaches the menu (and, with the menu open, never the game). Moves x, y while dragged; true then.



// ui_float_drag for panel slot (0 .. UI_FLOAT_SLOTS - 1), so several panels can each be dragged.



// A floating panel's see-through box: the window background at g_ui_theme.panel, a faint edge and the gradient along
// the top.



// ui_hud_panel at opacity f; the gradient is only copied while f < 1.



// --- watermark ---




// The watermark's chamfered outline inset by d, into g_ui_wm_s.pts: 8 points clockwise from the top edge's left end.
// Insetting a 45 degree cut by d moves its ends d * (sqrt 2 - 1) further along the straight edges, so every inset of
// the shape keeps parallel edges and the fill (inset 1) stays under the 1 px border (inset 0.5) at any scale.



// A diamond of half width rx and half height ry about (cx, cy), into g_ui_wm_s.pts.



// Ring vertex i (0..7, octagon order) as a vec2: the outer octagon, or the inner one when inner.



// The band of ring edge i (vertex i to i + 1) as a quad into out, mitred at both ends on the shared vertices.



// The 1 px gradient frame of the chamfered bar: the band between the outer octagon (inset 0) and the fill's edge (inset
// 1), every piece cut from that one ring of 16 vertices so the pieces meet exactly. Left cuts in the gradient's first
// stop, right cuts in its last, each a mitred quad; then the four straight runs as pixel-aligned rects over the quads'
// mitre fringes, which hides the anti-aliased seams where the pieces join. Top and bottom runs go through the whole
// gradient and end in exactly those colours. Built into the scratch here (the fill too: it is the ring's inner
// octagon), drawn by ui_wm_border_draw.



// The side colours are set every frame, so a theme change shows without a rebuild.



// A short accent highlight sliding left to right along the bottom run, eased, then resting until the next period.
// Clipped to the straight run so it never spills over the cuts.



// A group header's fade (card_top down to clear) over the bar's fill, fitted to the chamfer: the top band is a gradient
// between the cuts plus two flat triangles in the cut corners, then a full-width gradient that reaches clear where the
// bottom cuts start. Four draws, no scratch.



// True while the cached part sizes still hold: the same parts, font and metrics as when they were measured.



// Measures parts into pw / ph and the bar's width w, and keys them; the shapes are rebuilt on the next draw.



// The bar's shapes, dots and text spots for the rect (x, y, w, h) and cut, keyed so they are only rebuilt on a move,
// a width change or a new scale.



// A slim chamfered bar: the RT logo, then parts (the first bright, the rest dim) split by tiny accent dots, and an
// accent sweep along the bottom edge now and then; a part starting with "~" (a fallback value) is drawn muted. Drawn
// whether or not the menu is open; with interactive true (the menu is open and this frame's ui_begin/ui_end already
// ran) a press on it drags it, taken in the next ui_begin. Kept on screen either way. Text sizes and shapes are
// cached (ui_wm_sizes_valid, ui_wm_layout), so a still frame only draws.



// The card watermark, styled like the selected sidebar tab: a plain see-through panel whose left edge is the accent
// gradient bar (square there, rounded on the right), an accent diamond, then parts (the first bold and bright, the rest
// dim) set apart by space alone; a part starting with "~" is drawn muted. Placed and dragged as ui_watermark is,
// through the same rect.
