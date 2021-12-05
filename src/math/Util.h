#pragma once
#include "Typedefs.h"
#include "Vec2.h"

const static f32 PI = 3.14159265358979323846f;

static f32 ToRadians(f32 degrees)
{
    return degrees * (PI / 180.0f);
}

static f32 ToDegrees(f32 radians)
{
    return radians * (180.0f / PI);
}

static bool IsPositionInRect(const Vec2<f32>& pos, const Vec2<f32>& rectPos, const Vec2<f32>& rectSize)
{
    return pos.x > rectPos.x && 
           pos.y > rectPos.y &&
           pos.x < rectPos.x + rectSize.x &&
           pos.y < rectPos.y + rectSize.y;
}

//Returns true if pos is within the triangle. Based on the code & discussion here: https://www.gamedev.net/forums/topic.asp?topic_id=295943
static bool IsPositionInTriangle(const Vec2<f32>& pos, const Vec2<f32>& t0, const Vec2<f32>& t1, const Vec2<f32>& t2)
{
    auto sign = [](const Vec2<f32>& t0, const Vec2<f32>& t1, const Vec2<f32>& t2) -> f32
    {
        return (t0.x - t2.x) * (t1.y - t2.y) - (t1.x - t2.x) * (t0.y - t2.y);
    };

    f32 d0 = sign(pos, t0, t1);
    f32 d1 = sign(pos, t1, t2);
    f32 d2 = sign(pos, t2, t0);

    bool anyNegative = (d0 < 0) || (d1 < 0) || (d2 < 0);
    bool anyPositive = (d0 > 0) || (d1 > 0) || (d2 > 0);
    return !anyNegative || !anyPositive;
}