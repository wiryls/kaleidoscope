cbuffer regular_triangle_group : register(b0)
{
    float top_x;
    float top_y;
    float length;
};

struct vout
{
    float2 tex : TEXCOORD;
    float4 pos : SV_POSITION;
};

vout main(float2 pos : POS, float2 tex : TEX)
{
    vout value;
    value.tex = tex;
    value.pos = float4(pos, 0.0, 1.0);
    return value;
}
