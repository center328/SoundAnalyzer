#ifndef _PRECOMP_H
#define _PRECOMP_H

#define _CRT_SECURE_NO_DEPRECATE
// Activate memory leak detection in debug
#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <assert.h>
#include "Profile.h"

#define LOG_MAX_SIZE 1024

inline void Log(const char* fmt, ...)
{
	char buf[LOG_MAX_SIZE];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buf, sizeof(buf)-1, fmt, ap);
	va_end(ap);
	buf[LOG_MAX_SIZE-1] = 0;
	printf(buf);
}

#ifdef USE_LOGGER
	#define LOG(X, ...) Log(X,  __VA_ARGS__)
#else
	#define LOG(fmt, ...) 
#endif
#endif