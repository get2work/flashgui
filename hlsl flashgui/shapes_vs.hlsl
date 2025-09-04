cbuffer TransformCB : register(b0)
{
    float4x4 projection_matrix;
};

struct VS_INPUT
{
    float2 quad_pos   : POSITION;   // [-0.5, -0.5] to [+0.5, +0.5] (unit quad corners)
    // Instance data (from IA slot 1)
    float2 inst_pos   : TEXCOORD1;  // shape_instance.pos
    float2 inst_size  : TEXCOORD2;  // shape_instance.size
    float  inst_rot   : TEXCOORD3;  // shape_instance.rotation
    float  inst_stroke: TEXCOORD4;  // shape_instance.stroke_width
    float4 inst_clr   : TEXCOORD5;  // shape_instance.clr
    uint   inst_type  : TEXCOORD6;  // shape_instance.shape_type
};

struct VS_OUTPUT
{
    float4 sv_position : SV_POSITION;
    float2 quad_pos    : TEXCOORD0;
    float2 inst_pos    : TEXCOORD1;
    float2 inst_size   : TEXCOORD2;
    float  inst_rot    : TEXCOORD3;
    float  inst_stroke : TEXCOORD4;
    float4 inst_clr    : TEXCOORD5;
    uint   inst_type   : TEXCOORD6;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    // Apply rotation and scale to quad_pos
    float2 local = input.quad_pos;
    if (input.inst_rot != 0.0f)
    {
        float s = sin(input.inst_rot), c = cos(input.inst_rot);
        local = float2(c * local.x - s * local.y, s * local.x + c * local.y);
    }
    local *= input.inst_size;

    // World position
    float2 world_pos = input.inst_pos + local;
    float4 pos = float4(world_pos, 0.0f, 1.0f);

    output.sv_position = mul(projection_matrix, pos);

    // Pass through all instance data and quad_pos for SDF evaluation in PS
    output.quad_pos    = input.quad_pos;
    output.inst_pos    = input.inst_pos;
    output.inst_size   = input.inst_size;
    output.inst_rot    = input.inst_rot;
    output.inst_stroke = input.inst_stroke;
    output.inst_clr    = input.inst_clr;
    output.inst_type   = input.inst_type;

    return output;
}