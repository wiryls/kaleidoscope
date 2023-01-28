Texture2D<float3> tile : t0;
SamplerState s0;

float4 main(float2 tex : TEXCOORD) : SV_TARGET
{
    return float4(tile.Sample(s0, tex), 1.0);
}
