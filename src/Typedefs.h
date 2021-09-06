#pragma once
#include <cstdint>

//Taking a cue from rust and making types with their data size in their names
//Uses static_assert to verify they're the right size at compile time.
//This makes code much clearer instead of having to remember the size of a type on any given platform

//Signed ints
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

//Unsigned ints
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

//Floating point values
using f32 = float;
using f64 = double;

static_assert(sizeof(i8) == 1, "Expected sizeof(i8) == 1");
static_assert(sizeof(i16) == 2, "Expected sizeof(i16) == 2");
static_assert(sizeof(i32) == 4, "Expected sizeof(i32) == 4");
static_assert(sizeof(i64) == 8, "Expected sizeof(i64) == 8");

static_assert(sizeof(u8) == 1, "Expected sizeof(u8) == 1");
static_assert(sizeof(u16) == 2, "Expected sizeof(u16) == 2");
static_assert(sizeof(u32) == 4, "Expected sizeof(u32) == 4");
static_assert(sizeof(u64) == 8, "Expected sizeof(u64) == 8");

static_assert(sizeof(f32) == 4, "Expected sizeof(f32) == 4");
static_assert(sizeof(f64) == 8, "Expected sizeof(f64) == 8");