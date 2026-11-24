// A tiled map viewport: the tiles of an image pyramid under a pan and zoom transform, a layer delegate for the host's
// marks over them, and square overlay buttons in its top-right corner. The host owns a ui_tilemap_t per map, fills in
// its extent, levels and the two delegates, and calls ui_tilemap(m, x, y, w, h) where the map goes: a view's canvas,
// or any rect outside the body's clip. A press drags the map, the wheel zooms about the mouse, a press and release
// without a drag sets m.clicked, a right press m.rclicked.
//
// Levels: at level z (z_min coarsest .. z_max finest) a tile_px square tile covers tile_px * 2^-z map units, tile
// (z, col, row) starting at map point (col, row) times that. Map units are the level-0 pixels the host's marks use. A
// level is drawn up to UI_TILE_MAGNIFY times magnified before the next finer one takes over. Each tile is a PNG
// (key_path_fn with the map's key, else path_fn; asked once per tile and kept) decoded by the picture workers
// (ui_pics.em) the first frame it shows, and freed UI_TILE_IDLE_MS after it was last drawn; until it loads, its part of
// the nearest loaded coarser tile stands in.
//
// One transform per frame: ui_tilemap fixes it once input has moved the view (draw_ox, draw_oy, draw_scale), and the
// tiles, the host's layer and every ui_tilemap_screen / ui_tilemap_point until the next ui_tilemap use it, so marks
// stay on their spot of the tiles whatever moves the window or the map.
// --- constants ---

const int64 UI_TILE_IDLE_MS = 20000;
// A level's tiles are drawn up to this much magnified before the next finer level is used.
const float64 UI_TILE_MAGNIFY = 1.19;
// Zoom per wheel notch, and how far a press moves before it is a drag rather than a click (px).
const float64 UI_TILE_ZOOM_STEP = 1.25;
const float64 UI_TILE_DRAG_PX = 4.0;

// --- types ---

// The PNG of tile (z, col, row), "" when there is none. Asked once per tile.
using UiTilePathFn = std::function<std::string(int32 z, int32 col, int32 row)>;
// The same for the map keyed key (ui_tilemap_t.key): one provider for every map a host shows.
using UiTileKeyPathFn = std::function<std::string(int64 key, int32 z, int32 col, int32 row)>;
// Draws the host's marks over the tiles, inside the map's clip; ui_tilemap_screen maps its points.
using UiTileLayerFn = std::function<void()>;
struct ui_tilemap_t {
    // Extent in map units, tile size in pixels and the levels on disk.
    float64 map_w = 1.0;
    float64 map_h = 1.0;
    float64 tile_px = 256.0;
    int32 z_min = 0;
    int32 z_max = 0;
    // Screen px per map unit allowed, and the fill under the tiles.
    float64 scale_min = 0.01;
    float64 scale_max = 4.0;
    color back = color(16, 19, 26, 255);
    UiTilePathFn path_fn = nullptr;
    // Instead of path_fn: the host's key for this map, and the provider asked with it.
    int64 key;
    UiTileKeyPathFn key_path_fn = nullptr;
    UiTileLayerFn layer_fn = nullptr;
    // The view: the map point at the canvas centre and the scale. While placed is false the next draw fits the map.
    bool placed;
    float64 cx;
    float64 cy;
    float64 scale = 1.0;
    // The transform this frame was drawn with: screen = (draw_ox, draw_oy) + map point * draw_scale. A zoom, look or
    // fit after ui_tilemap moves the map from the next frame on.
    float64 draw_ox;
    float64 draw_oy;
    float64 draw_scale = 1.0;
    // This frame: the canvas, the mouse over it and its map point, a click (left without a drag, or right) and where,
    // a drag in progress.
    float64 x;
    float64 y;
    float64 w;
    float64 h;
    bool hovered;
    float64 mouse_mx;
    float64 mouse_my;
    bool clicked;
    bool rclicked;
    vec2 click_at;
    bool dragging;
    // The level drawn, and its visible tiles: drawn, still loading, with no file.
    int32 level;
    int32 drawn;
    int32 loading;
    int32 missing;
    // Tile paths by ui_tile_key, "" for none.
    std::map<int64, std::string> paths;
    // Overlays over the map (x, y, w, h each, x and y from the map's top-left so they move with it): this frame's so far
    // and the last frame's, whose presses the map leaves to them. Overlay buttons stacked this frame down the top-right
    // corner, and the left edge of those along the bottom-right one (x + w for none).
    std::vector<float64> holes;
    std::vector<float64> holes_last;
    int32 buttons;
    float64 corner_left;
    // The press: where, and the view centre then.
    vec2 press_at;
    float64 press_cx;
    float64 press_cy;
    int32 id;
};

// --- transform ---

// Screen point of map point (mx, my), as this frame's map was drawn.



// Map point under screen point (sx, sy), as this frame's map was drawn.



// The frame's transform from the view and the canvas, once input has moved the view.



// Centres the view on map point (mx, my); scale > 0 sets the zoom too.



// Zooms by f about screen point (sx, sy): the map point under it stays put. It works on the view rather than the drawn
// transform, so two zooms in one frame both keep their point.



// The whole map in the canvas.



// --- tiles ---







// Top-left of tile (z, col, row) on the screen as this frame draws it. A tile ends where its right and lower neighbours
// start, so they meet without a gap or an overlap. Not rounded to whole pixels: the host's marks use the same transform,
// and a rounded tile moved against them by up to a pixel as the map panned.



// The level for m's scale: the coarsest one magnified at most UI_TILE_MAGNIFY.



// A tile not loaded yet: its part of the nearest loaded coarser tile. The coarsest one is asked for, so there is always
// one to fall back on once it loads.



// The visible tiles of level z.



// --- widget ---

// The mouse over one of last frame's overlays, at their place on the map as it is now.



// Something the host draws over the map this frame (a chip, a button) at screen x, y: the next frame's map leaves presses
// there to it.



// Keeps the view centre on the map.



// The map in the rect x, y, w, h: input (drag, wheel, clicks), the tiles, the host's layer, a 1 px frame.



// A square button over the map's top-right corner, stacked downwards in call order after ui_tilemap: glyph
// (UI_GLYPH_*), its tooltip, lit drawn in the accent. True when clicked.



// A text button over the map's bottom-right corner, stacked leftwards in call order after ui_tilemap and its
// ui_tilemap_button calls: text (cut to half the map's width) and an up chevron for a list it opens above itself, open
// lighting it in the accent, tip its tooltip. top_right is set to its top-right corner, where such a list ends. Not drawn
// where it would meet the top-right stack or the map's left edge. True when clicked.
