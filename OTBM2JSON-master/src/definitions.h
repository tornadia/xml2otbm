////////////////////////////////////////////////////////////////////////////////
// Survival Server - An MMPORPG (Massive MultiPlayer Online Role Playing Game)//
////////////////////////////////////////////////////////////////////////////////
// Developed by Dark-bart.                                                    //
////////////////////////////////////////////////////////////////////////////////

#ifndef __definitions_h
#define __definitions_h

#define Survival_Version 5.67

typedef unsigned long long uint64_t;

#if defined __WINDOWS__ || defined WIN32

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#pragma warning(disable:4786)
#include <winsock.h>

#else

#include <stdint.h>
typedef int64_t __int64;

#endif

#endif
