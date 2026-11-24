// Exposed function API. The host never touches a g_ui_* or g_kb_* global; everything it reads or writes goes through
// one of the functions here. Grouped by concept (theme, metrics, style, scale, fonts, window, input, keys, cursor,
// config, watermark). Kit-internal code keeps using the globals directly.
// --- theme (colours) ---






















































// Accent and the derived gradient.





// Host edits the accent through here; ui_sync_accent runs on the next ui_begin so a HUD drawn with the menu closed
// follows through its own ui_sync_accent call. The alpha is kept as-is: ui_sync_accent forces the gradient's own stops
// to 255 regardless.



// The whole theme in one go (a full skin). Followed by ui_sync_accent on the next frame.



// --- metrics (sizes, scaled) ---










































// --- style (unscaled ratios) ---












// --- scale ---








// --- fonts ---












// The bold font at a non-default size (ui_font_bold() is the menu font size). Made on first use and kept; ui_font_bold
// is also a valid fallback when the file does not load.



// --- window ---













// Save size restore: a negative pos default means "centre on the first ui_begin", so the host sets the saved size and
// the kit centres the window as the view is known.



// --- input state ---




















// This frame's tooltip at the mouse (a hovered mark on a free canvas), shown by ui_end like a row's.



// Called once per frame before any keybind polls; true = features may fire, false = menu open or game in background.





// Keys the capture refuses to bind (beyond the one a bind already holds). Fill before drawing the rows.





// --- layout (the row cursor, for custom draw regions like a free-canvas page) ---








// --- config (fonts, logo, subtitle, tooltip kinds) ---



















// --- watermark slot ---

// Anchor a custom watermark's rect for the next ui_begin to hit-test. interactive = the menu is open and this frame's
// ui_begin/ui_end already ran, so the next ui_begin takes the press on it; follow g_ui_wm.active via ui_active_id ==
// UI_ID_WATERMARK.








// The watermark's x / y as currently kept by the kit (useful for a keys-HUD that sits under it by default).
