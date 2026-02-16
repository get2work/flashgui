struct PS_INPUT
{
    float4 sv_position : SV_POSITION; // clip-space pixel position from the vertex shader
    float2 quad_pos : TEXCOORD0; // local coordinates of the pixel within the unit quad (0,0 to 1,1)
    float2 inst_pos : TEXCOORD1; // start position of the shape instance
    float2 inst_size : TEXCOORD2; // size of the shape instance (full extents for quads, radius for circles, endpos for lines)
    float inst_rot : TEXCOORD3; // rotation in radians for quads, ignored for circles/lines
    float inst_stroke : TEXCOORD4; // stroke width for lines, >0 for outline on filled shapes
    float4 inst_clr : TEXCOORD5; // RGBA color for the instance, used for both fill and stroke (alpha can be used to fade out)
    uint inst_type : TEXCOORD6; // shape type
    float4 inst_uv : TEXCOORD7; // UV coordinates for text/texture rendering (u0,v0,u1,v1) (min_x, min_y, max_x, max_y)
};

// Texture and sampler for text rendering
Texture2D font_tex : register(t0);
SamplerState font_samp : register(s0);

// Signed Distance Function for an axis-aligned box
// Top left corner at the origin and size defined by 'size'. 
float sd_box(float2 p, float2 size)
{
    float2 d = min(p, size - p);
    return min(d.x, d.y); // positive inside, zero at edge, negative outside
}
		
// Signed Distance Function for a circle centered at the origin with given radius
float sd_circle(float2 p, float radius)
{
    return length(p) - radius;
}

// Signed Distance Function for a line segment from start to end, with thickness.
// p is the pixel position in world space.
float sd_line(float2 p, float2 start, float2 end, float thickness)
{
	// vector from start to pixel, and from start to end
    float2 pa = p - start, ba = end - start;
			
	// squared length of the line segment
    float ba_dot = dot(ba, ba);

	// project pa onto ba, clamping to the line segment
	// This gives us the closest point on the line segment to p
    float h = ba_dot > 0.0 ? saturate(dot(pa, ba) / ba_dot) : 0.0;
			
	// Distance from p to the closest point on the line segment
	// -minus half the thickness = signed distance for a line with thickness.
    return length(pa - ba * h) - thickness * 0.5;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    // Prepare outputs: color (RGB) and alpha (A). We'll output premultiplied RGB.
    float3 out_rgb = input.inst_clr.rgb;
    float out_a = input.inst_clr.a;

	// Local pixel position within the shape instance, in pixels. 
    float2 local = input.quad_pos * input.inst_size;

    // Textured quad (glyph)
    if (input.inst_type == 5)
    {
        // Sample UV inside glyph rect
        float2 uv = float2(lerp(input.inst_uv.x, input.inst_uv.z, input.quad_pos.x),
                           lerp(input.inst_uv.y, input.inst_uv.w, input.quad_pos.y));
        float4 texel = font_tex.Sample(font_samp, uv);

        // texel.rgb carries ClearType per-channel coverage; texel.a may be 255 for our atlas.
        float3 coverage = saturate(texel.rgb);
        float avg_cov = (coverage.x + coverage.y + coverage.z) / 3.0f;

        // Final alpha (coverage * instance alpha)
        out_a = avg_cov * input.inst_clr.a;

        // Premultiplied RGB per-subpixel
        out_rgb = input.inst_clr.rgb * coverage * input.inst_clr.a;
    }
    else if (input.inst_type == 2)
    {
        // Filled circle
        float2 center = 0.5f * input.inst_size;
        float2 p = local - center;
        float dist = sd_circle(p, center.x);
        float alpha = smoothstep(0.0, fwidth(dist), -dist);
        out_a *= alpha;

        // premultiply RGB by resulting alpha
        out_rgb = input.inst_clr.rgb * out_a;
    }
    else if (input.inst_type == 3)
    {
        // Circle outline
        float2 center = 0.5f * input.inst_size;
        float2 p = local - center;
        float radius = center.x;
        float thickness = input.inst_stroke;

        float dist_outer = sd_circle(p, radius);
        float dist_inner = sd_circle(p, radius - thickness);
        float aa = fwidth(dist_outer);

        float alpha_outer = smoothstep(0.0, aa, -dist_outer);
        float alpha_inner = smoothstep(0.0, aa, -dist_inner);
        float alpha = alpha_outer - alpha_inner;
        out_a *= alpha;

        // premultiply RGB by resulting alpha
        out_rgb = input.inst_clr.rgb * out_a;
    }
    else if (input.inst_type == 0)
    {
        // Filled box
        float2 p = local;
        float dist = sd_box(p, input.inst_size);
        float alpha = smoothstep(0.0, fwidth(dist), dist);
        out_a *= alpha;

        out_rgb = input.inst_clr.rgb * out_a;
    }
    else if (input.inst_type == 1)
    {
        // Quad outline
        float2 p = local;
        float2 half_size = input.inst_size * 0.5;
        float2 d = abs(p - half_size) - half_size;
        float dist = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
        float edge = input.inst_stroke * 0.5;
        float alpha = smoothstep(edge, edge + fwidth(dist), abs(dist));
        alpha = 1.0f - alpha;
        out_a *= alpha;

        out_rgb = input.inst_clr.rgb * out_a;
    }
    else if (input.inst_type == 4)
    {
        // Line
        float dist = sd_line(input.sv_position.xy, input.inst_pos, input.inst_size, input.inst_stroke);
        float alpha = smoothstep(0.0, fwidth(dist), -dist);
        out_a *= alpha;

        out_rgb = input.inst_clr.rgb * out_a;
    }

    // Return premultiplied color
    return float4(out_rgb, out_a);
}