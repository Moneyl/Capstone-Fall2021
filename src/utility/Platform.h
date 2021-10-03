#pragma once
#include "src/Typedefs.h"

//Windows specific
#include <winsdkver.h>
#define _WIN32_WINNT 0x0601 //Target windows 7
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <synchapi.h> //For windows Sleep()
#include <timeapi.h>

//Todo: Add support for linux and mac os. One way this might be done is to define functions here, then have separate .cpp files with the implementations for each platform
void ThreadSleep(u32 ms, bool setTimerPrecision = false)
{
    //On some windows systems timer precision needs to manually be set to 1ms to get accurate sleeps.
    if(setTimerPrecision)
        timeBeginPeriod(1);

    Sleep(ms);

    if(setTimerPrecision)
        timeEndPeriod(1);
}