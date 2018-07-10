/**
* Timer.h
* Definition of the Timer class
*
* Contributor: Sébastien Maes
*/
#ifndef __TIMER__
#define __TIMER__

#include <iostream>
#include <ctime>

class Timer
{
private:
    timespec beg_, end_;

public:
    Timer();

    double elapsed();

    void reset();


};

#endif // __TIMER__
