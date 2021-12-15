#pragma once
#include <algorithm>
#include <vector>

//Erase elements that match the conditions of the predicate. Equivalent to C++20s std::erase_if
//Predicate is a lambda of type [](T& val) -> bool { return ...; }
template<class T, class Pred>
void EraseIf(std::vector<T>& vec, Pred predicate)
{
    //Move matching items to end of vector & get an iterator at the first removed item
    auto eraseStart = std::remove_if(vec.begin(), vec.end(), predicate);
    //Erase the removed items
    vec.erase(eraseStart, vec.end());
}

template<class T>
bool Contains(const std::vector<T>& vec, T value)
{
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}