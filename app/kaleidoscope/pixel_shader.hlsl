// BGRA texture
Texture2D<float4> screenshot : register(t0);
//SamplerState s0;

float4 main(float2 tex : TEXCOORD) : SV_TARGET
{
    if (tex.x > 0.5 && tex.y > 0.5)
        return float4(0.0, 0.0, 0.0, 0.0);
    else
        return float4(tex, 1.0, 1.0);
}
