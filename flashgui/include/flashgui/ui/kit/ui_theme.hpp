// Colours: ui_theme_t holds every colour the kit draws with and the alpha factors of its soft variants. g_ui_theme is
// the live theme; assign a whole ui_theme_t to change the look. accent is also edited at runtime (the menu's colour
// picker, saved in the config).
// Fixed colours, not part of a theme: an untinted image, the SV square's black, fully clear.
const color UI_WHITE = color(255, 255, 255, 255);
const color UI_BLACK = color(0, 0, 0, 255);
const color UI_CLEAR = color(0, 0, 0, 0);

struct ui_theme_t {
    // Surfaces: the window; the sidebar, fields and tooltip; group boxes and the selected nav item; hover and pressed.
    color bg = color(9, 11, 16, 255);
    color bg2 = color(16, 19, 26, 255);
    color group = color(18, 22, 32, 255);
    color hover = color(26, 31, 42, 255);
    color pressed = color(33, 40, 54, 255);
    // Top of the card picture gradient; group headers and the watermark fade from it too, so they read as one family.
    color card_top = color(46, 54, 76, 255);
    color border = color(37, 43, 56, 255);
    color border_hi = color(55, 63, 80, 255);
    // Text: primary, secondary, muted.
    color text = color(232, 236, 244, 255);
    color dim = color(146, 155, 174, 255);
    color muted = color(89, 98, 115, 255);
    // Slider knob at rest, between dim and text; text while hovered.
    color knob = color(198, 204, 216, 255);
    // Brand hues: red is the default accent; orange marks key capture and the close X (orange_hi hovered); crimson
    // outlines buttons (crimson_hi hovered).
    color teal = color(239, 68, 68, 255);
    color purple = color(249, 115, 22, 255);
    color purple_hi = color(253, 186, 116, 255);
    color indigo = color(220, 38, 38, 255);
    color indigo_hi = color(252, 165, 165, 255);
    // Status: on or done, warning, refused or failed.
    color active = color(53, 211, 153, 255);
    color warn = color(250, 204, 21, 255);
    color deny = color(248, 113, 113, 255);
    // Floating panels and the watermark: the window background, see-through (the Render API has no backdrop blur).
    color panel = color(9, 11, 16, 196);
    color panel_edge = color(55, 63, 80, 160);
    // The watermark's dot after its mark.
    color wm_dot = color(239, 68, 68, 255);
    // The colour picker's checkerboard behind alpha.
    color check_lo = color(70, 74, 84, 255);
    color check_hi = color(120, 125, 136, 255);
    // Under the tooltip, twice (spread, then tight).
    color shadow = color(0, 0, 0, 90);
    // Toggles, slider fills, focus borders, group accent bars, the popup's current item.
    color accent = color(239, 68, 68, 255);
    // Red gradient for the window and watermark borders, the selected nav bar, the active tab underline and the
    // floating panels' top edge; the first and last stops are also the borders' solid ends.
    std::vector<color> gradient = { color(239, 68, 68, 255), color(185, 28, 28, 255) };

    // Soft variants. A factor (0..1) multiplies a colour's alpha (ui_fade); an alpha (0..255) replaces it.
    // The accent ring round a hovered, ticked box; the band behind a hovered clickable row.
    float64 check_ring_a = 0.45;
    float64 row_hover_a = 0.7;
    // card_top's alpha at the top of a group header, idle and hovered; the watermark's fill fade starts at head_a too.
    uint8 head_a = 110;
    uint8 head_a_lit = 200;
    // A badge pill's fill and outline, of its colour.
    float64 badge_fill_a = 0.14;
    float64 badge_edge_a = 0.6;
    // A slider fill starts at its colour mixed this far towards bg2; a dragged knob's halo, of the fill.
    float64 fill_shade = 0.45;
    float64 knob_halo_a = 0.25;
    // The tooltip's spread shadow (the tight one is full shadow) and its header band, of the kind's colour.
    float64 tip_shadow_a = 0.5;
    float64 tip_band_a = 0.09;
    // A popup's scrollbar thumb at rest, of the accent.
    float64 thumb_idle_a = 0.6;
    // The faint ring under the spinner's accent arc.
    uint8 spin_ring_a = 45;
    // The watermark sweep: the bright stroke is the accent mixed this far towards text, the second stroke this alpha;
    // the separator dots, of the accent.
    float64 wm_sweep_mix = 0.35;
    uint8 wm_sweep_a = 70;
    uint8 wm_sep_a = 170;
};

inline ui_theme_t g_ui_theme;




// The accent lightened toward the text colour: a hovered accent outline (buttons).



// c with its alpha times f (0..1).
