// Common.cpp
// Common stuff

#include "Common.h"

extern "C"
{
	#include <Windows.h>
}

const char* Common::GetFileNameFromFullPath(const char* fullPath)
{
    if (fullPath == nullptr)
        return nullptr;

    const char* c = fullPath + strlen(fullPath);

    while (c != fullPath)
    {
        if (*c == '\\' || *c == '/')
        {
            ++c;
            break;
        }

        --c;
    }

    char* fileName = new char[strlen(c) + 1];
    strcpy(fileName, c);

    return fileName;
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