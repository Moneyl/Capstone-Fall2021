#pragma once
#include "Typedefs.h"
#include <exception>
#include <vector>

//Non owning wrapper for a contiguous chunk of memory
template<class T>
class Span
{
public:
    Span(T* data, u64 size) : _data(data), _size(size) {}
    Span(std::vector<T>& vec) : _data(vec.data()), _size(vec.size()) {}

    T* data() { return _data; } //Get pointer to the memory referenced by the span
    u64 size() const { return _size; } //The number of elements in the span
    u64 size_bytes() const { return _size * sizeof(T); } //The size of the span in bytes

    //Get first element in the span
    T& front() { return *_data; }
    const T& front() const { return *_data; }
    //Get last element in the span
    T& back() { return *(_data + _size); }
    const T& back() const { return *(_data + _size); }

    //These are mainly defined so we can use range based for loops. E.g. for (T& val : someSpanT) { ... }
    //Get pointer to the start of the span
    T* begin() { return _data; }
    const T* begin() const { return _data; }
    //Get pointer to the end of the span (after the final element)
    T* end() { return _data + _size; }
    const T* end() const { return _data + _size; }

    //Overload operator[] to support indexing
    T& operator[](u64 index)
    {
#ifdef DEBUG_BUILD //Bounds checking in debug builds
        if (index >= _size)
            throw std::out_of_range("Span<T>::operator[] out of range. Size = " + std::to_string(_size) + ", Index = " + std::to_string(index));
#endif
        return _data[index];
    }
    const T& operator[](u64 index) const
    {
#ifdef DEBUG_BUILD //Bounds checking in debug builds
        if (index >= _size)
            throw std::out_of_range("const Span<T>::operator[] out of range. Size = " + std::to_string(_size) + ", Index = " + std::to_string(index));
#endif
        return _data[index];
    }

private:
    T* const _data;
    const u64 _size;
};