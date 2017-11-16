#include "main.h"

u32 IoBaseScu = 0;

dbgKernelExceptionHandlerContext exceptionHandlerCtx_NEW81 = {
	//for new 3ds 8.1 and 9.2
	(u8*)0xFFF2E008,
	(u8*)0xFFF2E00A,
	0xDFFF4258,
	0xFFFF0258,
	0xDFFF413C,
	0xFFFF013C
};

dbgKernelExceptionHandlerContext exceptionHandlerCtx_NEW112 = {
	//for new 3ds 11.2
	(u8*)0xFFF2F008,//only 2 offsets have been changed
	(u8*)0xFFF2F00A,
	0xDFFF4258,
	0xFFFF0258,
	0xDFFF413C,
	0xFFFF013C
};

dbgKernelExceptionHandlerContext exceptionHandlerCtx_Old96 = {
	//for old 3ds 9.0 9.6
	(u8*)0xFFF2D00A,
	(u8*)0xFFF2D008,
	0xDFFF4258,
	0xFFFF0258,
	0xDFFF413C,
	0xFFFF013C
};

dbgKernelExceptionHandlerContext exceptionHandlerCtx_Old112 = {
	//for old 3ds 11.2
	(u8*)0xFFF2E00A,
	(u8*)0xFFF2E008,
	0xDFFF4258,
	0xFFFF0258,
	0xDFFF413C,
	0xFFFF013C
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_NEW81 = {
	//for new 3ds 8.1
	(void*)0xFFF24C9C,
	(void*)0xFFF1CF7C,
	(void*)0xFFF1CCA0,
	(void*)0xFFF1F04C
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_NEW92 = {
	//for new 3ds 9.2
	(void*)0xFFF25768,
	(void*)0xFFF1D9D4,
	(void*)0xFFF1D67C,
	(void*)0xFFF1FEEC
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_NEW95 = {
	//for new 3ds 9.5
	(void*)0xFFF25BD8,
	(void*)0xFFF1D9AC,
	(void*)0xFFF1D654,
	(void*)0xFFF1FCE8
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_NEW102 = {
	//for new3ds 10.2
	(void*)0xFFF25BFC,
	(void*)0xFFF1D9AC,
	(void*)0xFFF1D654,
	(void*)0xFFF1FCE8
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_NEW112 = {
	//for new 3ds 11.2
	(void*)0xFFF26210,
	(void*)0xFFF1DF8C,
	(void*)0xFFF1DC34,
	(void*)0xFFF202C8
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_NEW114 =
{
	//for new 3ds 11.4
	(void*)0xFFF27480,
	(void*)0xFFF1E1DC,
	(void*)0xFFF1DE84,
	(void*)0xFFF20518
};


dbgKernelCacheInterfaceContext cacheInterfaceCtx_Old96 = {
	//for old 3ds 9.6
	(void*)0xFFF24FF0,
	(void*)0xFFF1CF98,
	(void*)0xFFF1CD30,
	(void*)0xFFF1F748
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_Old90 = {
	//for old 3ds 9.0
	(void*)0xFFF24B54,
	(void*)0xFFF1CC5C,
	(void*)0xFFF1C9F4,
	(void*)0xFFF1F47C
};

dbgKernelCacheInterfaceContext cacheInterfaceCtx_Old112 = {
    //for old 3ds 11.2
    (void*)0xFFF255C8,
    (void*)0xFFF1D7F4,
    (void*)0xFFF1D58C,
    (void*)0xFFF1FCEC
};

dbgKernelInterruptContext interruptCtx_NEW81 = {
	//for new 3ds 8.1
	0xFFF319F4,
	(u32*)0xFFF2D2A4,
	(void*)0xFFF174B4
};

dbgKernelInterruptContext interruptCtx_NEW92 = {
	//for new 3ds 9.2
	0xFFF318F4,
	(u32*)0xFFF2D2A8,
	(void*)0xFFF17FDC
};

dbgKernelInterruptContext interruptCtx_NEW95 = {
	//for new 3ds 9.5
	0xFFF318F4,
	(u32*)0xFFF2D2A8,
	(void*)0xFFF17D70
};

dbgKernelInterruptContext interruptCtx_NEW102 = {
	//for new 3ds 10.2
	0xFFF318F4,
	(u32*)0xFFF2D2A8,
	(void*)0xFFF17D70
};

dbgKernelInterruptContext interruptCtx_NEW112 = {
	//for new 3ds 11.2
	0xFFF32924,
	(u32*)0xFFF2E2D4,
	(void*)0xFFF17F50
};

dbgKernelInterruptContext interruptCtx_NEW114 = {
	//for new 3ds 11.4
	0xFFF32924,
	(u32*)0xFFF2E2D4,
	(void*)0xFFF180F8
};

dbgKernelInterruptContext interruptCtx_Old96 = {
	0xFFF308EC,
	(u32*)0xFFF2C2A8,
	(void*)0xFFF178BC
};

dbgKernelInterruptContext interruptCtx_Old90 = {
	0xFFF308EC,
	(u32*)0xFFF2C0E8,
	(void*)0xFFF177A0
};

dbgKernelInterruptContext interruptCtx_Old112 = {
	0xFFF3191C,
	(u32*)0xFFF2D2D4,
	(void*)0xFFF17D5C
};

dbgConflictResolutionContext conflictCtx_Old96 = {
	0xFFF02080,
	0xFFF020C8,
	0xFFFF3000,
	0xDFF80614,
	0xFFF00614
};

dbgConflictResolutionContext conflictCtx_New92 = {
	0xFFF0205C,
	0xFFF020A4,
	0xFFFF3000,
	0xDFF80618,
	0xFFF00618
};

dbgConflictResolutionContext conflictCtx_New95 = {
	0xFFF02068,
	0xFFF020B0,
	0xFFFF3000,
	0xDFF80624,
	0xFFF00624
};

dbgConflictResolutionContext conflictCtx_New102 = {
	0xFFF0206C,
	0xFFF020B4,
	0xFFFF3000,
	0xDFF80624,
	0xFFF00624
};

u32 dbgkGetMainThreadTLS_New3ds()
{
	u32 currentProcess = *(u32*)0xFFFF9004;
	u32 mainThread = *(u32*)(currentProcess + 0xc8);
	return *(u32*)(mainThread + 0x94);
}
u32 dbgkGetMainThreadTLS_Old3ds()
{
	u32 currentProcess = *(u32*)0xFFFF9004;
	u32 mainThread = *(u32*)(currentProcess + 0xb8);
	return *(u32*)(mainThread + 0x94);
}
u32 dbgkGetMainThreadTLS_Old3ds80()
{
	u32 currentProcess = *(u32*)0xFFFF9004;
	u32 mainThread = *(u32*)(currentProcess + 0xc0);
	return *(u32*)(mainThread + 0x94);
}

u64 dbgGetCurrentProcessName_New3ds()
{
	u32 currentProcess = *(u32*)0xFFFF9004;
	u32 codeset = *(u32*)(currentProcess + 0xb8);
	return *(u64*)(codeset + 0x50);
}
u64 dbgGetCurrentProcessName_Old3ds()
{
	u32 currentProcess = *(u32*)0xFFFF9004;
	u32 codeset = *(u32*)(currentProcess + 0xa8);
	return *(u64*)(codeset + 0x50);
}
u64 dbgGetCurrentProcessName_Old3ds80()
{
	u32 currentProcess = *(u32*)0xFFFF9004;
	u32 codeset = *(u32*)(currentProcess + 0xb0);
	return *(u64*)(codeset + 0x50);
}

void rtGenerateJumpCode(u32 dst, u32* buf) 
{
	buf[0] = 0xe51ff004;
	buf[1] = dst;
}

void kFlushDataCache(dbgPlatformContext * ctx, u32 address , u32 size)
{
	if (ctx->cache && ctx->cache->flushDataCache)
	{
		ctx->cache->flushDataCache((void*)address, size);
	}
}

void kFlushInstructionCache(dbgPlatformContext * ctx, u32 address , u32 size)
{
	if (ctx->cache && ctx->cache->flushInstructionCache)
	{
		ctx->cache->flushInstructionCache((void*)address, size);
	}
}

void kStoreDataCache(dbgPlatformContext * ctx, u32 address , u32 size)
{
	if (ctx->cache && ctx->cache->storeDataCache)
	{
		ctx->cache->storeDataCache((void*)address, size);
	}
}

void kInvalidateDataCache(dbgPlatformContext * ctx, u32 address , u32 size)
{
	if (ctx->cache && ctx->cache->invalidateDataCache)
	{
		ctx->cache->invalidateDataCache((void*)address, size);
	}
}

void kInterruptHax(dbgPlatformContext * ctx)
{
	if (ctx->interrupt)
	{
		static u32 fakeVirtualTable[2];
		u32 interruptTableAddr = ctx->interrupt->interruptTableAddr;
		u32* schedulerObjectVirtualTable = ctx->interrupt->schedulerObjectVirtualTable;
		fakeVirtualTable[0] = (u32)dbgkBindInterrupt;
		fakeVirtualTable[1] = schedulerObjectVirtualTable[1];
		dbgkDataSynchronizationBarrier();
		u32 cpuCount = ipiGetCpuNumber();
		u32 i;
		for(i = 0; i < cpuCount; i++)
		{
			u32 * schedulerObject = (u32*)(*(u32*)(interruptTableAddr + 0x100 * i + 8 * 8));
			if(schedulerObject)
				*schedulerObject = (u32)&fakeVirtualTable;
			dbgkDataSynchronizationBarrier();
		}
	}
}

void kExceptionHandlerHax(dbgPlatformContext * ctx)
{
	if (ctx->exceptionHandler)
	{
		if (ctx->exceptionHandler->enableFlag0)
			*ctx->exceptionHandler->enableFlag0 = 1;
		if (ctx->exceptionHandler->enableFlag1)
			*ctx->exceptionHandler->enableFlag1 = 1;
		dbgkDataSynchronizationBarrier();
		if (ctx->exceptionHandler->offestCheckPatchAddr)
		{
			*(u32*)ctx->exceptionHandler->offestCheckPatchAddr = 0xE1A00000;//nop
			kFlushDataCache(ctx, ctx->exceptionHandler->offestCheckPatchAddr, 4);
			kFlushInstructionCache(ctx, ctx->exceptionHandler->offestCheckPatchAddrK, 4);
		}
		if (ctx->exceptionHandler->hookAddr)
		{
			rtGenerateJumpCode((u32)&dbgkSetexceptionHandler, (u32*)ctx->exceptionHandler->hookAddr);
			kFlushDataCache(ctx, ctx->exceptionHandler->hookAddr, 8);
			kFlushInstructionCache(ctx, ctx->exceptionHandler->hookAddrK, 8);
			if (ctx->isNew3DS)
				dbgkProcessContextIDOffset = 0x4C;
			else
				dbgkProcessContextIDOffset = 0x44;
			dbgkDataSynchronizationBarrier();
		}
	}
}

void kResolveConflict(dbgPlatformContext * ctx)
{
	if (ctx->conflictResolution)
	{
		u32 prefetchAbortHandler = *(u32*)(ctx->conflictResolution->patchBase + 8);
		u32 dataAbortHandler = *(u32*)(ctx->conflictResolution->patchBase + 0x14);
		if (prefetchAbortHandler != ctx->conflictResolution->prefetchAbortHandler || dataAbortHandler != ctx->conflictResolution->dataAbortHandler)
		{
			dbgkExceptionModeStack = ctx->conflictResolution->exceptionModeStack;
			dbgkGateway3DSExceptionHandler = prefetchAbortHandler;
			dbgkOriginalPrefetchAbortHandler = ctx->conflictResolution->prefetchAbortHandler;
			dbgkOriginalDataAbortHandler = ctx->conflictResolution->dataAbortHandler;
			dbgkDataSynchronizationBarrier();
			*(u32*)(ctx->conflictResolution->patchBase + 8) = dbgkHandlePrefetchAbort;
			*(u32*)(ctx->conflictResolution->patchBase + 0x14) = dbgkHandleDataAbort;
			kFlushDataCache(ctx, ctx->conflictResolution->patchBase, 0x18);
			kFlushInstructionCache(ctx, ctx->conflictResolution->patchBaseK, 0x18);
		}
	}
}

void dbgInitPlatformContext(dbgPlatformContext * ctx, u32 isNew3DS, u32 firmVersion)
{
	memset(ctx, 0, sizeof(dbgPlatformContext));
	ctx->isNew3DS = isNew3DS;
	ctx->firmVersion = firmVersion;
	if (ctx->isNew3DS)
	{
		if (ctx->firmVersion == SYSTEM_VERSION(8, 1, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW81;
			ctx->exceptionHandler = &exceptionHandlerCtx_NEW81;
			ctx->interrupt = &interruptCtx_NEW81;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(9, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW92;
			ctx->exceptionHandler = &exceptionHandlerCtx_NEW81;
			ctx->interrupt = &interruptCtx_NEW92;
			ctx->conflictResolution = &conflictCtx_New92;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(9, 5, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW95;
			ctx->exceptionHandler = &exceptionHandlerCtx_NEW81;
			ctx->interrupt = &interruptCtx_NEW95;
			ctx->conflictResolution = &conflictCtx_New95;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(10, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW102;
			ctx->exceptionHandler = &exceptionHandlerCtx_NEW81;
			ctx->interrupt = &interruptCtx_NEW102;
			ctx->conflictResolution = &conflictCtx_New102;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(11, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW112;
			ctx->exceptionHandler = &exceptionHandlerCtx_NEW112;
			ctx->interrupt = &interruptCtx_NEW112;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(11, 4, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW114;
			ctx->exceptionHandler = &exceptionHandlerCtx_NEW112;
			ctx->interrupt = &interruptCtx_NEW114;
		}
		ctx->regInterruptBaseAddr = 0xfffee000;//all of firm are same
		ctx->dbgGetMainThreadTLS = (void*)dbgkGetMainThreadTLS_New3ds;
		ctx->dbgGetCurrentProcessName = (void*)dbgGetCurrentProcessName_New3ds;
	}
	else
	{
		//TODO:support old 3ds
		if (ctx->firmVersion >= SYSTEM_VERSION(8, 0, 0))
		{
			ctx->dbgGetMainThreadTLS = (void*)dbgkGetMainThreadTLS_Old3ds80;
			ctx->dbgGetCurrentProcessName = (void*)dbgGetCurrentProcessName_Old3ds80;
		}
		else
		{
			ctx->dbgGetMainThreadTLS = (void*)dbgkGetMainThreadTLS_Old3ds;
			ctx->dbgGetCurrentProcessName = (void*)dbgGetCurrentProcessName_Old3ds;
		}
		if (ctx->firmVersion == SYSTEM_VERSION(9, 0, 0))
		{
			ctx->cache = &cacheInterfaceCtx_Old90;
			ctx->exceptionHandler = &exceptionHandlerCtx_Old96;
			ctx->interrupt = &interruptCtx_Old90;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(9, 6, 0))
		{
			ctx->cache = &cacheInterfaceCtx_Old96;
			ctx->exceptionHandler = &exceptionHandlerCtx_Old96;
			ctx->interrupt = &interruptCtx_Old96;
			ctx->conflictResolution = &conflictCtx_Old96;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(11, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_Old112;
			ctx->exceptionHandler = &exceptionHandlerCtx_Old112;
			ctx->interrupt = &interruptCtx_Old112;
		}
		ctx->regInterruptBaseAddr = 0xfffee000;
	}
	IoBaseScu = ctx->regInterruptBaseAddr;
}