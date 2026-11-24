inline color ui_col_accent() { return g_ui_theme.accent; }

inline color ui_col_accent_hi() { return g_ui_theme.accent.lerp(g_ui_theme.text, 0.45); }

inline color ui_fade(color c, float64 f) {
    c.a = static_cast<uint8>(static_cast<float64>(c.a) * ui_clamp(f, 0.0, 1.0));
    return c;
}
