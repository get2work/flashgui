// Rows: labels, tooltips, badges, status and text lines, toggles, sliders, steppers, buttons and the text box.
// --- constants ---

// A text box scrolls this many lines per wheel notch.
const int32 UI_TEXT_SCROLL_LINES = 3;
// A text box re-wraps to a new width once the width has held this long: a long text takes tens of ms to wrap, too much
// for every frame of a window-resize drag.
const int64 UI_TEXT_REWRAP_MS = 150;

// --- types ---

// A ui_text_box's text word-wrapped to w at font (ui_text_box_wrap), and its first shown line.
struct ui_text_box_t {
    std::string text;
    float64 w = -1.0;
    int64 font;
    std::vector<std::string> lines;
    int32 top;
    // The width asked for last, and since when.
    float64 want_w = -1.0;
    int64 want_at;
};

// --- state ---

// The slider fill's two gradient stops, rewritten in place each draw.
inline std::vector<color> g_ui_fill_grad;
// Each ui_text_box's wrap and scroll, by id.
inline std::map<std::string, ui_text_box_t> g_ui_text_boxes;

// --- labels and text rows ---

// A row's label after the pending icon slot, cut with ".." to max_w; records the row for ui_tip.



// The label line over a field row (combo, multi combo): draws the label, records the row for ui_tip and returns where
// the field row starts.



// A "?" after the last row's label; hovering it or the label shows text. The marker is left out when it would
// reach the widget, the label still shows the tooltip.



// A small rounded pill after the last row's label, such as a risk tag ("MED", "RISKY"): text in c on a faint
// tint of c with a thin c outline. Call after the row's widget, before its ui_tip, so the "?" follows the pill. Left
// out where it would reach the widget (the width is still recorded with ui_need). True when drawn.



// A quiet section label inside a group: small dim text and a hairline to the right edge, in place of ui_separator
// where the rows below need a name.



// One compact line in c: the first " | " part of text, cut to the width; hovering it shows all of text, one part per
// line, when anything was left out.






// A bold line of text, for a sub-section inside a group.






// Word-wrapped to the content width; ui_gfx_text has no wrap limit of its own. The wrap is kept per call slot of the
// frame (g_ui_wraps) and redone only when the text, the width or the font (scale) changes.






// A thin bar at (x, y, w, h) filled to frac (0..1) in the accent colour.



// A row: text, then a bar filled to frac under it.



// --- toggles ---

// A row with a check box at its right end; a click anywhere on the row flips v. note, when not empty, is drawn dim
// right-aligned before the box (a count such as "12/66").






// One row of check boxes side by side in equal cells, each its label with the box at the cell's right end, as
// ui_toggle; tips[i], when not empty, is cell i's tooltip. The index of the box flipped, -1 for none.



// --- values ---

// v kept in [lo, hi] on the step grid from lo.



// Decimals that show every value on the step grid exactly (0.5 -> 1, 0.25 -> 2), at most 3; 2 for a free value.






// A slider track from x to x + w centred on cy, the knob at the value: press and hold to drag, or scroll over it
// for one step per notch (the same notch also zooms the game camera: overlay input is not consumed).



// Two lines: the label with the value in a small box at its right end (click it to type a value), then the track
// across the whole row.



// A stepper box at the right end: "-", the value, "+". A click on a side or a notch over the box moves one step, and
// a click on the value types one; kept in [lo, hi] on the step grid from lo.



// --- buttons ---

// A full-width compact button with the label centred; true on a click (press and release inside it).



// One row of equal-width buttons side by side; the index of the one clicked, -1 for none.



// A compact button at (x, y, w, h), muted and inert while disabled; true on a click.



// A text row ending in a small button sized to its caption; true on a click on the button while enabled. The text is
// cut before the button.



// --- text box ---

// Keeps tb's wrap of text at width tw: at once for a new text or font, for a new width once it has held for
// UI_TEXT_REWRAP_MS, so a resize drag shows the old wrap (clipped) instead of re-wrapping every frame.



// A text box's first line after this frame's wheel notch, over it when over. The notch is used up only while the text
// can move that way, so at either end it goes on to scroll the body.



// A read-only box rows lines high holding text, word-wrapped to its width ("\n" starts a line, " | " stays as written);
// hint shows muted while text is empty. The wheel over it scrolls UI_TEXT_SCROLL_LINES lines per notch, and the body
// once it is at that end; a thin accent thumb at its right edge shows where it is and drags it. A new text starts at its
// first line.
