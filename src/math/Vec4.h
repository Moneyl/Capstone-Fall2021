#pragma once
#include "Typedefs.h"
#include "vm/Instruction.h"
#include <type_traits>

//4D vector. Can use any arithmetic type for it's components.
template<typename T>
struct Vec4
{
    //Ensure T is an arithmetic type
    static_assert(std::is_arithmetic<T>::value, "Type T in Vec4<T> must be an arithmetic type!");

    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;

    Vec4() {}
    Vec4(T xInit, T yInit, T zInit, T wInit) : x(xInit), y(yInit), z(zInit), w(wInit) { }
    Vec4(T init) : x(init), y(init), z(init), w(init) { }

    //Helpers
    T Distance(const Vec4<T>& b) const
    {
        return sqrt<T>(pow<T>(b.x - x, 2) + pow<T>(b.y - y, 2) + pow<T>(b.z - z, 2) + pow<T>(b.w - w, 2));
    }

    T Length(const Vec4<T>& b) const
    {
        return sqrt<T>((x * x) + (y * y) + (z * z) + (w * w));
    }

    Vec4<T> Normalized() const
    {
        return *this / Length();
    }

    //Operator overloads
    Vec4<T> operator-(const Vec4<T>& b)
    {
        return Vec4<T>{ x - b.x, y - b.y, z - b.z, w - b.w };
    }

    Vec4<T> operator+(const Vec4<T>& b)
    {
        return Vec4<T>{ x + b.x, y + b.y, z + b.z, w + b.w };
    }

    Vec4<T> operator/(T scalar)
    {
        return Vec4<T>{ x / scalar, y / scalar, z / scalar, w / scalar };
    }

    Vec4<T> operator*(T scalar)
    {
        return Vec4<T>{ x * scalar, y * scalar, z * scalar, w * scalar };
    }

    Vec4<T> operator+=(const Vec4<T>& b)
    {
        *this = *this + b;
        return *this;
    }

    Vec4<T> operator-=(const Vec4<T>& b)
    {
        *this = *this - b;
        return *this;
    }

    Vec4<T> operator/=(T scalar)
    {
        *this = *this / b;
        return *this;
    }

    Vec4<T> operator*=(T scalar)
    {
        *this = (*this) * b;
        return *this;
    }

    //Convert Vec4<T> to any other Vec4 instantiation
    template<typename U>
    operator Vec4<U>()
    {
        return Vec4<U>(x, y, z, w);
    }
};