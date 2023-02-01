struct vout
{
    float2 tex : TEXCOORD;
    float4 pos : SV_POSITION;
};

vout main(float2 pos : POS, float2 tex : TEX)
{
    vout value;
    value.pos = float4(pos, 1.0, 1.0);
    value.tex = tex;
    return value;
}
