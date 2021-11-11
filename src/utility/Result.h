#pragma once
#include "Typedefs.h"
#include <type_traits>
#include <optional>
#include <variant>

//Return this from functions using Result<T, U> when the function succeeds
template<class T>
struct Success
{
    Success(T data) : Data(data) {}
    T Data;
};

//Return this from functions using Result<T, U> when the function fails
template<class T>
struct Error
{
    Error(T data) : Data(data) {}
    T Data;
};

//Contains either a function return value or error information if the function fails. Based on similar concept from Rust.
//Return Success<T> or Error<U> to implicitly construct a Result<T, U>
template<class ResultType, class ErrorType>
class Result 
    : std::is_nothrow_destructible<ResultType>, std::is_nothrow_destructible<ErrorType> //Required by std::variant<...>
{
public:
    //Implicitly constructed by returning either a Success<ResultType> or Error<ErrorType> from the function
    Result(Success<ResultType> success) : _data(success.Data) {}
    Result(Error<ErrorType> error) : _data(error.Data) {}

    //Get result state and data. Can check for error/success with `if (result.Success())` since std::optional<T> can be treated as a bool.
    std::optional<ResultType> Success() const { return std::holds_alternative<ResultType>(_data) ? std::get<ResultType>(_data) : std::optional<ResultType>{}; }
    std::optional<ErrorType> Error() const { return std::holds_alternative<ErrorType>(_data) ? std::get<ErrorType>(_data) : std::optional<ErrorType>{}; }

private:
    std::variant<ResultType, ErrorType> _data;
};

//Specializations for Success<void> and Result<void, ErrorType>
template<>
struct Success<void>
{
    Success() {}
};

template<class ErrorType>
class Result<void, ErrorType>
{
public:
    //Implicitly constructed by returning either a Success<void> or Error<ErrorType> from the function
    Result(Success<void> success) {}
    Result(Error<ErrorType> error) : _data(error.Data) {}

    //Get result state and data
    bool Success() const { return !_data.has_value(); }
    std::optional<ErrorType> Error() const { return _data; }

private:
    std::optional<ErrorType> _data;
};