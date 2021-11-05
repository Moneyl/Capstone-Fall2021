#pragma once
#include "Typedefs.h"
#include "vm/Instruction.h"
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
        return sqrt<T>(pow<T>(b.x - x, 2) + pow<T>(b.y - y, 2));
    }

    T Length(const Vec2<T>& b) const
    {
        return sqrt<T>((x * x) + (y * y));
    }

    Vec2<T> Normalized() const
    {
        return *this / Length();
    }

    //Operator overloads
    Vec2<T> operator-(const Vec2<T>& b)
    {
        return Vec2<T>{ x - b.x, y - b.y };
    }

    Vec2<T> operator+(const Vec2<T>& b)
    {
        return Vec2<T>{ x + b.x, y + b.y };
    }

    Vec2<T> operator/(T scalar)
    {
        return Vec2<T>{ x / scalar, y / scalar };
    }

    Vec2<T> operator*(T scalar)
    {
        return Vec2<T>{ x* scalar, y* scalar };
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

    //Convert Vec2<T> to any other Vec2 instantiation
    template<typename U>
    operator Vec2<U>()
    {
        return Vec2<U>(x, y);
    }
};