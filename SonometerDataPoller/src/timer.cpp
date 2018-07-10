/**
* timer.cpp
* Implementation of the Timer class
*
* Contributor: Sébastien Maes
*/
#include <Timer.h>

// Class constructor
Timer::Timer()
{
    clock_gettime(CLOCK_MONOTONIC, &beg_);
}

void Timer::reset() 
{
    clock_gettime(CLOCK_MONOTONIC, &beg_); 
}

double Timer::elapsed()
{
    clock_gettime(CLOCK_MONOTONIC, &end_);
    
    return end_.tv_sec - beg_.tv_sec +
        (end_.tv_nsec - beg_.tv_nsec) / 1e9;
}

