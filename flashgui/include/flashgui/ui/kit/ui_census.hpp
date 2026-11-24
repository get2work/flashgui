// Kit side of the zero-leak census (core/mem_census.em). Generic byte estimators shared with the project census, plus
// ui_kit_census / ui_kit_free_all over every g_ui_* container except g_ui_pics (reported on its own, ui_pic_count /
// ui_pic_bytes). Kept in the kit so no project file reads g_ui_* internals.
//
// Byte estimates are approximate: a vector is capacity * sizeof(element); a map / set adds ~48 bytes node overhead per
// entry; string KEYS and VALUES add their content; but a string stored inside a struct ELEMENT of a vector/map is
// counted only as that struct's sizeof (short names undercounted). Font textures are owned by the render API and never
// freed, so g_ui_fonts* are a distinct-size count, not freeable bytes. Good enough to match an MB climb to an owner.
// --- generic byte estimators (used by the project census and the map / icon accessors too) ---




// Releases a vector's capacity (clear() keeps it); used by the unload frees so a cleared vector reports 0 bytes.
























// --- kit census ---

// Distinct font sizes created (g_ui_fonts*). The render API caches fonts process-wide and never frees them.



// Entries and estimated bytes across every g_ui_* container except g_ui_pics. The id / label-keyed caches
// (g_ui_text_boxes, g_ui_group_*, g_ui_page_off, g_ui_pop_tops, g_ui_list_top, g_ui_key_text) are keyed by compile-time
// widget ids, so they are bounded by the number of distinct widgets the menu ever shows; the rest are per-frame buffers.



// Clears every g_ui_* container at unload (plus the pictures), so the kit lets go of its heap even if the VM keeps it.
