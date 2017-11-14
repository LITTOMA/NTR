#include "global.h"
#define MAKE_ARM_B(current, target) \
	( 0xea000000 | ((((int)(target)-(int)((current)+8))/4)&0x00ffffff) )

u32 IoBaseScu = 0;

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


dbgKernelMemoryControlContext memoryControlCtx_Old96 = {
	(void*)0xFFF1B978,
	0xFFF30858,
	(u32*)0xDFFE9DA0,
	0xFFF2BDA0,
	0xFFF308EC,
	0xFFF2C2A8
};

dbgKernelMemoryControlContext memoryControlCtx_Old90 = {
	(void*)0xFFF1B63C,
	0xFFF30858,
	(u32*)0xDFFE9830,
	0xFFF2B830,
	0xFFF308EC,
	0xFFF2C0E8
};

dbgKernelMemoryControlContext memoryControlCtx_Old112 = {
	(void*)0xFFF1C0D4,
	0xFFF31858,
	(u32*)0xDFFF4E00,
	0xFFFF0E00,
	0xFFF3191C,
	0xFFF2D2D4
};

dbgKernelMemoryControlContext memoryControlCtx_NEW81 = {
	(void*)0xFFF1B8BC,
	0xFFF31958,
	(u32*)0xDFFEB500,
	0xFFF2C500,
	0xFFF319F4,
	0xFFF2D2A4
};

dbgKernelMemoryControlContext memoryControlCtx_NEW92 = {
	(void*)0xFFF1C298,
	0xFFF31858,
	(u32*)0xDFFEBA30,
	0xFFF2CA30,
	0xFFF318F4,
	0xFFF2D2A8
};

dbgKernelMemoryControlContext memoryControlCtx_NEW95 = {
	(void*)0xFFF1C270,
	0xFFF31858,
	(u32*)0xDFFF4E00,
	0xFFFF0E00,
	0xFFF318F4,
	0xFFF2D2A8
};

dbgKernelMemoryControlContext memoryControlCtx_NEW112 = {
	(void*)0xFFF1C750,
	0xFFF32858,
	(u32*)0xDFFF4E00,
	0xFFFF0E00,
	0xFFF32924,
	0xFFF2E2D4
};

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

s32 kBootDEXKernel(dbgPlatformContext * ctx)
{
	s32 ret = -1;
	if (ctx->memory && ctx->memory->alloc)
	{
		u32 virtualAddress = 0xC8000000;
		u32 pageNum = ((size_dexKernel + 0x3FFF) & ~0x3FFF) >> 12;
		ret = ctx->memory->alloc(ctx->memory->kmemBlockObject, virtualAddress, pageNum, 0,
			0xBC04, 0x3F, 0, 0x300);//controlMemoryUltra(0xFFF30858,0xc8000000,4,0,0xbc04,0x3f,0,0x300)
#if 1
		if (!ret)
		{
			u32 i = 0;
			u32 KSchedulerObjectVtable = ctx->memory->schedulerObjectVtableAddr;
			u32 * injectAddrAWX = ctx->memory->injectAddr;
			u32 injectAddrKernel = ctx->memory->injectAddrK;
			if (injectAddrAWX)
			{
				memcpy(injectAddrAWX , mmuSync, size_mmuSync);
				u32 mmuAddr = 0xFFFF2000 + (virtualAddress >> 20) * 4;
				injectAddrAWX[1] = KSchedulerObjectVtable;
				injectAddrAWX[2] = mmuAddr;
				injectAddrAWX[3] = *(u32*)mmuAddr;//set mmu descriptor
				injectAddrAWX[4] = injectAddrKernel;
				injectAddrAWX[5] = *(u32*)(KSchedulerObjectVtable + 4);
				injectAddrAWX[6] = virtualAddress;
				injectAddrAWX[7] = pageNum;
				u32 interruptTableAddr = ctx->memory->interruptTableAddr;
				for(i = 0; i < size_mmuSync;i += 0x80)
					kFlushDataCache(ctx, (u32)injectAddrAWX + i, 0x80);
				u32 cpuCount = ipiGetCpuNumber();
				u32 currentCpu = dbgGetCurrentProcessorNumber();
				for(i = 0; i < cpuCount; i++)
				{
					if (i == currentCpu) continue;
					u32 * schedulerObject = (u32*)(*(u32*)(interruptTableAddr + 0x100 * i + 8 * 8));
					if(schedulerObject)
						*schedulerObject = injectAddrKernel + 4 * 4;
					dbgDataSynchronizationBarrier();
					dbgDelay(0x100000);
					ipiTriggerInterrupt(SOFTWARE_INTERRUPT_FILTER_CPULIST, 1 << i, 8);
					do
					{
						dbgDelay(0x100000);
					}
					while(*schedulerObject != KSchedulerObjectVtable);
					dbgDelay(0x100000);
				}
				memcpy(virtualAddress, dexKernel, size_dexKernel);
				if (ctx->cache && ctx->cache->flushDataCache)
				{
					ctx->cache->flushDataCache(virtualAddress, pageNum * 0x1000);
				}
				void (*bootDEXKernel)(u32, u32) = (void*)virtualAddress;
				bootDEXKernel(ctx->isNew3DS, ctx->firmVersion);
			}
		}
#endif
	}
	return ret;
}

void dbgInitPlatformContext(dbgPlatformContext * ctx)
{
	memset(ctx, 0, sizeof(dbgPlatformContext));
	ctx->isNew3DS = ntrConfig->isNew3DS;
	ctx->firmVersion = ntrConfig->firmVersion;
	if (ctx->isNew3DS)
	{
		if (ctx->firmVersion == SYSTEM_VERSION(8, 1, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW81;
			ctx->memory = &memoryControlCtx_NEW81;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(9, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW92;
			ctx->memory = &memoryControlCtx_NEW92;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(9, 5, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW95;
			ctx->memory = &memoryControlCtx_NEW95;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(10, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW102;
			ctx->memory = &memoryControlCtx_NEW95;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(11, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_NEW112;
			ctx->memory = &memoryControlCtx_NEW112;
		}
		ctx->dbgGetMainThreadTLS = (void*)dbgGetMainThreadTLS_New3ds;
	}
	else
	{
		//TODO:support old 3ds
		if (ctx->firmVersion >= SYSTEM_VERSION(8, 0, 0))
			ctx->dbgGetMainThreadTLS = (void*)dbgGetMainThreadTLS_Old3ds80;
		else
			ctx->dbgGetMainThreadTLS = (void*)dbgGetMainThreadTLS_Old3ds;
		if (ctx->firmVersion == SYSTEM_VERSION(9, 0, 0))
		{
			ctx->cache = &cacheInterfaceCtx_Old90;
			ctx->memory = &memoryControlCtx_Old90;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(9, 6, 0))
		{
			ctx->cache = &cacheInterfaceCtx_Old96;
			ctx->memory = &memoryControlCtx_Old96;
		}
		else if (ctx->firmVersion == SYSTEM_VERSION(11, 2, 0))
		{
			ctx->cache = &cacheInterfaceCtx_Old112;
			ctx->memory = &memoryControlCtx_Old112;
		}
	}
	ctx->stateAddr = 0xDFFF4FF8;
	ctx->argsAddr = 0xDFFF4FE4;
	ctx->syncStatusAddress = 0xDFFF4FE0;
	ctx->regInterruptBaseAddr = 0xfffee000;
	IoBaseScu = ctx->regInterruptBaseAddr;
}
