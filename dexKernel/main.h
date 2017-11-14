#ifndef TYPES_H
#define TYPES_H


typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define true 1
#define false 0

#define NULL ((void*)0)

#define U64_MAX (0xFFFFFFFFFFFFFFFF)

typedef unsigned char u8;
typedef unsigned char uint8_t;

typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int uint32_t;
typedef unsigned int size_t;

typedef unsigned long long u64;
typedef long long __int64;

typedef signed char s8;
typedef signed char bool;

typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef u16 wchar;
typedef u16 wchar_t;

typedef u32 Handle;
typedef s32 Result;
typedef void (*ThreadFunc)(u32);

typedef unsigned char           bit8;
typedef unsigned short          bit16;
typedef unsigned int            bit32;
typedef unsigned long long int  bit64; 

#define REG(x)   (*(volatile u32*)(x))
#define REG8(x)  (*(volatile  u8*)(x))
#define REG16(x) (*(volatile u16*)(x))
#define WRITEREG8(addr,data) *(vu8*)(addr) = (vu8)(data)
#define WRITEREG16(addr,data) *(vu16*)(addr) = (vu16)(data)
#define WRITEREG(addr,data) *(vu32*)(addr) = (vu32)(data)
#define SW(addr, data)  *(u32*)(addr) = data


static inline char *strcpy(char *dst, char *src)
{
    char * cp = dst;  
    while( ((*cp++) = (*src++)) );        /* Copy src over dst */
    return( dst );  
}

static inline int strlen(char *s)
{
	int len;

	for (len = 0; s[len]; len++)
		;

	return len;
}

#include "platform.h"

u32 dbgkGetCurrentProcessorNumber();
void dbgkInitHardwareDebugger();
void dbgkSetContextIDToBRP(u32 contextID);
void dbgkSetHardwareWatchpoint(u8 id , u8 loadstoreFlag , u8 byteAddressSelectFlag , u32 address);
void dbgkDisableHardwareWatchpoint(u8 id);

void dbgkDataSynchronizationBarrier();


extern u32 dbgkProcessContextIDOffset;
extern u32 dbgkSetexceptionHandler;
extern u32 IoBaseScu;
u32 dbgkSyncCP14();
u32 dbgkBindInterrupt(u32 * kSchedulerObject);

u32 ipiGetCpuNumber();

u32 dbgkGetTranslationTableBase();

void dbgkHandlePrefetchAbort();
void dbgkHandleDataAbort();
extern u32 dbgkExceptionModeStack;
extern u32 dbgkGateway3DSExceptionHandler;
extern u32 dbgkOriginalPrefetchAbortHandler;
extern u32 dbgkOriginalDataAbortHandler;

#endif

