#pragma once
#include "Typedefs.h"
#include <type_traits>

//3D vector. Can use any arithmetic type for it's components.
template<typename T>
struct Vec3
{
    //Ensure T is an arithmetic type
    static_assert(std::is_arithmetic<T>::value, "Type T in Vec3<T> must be an arithmetic type!");

    T x = 0;
    T y = 0;
    T z = 0;

    Vec3() {}
    Vec3(T xInit, T yInit, T zInit) : x(xInit), y(yInit), z(zInit) { }
    Vec3(T init) : x(init), y(init), z(init) { }

    //Helpers
    T Distance(const Vec3<T>& b) const
    {
        return sqrt<T>(pow<T>(b.x - x, 2) + pow<T>(b.y - y, 2) + pow<T>(b.z - z, 2));
    }

    T Length(const Vec3<T>& b) const
    {
        return sqrt<T>((x * x) + (y * y) + (z * z));
    }

    Vec3<T> Normalized() const
    {
        return *this / Length();
    }

    //Operator overloads
    Vec3<T> operator-(const Vec3<T>& b)
    {
        return Vec3<T>{ x - b.x, y - b.y, z - b.z };
    }

    Vec3<T> operator+(const Vec3<T>& b)
    {
        return Vec3<T>{ x + b.x, y + b.y, z + b.z };
    }

    Vec3<T> operator/(T scalar)
    {
        return Vec3<T>{ x / scalar, y / scalar, z / scalar };
    }

    Vec3<T> operator*(T scalar)
    {
        return Vec3<T>{ x * scalar, y * scalar, z * scalar };
    }

    Vec3<T> operator+=(const Vec3<T>& b)
    {
        *this = *this + b;
        return *this;
    }

    Vec3<T> operator-=(const Vec3<T>& b)
    {
        *this = *this - b;
        return *this;
    }

    Vec3<T> operator/=(T scalar)
    {
        *this = *this / b;
        return *this;
    }

    Vec3<T> operator*=(T scalar)
    {
        *this = (*this) * b;
        return *this;
    }

    //Convert Vec3<T> to any other Vec3 instantiation
    template<typename U>
    operator Vec3<U>()
    {
        return Vec3<U>(x, y, z);
    }
};