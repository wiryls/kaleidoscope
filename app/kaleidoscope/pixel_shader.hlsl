SamplerState screenshot_sampler : register(s0);
Texture2D screenshot : register(t0);

// Note: about "gourp"
// https://stackoverflow.com/a/61378340
cbuffer triangle_group : register(b0)
{
    float2 const origin;
    float  const length;
    float  const height;
};

float cross2(float2 a, float2 b)
{
    return a.x * b.y - a.y * b.x;
}

float2 reflect(float2 source, float2 anchor, float2 mirror, float2 project)
{
    // (? - source) x project = 0
    // (? + source - 2 * anchor) x mirror = 0
    // return ?

    float a = cross2(source, project);
    float b = cross2(2.0 * anchor - source, mirror);
    float k = cross2(mirror, project);
    return (a * mirror - b * project) / k;
}

float2 redirect(float2 o)
{
    // Bounding box of the repeat pattern
    float2 const size = float2(length * 3.0, height * 2.0);
    float2 const center = float2(origin.x + length, origin.y);
    float2 const top_left = float2(origin.x - length * 0.5, origin.y - height);

    // Calculate (x, y) in bounding box
    o = top_left + frac((o - top_left) / size) * size;

    // Reduce range again
    if (o.x >= center.x && o.y < center.y)
    {
        o.x -= size.x * 0.5;
        o.y += size.y * 0.5;
    }
    else if (o.x >= center.x && o.y >= center.y)
    {
        o.x -= size.x * 0.5;
        o.y -= size.y * 0.5;
        o.y = center.y * 2.0 - o.y;
    }
    else if (o.x < center.x && o.y < center.y)
    {
        o.y = center.y * 2.0 - o.y;
    }
    return o;
}

float4 main(float2 o : TEXCOORD) : SV_TARGET
{
    float const half_length = length * 0.5;

    float2 const left_to_top = float2(half_length, -height);
    float2 const right_to_top = float2(-half_length, -height);

    float2 const top = origin;
    float2 const left = top - left_to_top;
    float2 const right = top - right_to_top;

    // Ignore if o is inside triangle
    if (cross2(top - left, o - left) >= 0 &&
        cross2(right - top, o - top) >= 0 &&
        cross2(left - right, o - right) >= 0)
    {
        return float4(0, 0, 0, 0);
    }

    // [1] Minimum repeat pattern
    o = redirect(o);

    // [2] Triangulation
    if (cross2(left_to_top, o - right) > 0)
    {
        o.x -= length * 1.5;
        o.y = 2 * top.y + height - o.y;
    }

    // [3] Reflect
    if (cross2(left_to_top, o - left) < 0)
    {
        o = reflect(o, left, left_to_top, 0.5 * left_to_top - right_to_top);
    }
    else if (cross2(right_to_top, o - right) > 0)
    {
        o = reflect(o, right, right_to_top, 0.5 * right_to_top - left_to_top);
    }

    return screenshot.Sample(screenshot_sampler, o);
}
