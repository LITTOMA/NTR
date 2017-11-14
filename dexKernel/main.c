#include "main.h"

dbgPlatformContext * dbgkGetPlatformContext()
{
	static dbgPlatformContext platformCtx;
	return &platformCtx;
}

u32 dbgkSyncCP14()
{
	u8 * SyncStatus = (u8*)0xDFFF4FE0;
	u32 * CP14Args = (u32*)0xDFFF4FE4;
	if (CP14Args)
	{
		u32 type, contextID, wpid, flagLoadStore, ByteAddressSelect, address;
		type = CP14Args[0];
		switch(type)
		{
			case 0:
				dbgkInitHardwareDebugger();
				break;
			case 1:
				contextID = CP14Args[1];
				dbgkSetContextIDToBRP(contextID);
				break;
			case 2:
				wpid = CP14Args[1];
				flagLoadStore = CP14Args[2];
				ByteAddressSelect = CP14Args[3];
				address = CP14Args[4];
				dbgkSetHardwareWatchpoint(wpid, flagLoadStore, ByteAddressSelect, address);
				break;
			case 3:
				wpid = CP14Args[1];
				dbgkDisableHardwareWatchpoint(wpid);
				break;
			default:
				return 1;
		}
	}
	if (SyncStatus)
	{
		*(SyncStatus + dbgkGetCurrentProcessorNumber()) = 1;
		dbgkDataSynchronizationBarrier();
	}
	return 1;
}

u32 dbgkBindInterrupt(u32 * kSchedulerObject)
{
	static u32 dbgkCP14SyncVirtualTable[1];
	static u32 dbgkCP14SyncObject[1];
	static u32 translationTableBase[4];
	dbgPlatformContext * ctx = dbgkGetPlatformContext();
	if (kSchedulerObject && ctx && ctx->interrupt)
	{
		if (ctx->interrupt->schedulerObjectVirtualTable)
			*kSchedulerObject = (u32)ctx->interrupt->schedulerObjectVirtualTable;//restore virtual table
		dbgkCP14SyncVirtualTable[0] = (u32)dbgkSyncCP14;
		dbgkCP14SyncObject[0] = (u32)&dbgkCP14SyncVirtualTable;
		if (ctx->interrupt->bindInterrupt && ctx->interrupt->interruptTableAddr)
		{
			ctx->interrupt->bindInterrupt((void*)ctx->interrupt->interruptTableAddr, &dbgkCP14SyncObject, 12, 
				dbgkGetCurrentProcessorNumber(), 0, 0, 0, 0);
		}
		
	}
	translationTableBase[dbgkGetCurrentProcessorNumber()] = dbgkGetTranslationTableBase();
	return 1;
}

u32 dbgkCheckException(u32 pc)
{
	int flag3 = 1;
	if ((pc & 3) == 0)
	{
		u32 instr = *(u32*)pc;
		flag3 = instr != 0xE1200070;
	}
	dbgPlatformContext * ctx = dbgkGetPlatformContext();
	u32 tls = ctx->dbgGetMainThreadTLS();
	int flag0 = *(u32*)(tls + 0x40) != 0;
	u64 pname = ctx->dbgGetCurrentProcessName();
	int flag1 = pname != 0x756E656D && pname != 0x7465736D;
	return flag0 && flag1 && flag3;
}

void entry(u32 isNew3DS, u32 firmVersion)
{
	dbgPlatformContext * ctx = dbgkGetPlatformContext();
	dbgInitPlatformContext(ctx, isNew3DS, firmVersion);
	kInterruptHax(ctx);
	kExceptionHandlerHax(ctx);
	kResolveConflict(ctx);
}