// Colour: the picker popup (one open at a time) and the swatch rows that open it.
// --- constants ---

// Colour picker: the hue bar's stops (red round to red) and the two checkerboard shades behind alpha.
const std::vector<color> UI_HUE_STOPS = { color(255, 0, 0, 255), color(255, 255, 0, 255), color(0, 255, 0, 255),
    color(0, 255, 255, 255), color(0, 0, 255, 255), color(255, 0, 255, 255), color(255, 0, 0, 255) };
const std::vector<std::string> UI_CHANNEL_NAMES = { "R", "G", "B", "A" };
// Edit labels of the picker's hex box and its R G B A boxes.
const std::string UI_PICKER_HEX = "picker hex";
const std::vector<std::string> UI_PICKER_FIELDS = { "picker R", "picker G", "picker B", "picker A" };
// The accent gradient's dark stop: the default crimson (185, 28, 28)'s saturation and value, re-hued to the accent each
// frame (ui_sync_accent) and scaled down as far as the accent is paler or darker than the default red UI_GRAD_BASE, so
// a grey accent gets a grey stop. At the default red accent this reproduces the kit's original gradient exactly.
const color UI_GRAD_BASE = color(239, 68, 68, 255);
const float64 UI_GRAD_DARK_S = 0.8486;
const float64 UI_GRAD_DARK_V = 0.7255;

// --- types ---

// A checkerboard's cells as ui_gfx_fills data, rebuilt only when its rect or cell size changes.
struct ui_checker_t {
    float64 x = -1.0;
    float64 y;
    float64 w;
    float64 h;
    float64 cell;
    std::vector<float32> cells;
};

// The open colour picker (ui_color_picker, ui_checkbox_color). Drawn by ui_end over everything; its owning swatch row
// copies col into its colour when dirty and anchors it each frame (ui_picker_bind). open goes false on a press outside
// or Esc; label stays until the owner has taken the last edit.
struct ui_picker_t {
    std::string label;
    bool open;
    // The owner was drawn this frame; ui_picker_draw closes a picker whose row went away.
    bool seen;
    // Open at ui_begin: rows under its rect are not hot. drawing: its own parts are being hit-tested.
    bool live;
    bool drawing;
    // A press inside it, held from ui_begin to ui_picker_draw so nothing under it takes the press.
    bool press;
    bool esc_down;
    color col;
    // The colour when it opened, for the old half of the preview.
    color old;
    // Kept apart from col so hue and saturation survive grey and black.
    float64 hue;
    float64 sat;
    float64 val;
    bool dirty;
    // The hex box text and the colour it was made from.
    std::string hex;
    color hex_of;
    // The owner's swatch, and the popup rect as last drawn.
    float64 ax;
    float64 ay;
    float64 aw;
    float64 ah;
    float64 x;
    float64 y;
    float64 w;
    float64 h;
    ui_checker_t alpha_ck;
    ui_checker_t preview_ck;
};

// --- state ---

inline ui_picker_t g_ui_pk;

// --- colour math ---




// h, s and v in 0..1.



// The hue (0..1) of c; 0 for a grey. Mirrors ui_picker_set's hue extraction.



// The saturation and value (0..1) of c; saturation 0 for a grey.



// Re-tints the accent-derived theme colours from g_ui_theme.accent (the gradient used by the window border, the nav
// stripe, the tab underline, the watermark and the floating panels, and the watermark dot), so the accent picker drives
// every red in the UI from one place. Cheap, so it runs wherever those colours are drawn with the menu closed too: at the
// top of ui_begin, ui_hud_panel_faded and ui_watermark, and from the app after a config load and once per frame. Group
// strips, sliders, checks, keybinds and the popup scrollbar already read the accent directly; buttons read it through
// ui_col_accent / ui_col_accent_hi.






// RRGGBB or RRGGBBAA in any case, '#' optional (a text input cannot type it). RRGGBB keeps c's alpha. False when text is
// neither, leaving c as it was.



// --- picker ---




// Takes c as the picker's colour. The hue is kept while c is grey, the saturation while it is black.



// The cells of ck are rebuilt only when the rect or the cell size changes.






// A press on label's swatch: opens the picker on c (a click on the open one already closed it in ui_picker_input).



// Called by label's swatch row every frame: applies the picker's edit to c, follows a change made to c elsewhere (a
// config load, a Reset button) and anchors the picker to the swatch at (x, y, w, h). Closes it once the row has
// scrolled out of the visible body or group. True when c changed.



// The picker is drawn over everything, so it takes presses first: one inside is held for ui_picker_draw, one outside
// closes it and is used up, as a click off a popup. Esc closes it too, unless it ends a typed edit or a key capture.



// A position marker across a vertical bar at y: a light frame marker_h high reaching just past the bar's sides, inside
// a dark one 1 px larger all round.



// Value 0..1 of a press held on the vertical bar or square part id over (x, y, w, h); false while it is not held.



// The open picker, square, beside its swatch and kept on screen: saturation/value square, hue bar, alpha bar over a
// checkerboard, the old and new colour (a click on the old half reverts), the hex box and R G B A boxes. Every edit is
// applied to the bound colour by its row on the next frame (ui_picker_bind).



// --- swatch rows ---

// Swatch row: the label, the hex value and a swatch. A click on the row opens the colour picker beside the swatch.



// A toggle whose row also holds a colour swatch left of the check box, and after it a small button when button is not
// "" (clicked is set on its click). A click on the swatch opens the colour picker (keyed by label, as
// ui_color_picker); a click on the rest of the row flips v. True when v or c changed.
