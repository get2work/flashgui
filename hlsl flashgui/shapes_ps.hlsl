struct PS_INPUT
{
    float4 sv_position : SV_POSITION;
    float2 quad_pos    : TEXCOORD0; // [-0.5, -0.5] to [+0.5, +0.5] from unit quad
    float2 inst_pos    : TEXCOORD1; // shape_instance.pos
    float2 inst_size   : TEXCOORD2; // shape_instance.size
    float  inst_rot    : TEXCOORD3; // shape_instance.rotation
    float  inst_stroke : TEXCOORD4; // shape_instance.stroke_width
    float4 inst_clr    : TEXCOORD5; // shape_instance.clr (RGBA, 0..1)
    uint   inst_type   : TEXCOORD6; // shape_instance.shape_type
};

// Signed distance to axis-aligned box
float sdBox(float2 p, float2 halfSize)
{
    float2 d = abs(p) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

// Signed distance to circle
float sdCircle(float2 p, float radius)
{
    return length(p) - radius;
}

// Signed distance to line segment (capsule)
float sdLine(float2 p, float2 a, float2 b, float thickness)
{
    float2 pa = p - a, ba = b - a;
    float h = saturate(dot(pa, ba) / dot(ba, ba));
    return length(pa - ba * h) - thickness * 0.5;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 local = input.quad_pos;

    // Apply rotation for quads
    if (input.inst_type == 0 || input.inst_type == 1) {
        float s = sin(input.inst_rot), c = cos(input.inst_rot);
        local = float2(c * local.x - s * local.y, s * local.x + c * local.y);
    }

    float dist = 0.0;
    float alpha = 1.0;

    if (input.inst_type == 0) { // Quad
        dist = sdBox(local, input.inst_size);
        alpha = smoothstep(0.0, fwidth(dist), -dist);
    }
    else if (input.inst_type == 1) { // Quad outline
        dist = sdBox(local, input.inst_size);
        alpha = smoothstep(input.inst_stroke * 0.5, input.inst_stroke * 0.5 + fwidth(dist), abs(dist));
    }
    else if (input.inst_type == 2) { // Circle
        dist = sdCircle(local, input.inst_size.x);
        alpha = smoothstep(0.0, fwidth(dist), -dist);
    }
    else if (input.inst_type == 3) { // Circle outline
        dist = sdCircle(local, input.inst_size.x);
        alpha = smoothstep(input.inst_stroke * 0.5, input.inst_stroke * 0.5 + fwidth(dist), abs(dist));
    }
    else if (input.inst_type == 4) { // Line
        // pos = start, size = end
        dist = sdLine(local, input.inst_pos, input.inst_size, input.inst_stroke);
        alpha = smoothstep(0.0, fwidth(dist), -dist);
    }

    float4 outColor = input.inst_clr;
    outColor.a *= alpha;
    return outColor;
}