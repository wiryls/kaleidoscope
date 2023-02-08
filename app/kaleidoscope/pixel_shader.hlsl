SamplerState screenshot_sampler : register(s0);
Texture2D screenshot : register(t0);

// Note: about "gourp"
// https://stackoverflow.com/a/61378340
cbuffer regular_triangle_group : register(b0)
{
    float2 top;
    float length;
    float height;
};

float cross2(float2 a, float2 b)
{
    return a.x * b.y - a.y * b.x;
}

bool is_inside(float2 source)
{
    float2 left = top + float2(-length * 0.5, height);
    float2 right = top + float2(length * 0.5, height);
    return cross2(top - left, source - left) >= 0
        && cross2(right - top, source - top) >= 0
        && cross2(left - right, source - right) >= 0;
}

float2 solve(float2 source, float2 anchor, float2 mirror, float2 project)
{
    // (? - source) x project = 0
    // (? + source - 2 * anchor) x mirror = 0
    // return ?

    float a = cross2(source, project);
    float b = cross2(2.0 * anchor - source, mirror);
    float k = cross2(mirror, project);
    return (a * mirror - b * project) / k;
}

float2 redirect(float2 pos)
{
    // Bounding box of the repeat pattern
    float2 size = float2(length * 3.0, height * 2.0);
    float2 center = float2(top.x + length, top.y);
    float2 top_left = float2(top.x - length * 0.5, top.y - height);

    // Calculate (x, y) in bounding box
    pos = top_left + frac((pos - top_left) / size) * size;

    // Reduce range again
    if (pos.x >= center.x && pos.y < center.y)
    {
        pos.x -= size.x * 0.5;
        pos.y += size.y * 0.5;
    }
    else if (pos.x >= center.x && pos.y >= center.y)
    {
        pos.x -= size.x * 0.5;
        pos.y -= size.y * 0.5;
        pos.y = center.y * 2.0 - pos.y;
    }
    else if (pos.x < center.x && pos.y < center.y)
    {
        pos.y = center.y * 2.0 - pos.y;
    }
    return pos;
}

float2 reflect(float2 pos)
{
    // Define points of boundary
    float2 to_top_left = float2(-length * 0.5, -height);
    float2 to_top_right = float2(length * 0.5, -height);

    float2 left = top - to_top_right;
    float2 right = top - to_top_left;

    // Repair
    if (cross2(to_top_right, pos - right) > 0)
    {
        pos.x -= length * 1.5;
        pos.y = 2 * top.y + height - pos.y;
    }

    // Reflect
    if (cross2(to_top_right, pos - left) < 0)
    {
        pos = solve(pos, left, to_top_right, 0.5 * to_top_right - to_top_left);
    }
    else if (cross2(to_top_left, pos - right) > 0)
    {
        pos = solve(pos, right, to_top_left, 0.5 * to_top_left - to_top_right);
    }
    return pos;
}

float4 main(float2 tex : TEXCOORD) : SV_TARGET
{
    if (is_inside(tex))
        return float4(0.0, 0.0, 0.0, 0.0);
    else
        return screenshot.Sample(screenshot_sampler, reflect(redirect(tex)));
}
