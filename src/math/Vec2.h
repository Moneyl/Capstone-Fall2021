#pragma once
#include "Typedefs.h"
#include <type_traits>

//2D vector. Can use any arithmetic type for it's components. 
//E.g. The VM and robots use Vec2<VmValue> since registers hold VmValue, while the renderer uses Vec2<i32> for greater range and Vec4<u8> for RGBA colors.
template<typename T>
struct Vec2
{
    //Ensure T is an arithmetic type
    static_assert(std::is_arithmetic<T>::value, "Type T in Vec2<T> must be an arithmetic type!");

    T x = 0;
    T y = 0;

    Vec2() {}
    Vec2(T xInit, T yInit) : x(xInit), y(yInit) { }
    Vec2(T init) : x(init), y(init) { }

    //Helpers
    T Distance(const Vec2<T>& b) const
    {
        return std::sqrt(pow(b.x - x, 2) + pow(b.y - y, 2));
    }

    T Length() const
    {
        return std::sqrt((x * x) + (y * y));
    }

    Vec2<T> Normalized() const
    {
        return *this / Length();
    }

    f32 Dot(const Vec2<T>& b) const
    {
        return (x * b.x) + (y * b.y);
    }

    //Calculate the angle of the vector on the unit circle. Assumes it's a 2d normalized direction vector.
    f32 AngleUnitRadians() const
    {
        f32 angle = std::atan2(y, x);
        if (angle < 0.0f)
            angle += 2 * PI;

        return angle;
    }

    f32 AngleUnitDegrees() const
    {
        return ToDegrees(AngleUnitRadians());
    }

    void Rotate(const Vec2<f32>& origin, f32 angleDegrees)
    {
        const f32 angleRadians = ToRadians(angleDegrees);
        const Vec2<f32> rotation = { cos(angleRadians), sin(angleRadians) };
        const Vec2<f32> posToOrigin = *this - origin;
        x = (posToOrigin.x * rotation.x) - (posToOrigin.y * rotation.y) + origin.x;
        y = (posToOrigin.x * rotation.y) + (posToOrigin.y * rotation.x) + origin.y;
    }

    //Operator overloads
    Vec2<T> operator-(const Vec2<T>& b) const
    {
        return Vec2<T>{ x - b.x, y - b.y };
    }

    Vec2<T> operator+(const Vec2<T>& b) const
    {
        return Vec2<T>{ x + b.x, y + b.y };
    }

    Vec2<T> operator/(T scalar) const
    {
        return Vec2<T>{ x / scalar, y / scalar };
    }

    Vec2<T> operator*(T scalar) const
    {
        return Vec2<T>{ x* scalar, y* scalar };
    }

    Vec2<T> operator*(const Vec2<T>& b) const
    {
        return Vec2<T>{ x* b.x, y* b.y };
    }

    Vec2<T> operator+=(const Vec2<T>& b)
    {
        *this = *this + b;
        return *this;
    }

    Vec2<T> operator-=(const Vec2<T>& b)
    {
        *this = *this - b;
        return *this;
    }

    Vec2<T> operator/=(T scalar)
    {
        *this = *this / b;
        return *this;
    }

    Vec2<T> operator*=(T scalar)
    {
        *this = (*this) * b;
        return *this;
    }

    Vec2<T> operator*=(const Vec2<T>& b)
    {
        this->x *= b.x;
        this->y *= b.y;
        return *this;
    }

    //Convert Vec2<T> to any other Vec2 instantiation
    template<typename U>
    operator Vec2<U>()
    {
        return Vec2<U>(x, y);
    }
};