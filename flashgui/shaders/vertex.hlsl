cbuffer TransformCB : register(b0)
{
	// Projection matrix that maps world / screen positions into clip space
    float4x4 projection_matrix;
};

// Input from the IA / vertex buffer / instancing data
struct VS_INPUT
{
    float2 quad_pos : POSITION; // [0,0] to [1,1] local coordinate on the unit quad
    float2 inst_pos : TEXCOORD1; // start position of the shape in world / screen space
    float2 inst_size : TEXCOORD2; // full extents (width, height) for the instance
    float inst_rot : TEXCOORD3; // rotation in radians; used for quads when inst_type >= 2
    float inst_stroke : TEXCOORD4; // stroke width for outlines and lines
    float4 inst_clr : TEXCOORD5; // RGBA color for the instance (fill/tint)
    uint inst_type : TEXCOORD6; // shape type (0=box, 1=box outline, 2=circle, 3=circle outline, 4=line, 5=textured quad)
    float4 inst_uv : TEXCOORD7; // texture UV rectangle (u0,v0,u1,v1) for textured quads
};

			// Output sent to the rasterizer and pixel shader
struct VS_OUTPUT
{
    float4 sv_position : SV_POSITION; // clip space position to be interpolated
    float2 quad_pos : TEXCOORD0; // [0,0] to [1,1] across the quad (for SDF / UV in PS)
    float2 inst_pos : TEXCOORD1; // instance start position (screen origin)
    float2 inst_size : TEXCOORD2; // full extents of the instance (for SDF / size)
    float inst_rot : TEXCOORD3; // radians of rotation, passed through
    float inst_stroke : TEXCOORD4; // stroke width, for outlines / lines
    float4 inst_clr : TEXCOORD5; // RGBA tint, passed to pixel shader
    uint inst_type : TEXCOORD6; // shape type, used in pixel shader branches
    float4 inst_uv : TEXCOORD7; // UV rectangle for text / texture sampling
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

	// Convert [0,1]x[0,1] quad_pos to local pixel offset within the instance
    float2 local = input.quad_pos * input.inst_size;

	// Compute world / screen position of this corner
    float2 world_pos = input.inst_pos + local;

	// Optional in-place rotation about the instance center for shapes like quads
	// Only applied when explicitly requested and for types that should rotate (e.g., not raw lines)
    if (input.inst_rot != 0.0f && input.inst_type >= 2) // circles, boxes, etc.
    {
		// Center of the instance in screen space
        float2 center = input.inst_pos + 0.5f * input.inst_size;

		// Vector from center to current point
        float2 pos_rel = world_pos - center;

		// Compute sin / cos of the rotation
        float s = sin(input.inst_rot), c = cos(input.inst_rot);

		// Apply 2D rotation matrix to pos_rel
        pos_rel = float2(c * pos_rel.x - s * pos_rel.y,
									 s * pos_rel.x + c * pos_rel.y);

		// Transform back into world space with center restored
        world_pos = center + pos_rel;
    }

	// Promote to 4D homogeneous clip space position; z=0, w=1
    float4 pos = float4(world_pos, 0.0f, 1.0f);

	// Transform by projection to get final screen clip position
    output.sv_position = mul(projection_matrix, pos);

	// Pass all per-instance data to the pixel shader for SDF / text / etc.
    output.quad_pos = input.quad_pos; // [0,0] to [1,1]
    output.inst_pos = input.inst_pos; // instance origin in screen space
    output.inst_size = input.inst_size; // full width/height
    output.inst_rot = input.inst_rot; // radians
    output.inst_stroke = input.inst_stroke; // stroke / outline width
    output.inst_clr = input.inst_clr; // RGBA tint
    output.inst_type = input.inst_type; // shape type selector
    output.inst_uv = input.inst_uv; // UV rect for text / textures

    return output;
}