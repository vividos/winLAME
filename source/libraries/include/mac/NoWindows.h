#pragma once

#if !defined(PLATFORM_WINDOWS)

// we treat bool as a global type, so don't declare it in the namespace
#ifdef PLATFORM_APPLE
    #include <AvailabilityMacros.h>

    #ifndef OBJC_BOOL_DEFINED
        #if OBJC_BOOL_IS_BOOL
            typedef bool BOOL;
        #else
            typedef signed char BOOL; // this is the way it's defined in Obj-C
        #endif
    #endif
#else
    typedef unsigned char BOOL; // this is the way it's defined in X11
#endif

namespace APE
{
#undef __forceinline
#define __forceinline inline

#ifndef __stdcall
#define __stdcall
#endif

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

#undef ZeroMemory
#define ZeroMemory(POINTER, BYTES) memset(POINTER, 0, BYTES);

#define TRUE 1
#define FALSE 0

#define CALLBACK

#define _strnicmp strncasecmp
#define _wtoi(x) wcstol(x, NULL, 10)

#ifdef PLATFORM_APPLE
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
        #define _wcsicmp wcscasecmp
    #else
        #define _wcsicmp wcscmp // fall back to case sensitive comparison on Mac OS X 10.6.x and earlier
    #endif
#else
    #define _wcsicmp wcscasecmp
#endif

#define MAX_PATH    4096

}

#include <wctype.h>
#include <string.h>

#endif // #ifndef _WIN32
