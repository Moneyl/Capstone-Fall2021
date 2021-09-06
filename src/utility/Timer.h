#pragma once
#include "src/Typedefs.h"
#include <chrono>

class Timer
{
public:
    Timer(bool StartNow = false)
    {
        if (StartNow)
            Restart();
    };

    void Restart(); //Start/restart the time by setting StartTime to now
    f32 ElapsedNanoseconds();
    f32 ElapsedMicroseconds();
    f32 ElapsedMilliseconds();
    f32 ElapsedSeconds();
    f32 ElapsedMinutes();
    f32 ElapsedHours();

private:
    std::chrono::high_resolution_clock::time_point _startTime;
    std::chrono::high_resolution_clock::time_point _endTime;
};