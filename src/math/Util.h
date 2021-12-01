#pragma once
#include "Typedefs.h"
#include "Vec2.h"

const f32 PI = 3.14159265358979323846f;

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