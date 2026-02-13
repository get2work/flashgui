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
float sd_box(float2 p, float2 size) {
    float2 d = min(p, size - p);
    return min(d.x, d.y); // positive inside, zero at edge, negative outside
}
		
// Signed Distance Function for a circle centered at the origin with given radius
float sd_circle(float2 p, float radius) {
    return length(p) - radius;
}

// Signed Distance Function for a line segment from start to end, with thickness.
// p is the pixel position in world space.
float sd_line(float2 p, float2 start, float2 end, float thickness) {
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

float4 PSMain(PS_INPUT input) : SV_TARGET {
    float4 out_color = input.inst_clr;
	// Local pixel position within the shape instance, in pixels. 
	// For quads, this goes from (0,0) to (size.x, size.y). 
	// For circles, this goes from (0,0) to (radius,radius).
	// For lines, this is the pixel position relative to the start point.
    float2 local = input.quad_pos * input.inst_size;

				// Textured quad
    if (input.inst_type == 5)
    {

		// Sample the font texture using inst_uv and quad_pos to get the glyph alpha
		// inst_uv contains the UV rect for the glyph in the atlas: (u0,v0,u1,v1)
		// quad_pos goes from (0,0) to (1,1) across the quad, so we can lerp between u0 and u1, v0 and v1
        float2 uv = float2(lerp(input.inst_uv.x, input.inst_uv.z, input.quad_pos.x),
										 lerp(input.inst_uv.y, input.inst_uv.w, input.quad_pos.y));
				
		// Sample the font texture to get the glyph alpha
        float4 texel = font_tex.Sample(font_samp, uv);

		// Multiply the sampled alpha by the instance color alpha to get final alpha
        float alpha = texel.a * input.inst_clr.a;
 
		// Tint the glyph using inst_clr.rgb
        float3 rgb = input.inst_clr.rgb * texel.a;
                
		// Output the final color with the glyph alpha applied
        out_color = float4(rgb, alpha);
    }
    else if (input.inst_type == 2)
    {
		// Circle/outline, centered
		// For circles, local is the pixel position relative to the top-left of the bounding box. We want it relative to the center for the SDF.
        float2 center = 0.5f * input.inst_size;
				
		// SDF for circle centered at origin, radius = center.x
        float2 p = local - center;
				
		// For filled circle, alpha is smoothstep based on distance to edge. For outline, we will do a band based on stroke width.
        float dist = sd_circle(p, center.x);

		// For filled circles, we want alpha to be 1 inside the circle and fade out at the edge. For outlines, we will handle that in the next case.
        float alpha = smoothstep(0.0, fwidth(dist), -dist);
        out_color.a = alpha;
    }
    else if (input.inst_type == 3)
    {
		//circle outline

		// we want to create a band for the outline based on the stroke width.
        float2 center = 0.5f * input.inst_size;
        float2 p = local - center; // pixel position relative to circle center
        float radius = center.x;
        float thickness = input.inst_stroke;

		// Calculate SDF for outer edge (radius) and inner edge (radius - thickness)
        float dist_outer = sd_circle(p, radius);
        float dist_inner = sd_circle(p, radius - thickness);
        float aa = fwidth(dist_outer);

		// Use smoothstep to create a band for the outline.
		// The alpha will be 1 between the inner and outer edges, and fade out outside of that.
        float alpha_outer = smoothstep(0.0, aa, -dist_outer);
        float alpha_inner = smoothstep(0.0, aa, -dist_inner);
		// final alpha is the difference between the outer and inner alpha, creates a ring for the outline.
        float alpha = alpha_outer - alpha_inner;
        out_color.a *= alpha;
    }
    else if (input.inst_type == 0)
    {
		// Box/outline, not centered
        float2 p = local;
		// For filled box, alpha is smoothstep based on distance to edge
		// For outline, we will do a band based on stroke width in the next case. 
        float dist = sd_box(p, input.inst_size);
        float alpha = (input.inst_type == 0)
						? smoothstep(0.0, fwidth(dist), dist)
						: smoothstep(input.inst_stroke * 0.5, input.inst_stroke * 0.5 + fwidth(dist), abs(dist));
        out_color.a *= alpha;
    }
    else if (input.inst_type == 1)
    {
		// Quad outline
        float2 p = local;
		// create a band for the outline based on the stroke width. 
		// The SDF gives us the distance to the edge, so we can use that to create a band.
			
        float2 half_size = input.inst_size * 0.5;
		// distance from the center, minus half the size gives us a sdf to the edge
        float2 d = abs(p - half_size) - half_size;
		// length of the positive part of d gives us the dist to the edge.
        float dist = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
		// create a band for the outline based on the stroke width
        float edge = input.inst_stroke * 0.5;
		// alpha is 1 in the band around the edge, and fades out outside of that. 
		// We abs(dist) to create a band on both sides of the edge for the outline.
        float alpha = smoothstep(edge, edge + fwidth(dist), abs(dist));
        alpha = 1.0 - alpha; // Only the shell is visible
        out_color.a *= alpha;
    }
    else if (input.inst_type == 4)
    {
		// Line: input.inst_pos to (size.x, size.y)
		// use line SDF to create a band based on stroke width.
        float dist = sd_line(input.sv_position.xy, input.inst_pos, input.inst_size, input.inst_stroke);
        float alpha = smoothstep(0.0, fwidth(dist), -dist);
        out_color.a *= alpha;
    }

    return out_color;
}