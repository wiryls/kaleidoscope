//Texture2D<float3> tile : t0;
//SamplerState s0;

float4 main(float2 tex : TEXCOORD) : SV_TARGET
{
    return float4(tex, 0.0, 0.5);
}
