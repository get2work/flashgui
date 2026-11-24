// Card grid: square pictures (ui_pics.em or icon keys) over a title, for pick-one lists.
// --- constants ---

const int64 UI_SPIN_MS = 900;
const int64 UI_SPIN_PULSE_MS = 1800;

// --- types ---

// The open card grid (ui_grid_begin .. ui_grid_end): as many columns of at least card_w as fit, stretched to the row.
struct ui_grid_t {
    float64 x;
    float64 y;
    float64 cw;
    float64 ch;
    float64 gap;
    int32 cols;
    int32 n;
};

// --- state ---

inline ui_grid_t g_ui_grid;

// --- card grid ---

// A loading spinner centred on (cx, cy): a faint ring under an accent arc that turns and breathes.









// A card's picture: a PNG path through ui_pic, else an icon key (loading until its texture settles).



// The next card of the grid: a square picture (ui_card_picture; a spinner while it loads, the title's first letter
// while it is missing) over its title, tag in a pill on the picture's top-left corner, and when selected the accent border and a check.
// Pictures are only asked for while the card is in view. Disabled, it is dimmed and inert. True on a click.
