// Common.cpp
// Common stuff

#include "Common.h"

extern "C"
{
	#include <Windows.h>
}

// Gets time with microsecond precision
double Common::GetUTime()
{
		LARGE_INTEGER li;
		if(!QueryPerformanceFrequency(&li))
				return 0;

		double PCFreq = (double) li.QuadPart;

		QueryPerformanceCounter(&li);
		return (double) (li.QuadPart/PCFreq)*10000000;
}