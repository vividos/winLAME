#pragma once

#if !defined(PLATFORM_WINDOWS)

// we treat bool as a global type, so don't declare it in the namespace
#ifdef PLATFORM_APPLE
    typedef signed char BOOL;  // this is the way it's defined in Obj-C
#else
    typedef unsigned char BOOL; // this is the way it's defined in X11
#endif

namespace APE
{
#undef __forceinline
#define __forceinline inline
#define __stdcall

#define NEAR
#define FAR

typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef void *              HANDLE;
typedef unsigned int        UINT;
typedef const char *        LPCSTR;
typedef char *              LPSTR;
typedef long                LRESULT;

#undef ZeroMemory
#define ZeroMemory(POINTER, BYTES) memset(POINTER, 0, BYTES);

#define TRUE 1
#define FALSE 0

#define CALLBACK

#ifdef _T
#undef _T
#endif
#define _T(x) L ## x

#define _strnicmp strncasecmp
#define _wtoi(x) wcstol(x, NULL, 10)
#define _tcscat wcscat
#undef _totlower
#define _totlower towlower
#define _totupper towupper
#define _tcschr wcschr
#define _tcsicmp wcscasecmp
#define _tcscpy wcscpy
#define _tcslen wcslen

#define MAX_PATH    4096

#include <wctype.h>
#include <string.h>

}

#endif // #ifndef _WIN32
