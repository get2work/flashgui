// Sizes and shapes: ui_metrics_t (every size at 1.0, and g_ui_m scaled), ui_style_t (unscaled ratios and pixel
// offsets), ui_config_t (what the app sets: fonts, logo, mark, tooltip kinds), the GUI scale and the fonts.
// --- scale presets ---

// The GUI scales the menu offers, and the smallest window at each, in overlay pixels. Sliders, combos and multi combos
// sit under their label, so a column only needs its widest inline row (a checkbox with a key box); the widths are two
// col_min_w columns plus the sidebar, rounded up, and the heights keep the sidebar nav above the footer with room to
// spare. ui_min_w_at / ui_min_h_at also take the live measurement.
const float64 UI_MIN_W_1_25X = 700.0;
const float64 UI_MIN_H_1_25X = 450.0;
const float64 UI_MIN_W_1_5X = 840.0;
const float64 UI_MIN_H_1_5X = 540.0;
const float64 UI_MIN_W_2X = 1120.0;
const float64 UI_MIN_H_2X = 720.0;
const std::vector<float64> UI_SCALE_PRESETS = { 1.25, 1.5, 2.0 };
const std::vector<float64> UI_PRESET_MIN_W = { UI_MIN_W_1_25X, UI_MIN_W_1_5X, UI_MIN_W_2X };
const std::vector<float64> UI_PRESET_MIN_H = { UI_MIN_H_1_25X, UI_MIN_H_1_5X, UI_MIN_H_2X };
// Body columns the smallest window keeps room for (ui_min_w_at).
const int32 UI_MIN_COLS = 2;
// A scale edit is applied this long after it stops changing, or at once on release, so fonts are not rebuilt
// every frame of a drag.
const int64 UI_SCALE_SETTLE_MS = 250;
// Floor of the scale a measurement is divided by to store it per 1.0 of scale.
const float64 UI_SCALE_EPS = 0.01;
// The RT logo font's glyphs, "R" and "T" (ui_gfx_font_ranges): it is drawn large, so it bakes nothing else.
const std::vector<uint32> UI_RT_GLYPHS = { 0x52, 0x52, 0x54, 0x54, 0 };

// --- metrics ---

// Every size the menu draws with. g_ui_base holds the 1.0 values; ui_apply_scale writes them times the scale into g_ui_m.
struct ui_metrics_t {
    float64 pad = 8.0;
    float64 side_w = 96.0;
    float64 logo = 40.0;
    float64 head_h = 38.0;
    float64 nav_h = 30.0;
    float64 foot_h = 26.0;
    float64 row_h = 24.0;
    float64 frame_h = 18.0;
    // Combo and multi combo fields, taller than the other fields, and one item of a popup or an inline multi list.
    float64 combo_h = 24.0;
    float64 item_h = 28.0;
    float64 box = 14.0;
    // Colour swatch height.
    float64 toggle_h = 14.0;
    float64 knob_r = 5.0;
    float64 track_h = 4.0;
    // Shortest slider track a row is measured with (ui_need).
    float64 track_min_w = 70.0;
    float64 value_w = 44.0;
    float64 number_w = 88.0;
    float64 key_w = 44.0;
    // The shortest stub of a cut text a row is measured with (combo names, list keys, text inputs).
    float64 stub_w = 88.0;
    float64 swatch_w = 22.0;
    // Icon left of a list row's label (ui_icon_next).
    float64 icon = 16.0;
    float64 group_title_h = 24.0;
    float64 accent_w = 3.0;
    float64 underline_h = 2.0;
    float64 tab_pad = 10.0;
    float64 col_min_w = 210.0;
    // The resize grip's hit square, its dots and the gaps between them.
    float64 grip = 16.0;
    float64 grip_dot = 2.0;
    float64 grip_gap = 1.5;
    // Narrowest card of a card grid (ui_grid_begin); cards stretch to fill the row.
    float64 card_w = 96.0;
    float64 tip_r = 5.5;
    float64 tip_max_w = 260.0;
    float64 dot_r = 3.5;
    float64 wm_h = 20.0;
    // Watermark: info text, kept under the menu font so it stays out of the way; the chamfer cut off each corner, the
    // separator dot radius and the length of the bottom-edge sweep.
    float64 wm_font_info = 12.0;
    float64 wm_cut = 6.0;
    float64 wm_sep_r = 1.25;
    // The dot between the mark and the first part.
    float64 wm_dot_r = 2.0;
    // The card watermark's diamond, before ui_style_t's wm_diamond_x / wm_diamond_y.
    float64 wm_gem = 6.0;
    float64 wm_sweep_w = 44.0;
    float64 scroll_w = 3.0;
    float64 line_gap = 2.0;
    float64 round = 3.0;
    // Drawn strokes: check mark, logo ring, close X, picker knob (ui_stroke_w).
    float64 stroke = 1.5;
    float64 font = 13.0;
    float64 font_small = 10.0;
};

// --- style ---

// The shapes widgets draw with, never scaled: a ratio multiplies the size its comment names, "px" marks overlay pixels
// kept the same at every scale (insets, halos, pixel floors).
struct ui_style_t {
    // Text-only lines (a label over a field, a status line, a subheader), times row_h.
    float64 text_line = 0.8;
    // The thinnest bar drawn (px): scrollbars, slider tracks, grip dots, spinner rings.
    float64 bar_min = 2.0;
    // A lit scrollbar thumb is this much wider (px).
    float64 thumb_grow = 2.0;
    // How much narrower than its minimum a body column may be before the body drops a column (px).
    float64 col_slack = 2.0;
    // Check box: the hover ring's gap outside the box (px), the tick's stroke (times the stroke width, at least
    // check_stroke_min px) and the tick's three points, x and y as fractions of the box.
    float64 check_ring = 1.5;
    float64 check_stroke = 1.3;
    float64 check_stroke_min = 1.75;
    std::vector<float64> check_mark = { 0.25, 0.52, 0.42, 0.69, 0.75, 0.32 };

    // Sidebar head: the brand area's top and the gap under it, times pad; the brand area height, times logo; the RT
    // logo's height in it, times that height; the platform logo's fallback emblem dot, times the logo size.
    float64 side_logo_top = 1.5;
    float64 side_name_gap = 0.75;
    float64 side_brand = 1.3;
    float64 side_rt = 0.65;
    float64 logo_dot = 0.18;
    // RT logo (ui_draw_rt_logo), times the logo font's line height, measured on Segoe UI Black Italic
    // (g_ui_cfg.font_logo_path): the R's cap top under the line top, the T's drop and pen offset from the R's, and the
    // ink's left edge, width and height. The R stands a quarter of its height above the T, which sits right of it with a
    // small gap. rt_ink_w is the drawn width in line heights: rendering the pair measures 1.01 at the touching layout
    // (rt_tuck 0.4837, _scratch/ui2/logo_render_probe), and moving the T right is a rigid shift, so rt_ink_w = rt_tuck +
    // 0.526. At rt_tuck 0.55 the R-T gap is well under a quarter letter and the ink is 1.076 wide; the sidebar logo is
    // then side_rt of its area (42/51/68 px, uncapped) and both it and the watermark sit centred with no dead space
    // (_scratch/uilib/rt_metrics). Then the accent outline, times the logo height (at least 1 px).
    float64 rt_top = 0.2857;
    float64 rt_drop = 0.1215;
    float64 rt_tuck = 0.55;
    float64 rt_ink_x = 0.0113;
    float64 rt_ink_w = 1.076;
    float64 rt_ink_h = 0.6478;
    float64 rt_outline = 0.04;
    // Small sizes: the T is pushed right until o + 1 px stay clear between the two outlines (ui_rt_t_dx). rt_touch is
    // where the R and T inks touch (worst of the six sizes the kit draws, 0.43-0.46 of the line height) and rt_eat how
    // much of the ink gap the outlines take, times o; measured with _scratch/uilib/rt_gap (the reviewer's PIL method).
    float64 rt_touch = 0.46;
    float64 rt_eat = 1.5;
    // A nav item's label indent, times pad; the tightest tab padding, times tab_pad.
    float64 nav_indent = 1.75;
    float64 tab_pad_min = 0.3;
    // A top-bar button's square (ui_head_button), times head_h; its glyph's half size, times the square.
    float64 head_btn = 0.62;
    float64 head_glyph = 0.3;
    // A map overlay button's square (ui_tilemap_button), times row_h, and its glyph's half size, times the square;
    // the gap between those buttons and to the map's edge, times pad.
    float64 tile_btn = 1.15;
    float64 tile_glyph = 0.3;
    float64 tile_btn_gap = 0.75;
    // A map corner button's (ui_tilemap_corner_button) up chevron, its half width times the button's height.
    float64 tile_chevron = 0.16;
    // Group header, times its height: the accent bar's top and height, the chevron's centre from the right pad and its
    // half width.
    float64 group_bar_top = 0.28;
    float64 group_bar_h = 0.44;
    float64 group_chevron_x = 0.2;
    float64 group_chevron = 0.16;
    // Footer: the platform logo sits in the bottom-left corner with a gap equal to (foot_h - logo_size) / 2 on all four
    // sides (window left, window bottom, footer top, first indicator) - computed in ui_footer, no indent constant. The
    // other ratios are times pad: dot to text and item to item; the logo's size is times the footer height.
    float64 footer_dot_gap = 0.75;
    float64 footer_gap = 2.0;
    float64 foot_logo = 0.6;
    // Dots along each side of the resize grip's triangle.
    int32 grip_n = 3;
    // Tooltip: padding above and below the text, the header dot to name gap and the offset from the mouse, times pad;
    // corner radius, times round; the shadow's drop and spread (px).
    float64 tip_pad_y = 0.75;
    float64 tip_head_gap = 0.75;
    float64 tip_off_x = 1.5;
    float64 tip_off_y = 2.5;
    float64 tip_round = 1.5;
    float64 tip_shadow_drop = 1.0;
    float64 tip_shadow_spread = 1.0;

    // Badge and card tag pills: height over the text (px), side padding (times the pill's height).
    float64 badge_pad_y = 2.0;
    float64 badge_pad_x = 0.9;
    // Slider: the track's line under the label, times row_h, and the track's centre in it; the knob's growth while lit,
    // its dark rim and the drag halo (px).
    float64 slider_line = 0.6;
    float64 slider_track_y = 0.45;
    float64 knob_grow = 1.0;
    float64 knob_rim = 1.0;
    float64 knob_halo = 3.0;
    // The stepper's - and + half length, times the field height.
    float64 stepper_glyph = 0.22;
    // Typed edits: the caret's (and the first-key highlight's) inset from the box's top and bottom, and the room kept
    // right of the text for the caret (px).
    float64 caret_inset = 3.0;
    float64 caret_room = 2.0;
    // Combo field: the arrow's room and the chevron's half width, times frame_h (so the taller combo_h field keeps the
    // arrow of the other fields), and the chevron's nudge left (px); the popup's gap under the field (px); an icon in a
    // combo field or a popup item, times its height.
    float64 arrow_w = 0.7;
    float64 arrow = 0.14;
    float64 arrow_nudge = 2.0;
    float64 popup_gap = 2.0;
    float64 item_icon = 0.75;
    // The accent bar of a popup's current item: inset from the item's top and bottom (px).
    float64 mark_inset = 3.0;
    // Keybind rows, times pad: key box to check box, the check box's extra hit margin, and a pair's gap between halves.
    float64 key_gap = 0.75;
    float64 check_hit = 0.25;
    float64 pair_gap = 1.5;
    // An inline list's icon inset from its row's top and bottom (px).
    float64 list_icon_inset = 2.0;
    // Progress bar height, times pad, at least progress_min px.
    float64 progress_h = 0.6;
    float64 progress_min = 3.0;
    // Colour picker: the SV square, times row_h; the old / new preview, times the square; its checker cell, times the
    // field height; the smallest knob and alpha checker cell, and the bar marker's height (px).
    float64 picker_square = 6.5;
    float64 picker_preview = 0.4;
    float64 preview_cell = 0.25;
    float64 picker_knob_min = 3.0;
    float64 checker_min = 3.0;
    float64 marker_h = 3.0;
    // Card grid: gap and top margin, times pad; the loading spinner, times the picture (at least card_spin_min px); the
    // selected border (px).
    float64 card_gap = 0.75;
    float64 card_top = 0.25;
    float64 card_spin = 0.14;
    float64 card_spin_min = 6.0;
    float64 card_sel_w = 2.0;
    // Spinner: the arc's shortest length and its breathing range (degrees), the stroke (times the radius).
    float64 spin_arc_min = 50.0;
    float64 spin_arc_span = 200.0;
    float64 spin_w = 0.22;
    // Watermark: the RT logo's height, times wm_h; right end padding and the gaps between parts, times pad.
    float64 wm_rt = 0.6;
    float64 wm_edge = 1.25;
    float64 wm_gap = 0.75;
    // Card watermark: the diamond's half width and half height, times wm_gem; the room inside both ends, the diamond's
    // gap to the first part and the gap between parts, times pad.
    float64 wm_diamond_x = 0.55;
    float64 wm_diamond_y = 0.8;
    float64 wm_pad = 0.75;
    float64 wm_mark_gap = 0.5;
    float64 wm_part_gap = 1.0;
};

// --- config ---

// What the app may override before its first frame: fonts, its logo, the sidebar subtitle, and the tooltip kinds. The
// defaults below are this project's own (the brand and RT logo fonts, the footer platform logo, and the
// [READ]/[CLIENT]/[SERVER] tooltip kinds), so the kit looks right with no setup call. Paths are relative to the scripts
// folder, fonts absolute.
struct ui_config_t {
    std::string font_path = "C:/Windows/Fonts/segoeui.ttf";
    std::string font_bold_path = "C:/Windows/Fonts/segoeuib.ttf";
    // The brand name (sidebar head, watermark), "" for the bold font; the sidebar head draws it at brand_scale times
    // the menu font.
    std::string font_brand_path = "C:/Windows/Fonts/verdanab.ttf";
    float64 brand_scale = 1.3;
    // The RT logo's letters (ui_draw_rt_logo: the sidebar head and the watermark); the bold font when it cannot load.
    // The rt_* ratios of ui_style_t are measured on this font.
    std::string font_logo_path = "C:/Windows/Fonts/seguibli.ttf";
    // The footer's platform logo (built-in emblem when "").
    std::string logo_path = "assets/logo.png";
    // The sidebar subtitle under the brand; "" uses the window title's part after " - ".
    std::string subtitle = "";
    // A tooltip text that starts (exact case) with tip_prefixes[i] gets a header line tip_heads[i] in tip_cols[i]; the
    // prefix and the spaces or newlines after it are cut, the rest is the body. Other texts have no header.
    std::vector<std::string> tip_prefixes = { "[READ]", "[CLIENT]", "[SERVER]" };
    std::vector<std::string> tip_heads = { "Read-only", "Client-only", "Server-visible" };
    std::vector<color> tip_cols = { color(96, 165, 250, 255), color(53, 211, 153, 255), color(251, 191, 36, 255) };
};

// --- state ---

// Scales every size and the font; the menu edits g_ui_scale, layout and fonts follow g_ui_scale_applied, which
// ui_settle_scale catches up.
inline float64 g_ui_scale = 1.25;
inline float64 g_ui_scale_applied;
inline float64 g_ui_scale_seen;
inline int64 g_ui_scale_changed_at;
// Set when a scale change is applied: the next ui_begin moves the whole window back on screen, not only its head.
inline bool g_ui_refit;
inline ui_metrics_t g_ui_base;
inline ui_metrics_t g_ui_m;
inline ui_style_t g_ui_style;
inline ui_config_t g_ui_cfg;
// Fonts by whole-pixel size. ui_gfx_font handles are never released, so each size is created once and reused.
inline std::map<int64, int64> g_ui_fonts;
inline std::map<int64, int64> g_ui_fonts_bold;
inline std::map<int64, int64> g_ui_fonts_brand;
inline std::map<int64, int64> g_ui_fonts_logo;
inline int64 g_ui_font;
inline int64 g_ui_font_bold;
inline int64 g_ui_font_small;
// The brand name at the menu font size (watermark) and at g_ui_cfg.brand_scale (sidebar head).
inline int64 g_ui_font_brand;
inline int64 g_ui_font_brand_big;
// The watermark's font (wm_font_info).
inline int64 g_ui_font_wm;
// Line height of g_ui_font and the width of the bold capture dots "...", measured when the fonts change.
inline float64 g_ui_font_h;
inline float64 g_ui_dots_w;

// Sets the sidebar subtitle shown under the brand (overrides the window title's part after " - ").



// --- scale and fonts ---

// Width of drawn strokes (check mark, logo ring, close X, picker knob), never under 1 px.









// The RT logo's font at px, made on first use (each size the logo is drawn at); the bold font at px when it fails.



// Index of the UI_SCALE_PRESETS entry nearest s.






// Smallest window at scale s: the preset's pinned size, or more when UI_MIN_COLS body columns as wide as the widest
// group measured (ui_need) need it, so no row is cut, and the sidebar nav (as measured last frame) fits above the footer.









// Applies a g_ui_scale edit (snapped to a preset) once it has settled, never while a drag is held (the layout would
// move under it).
