#include "global.h"

void dbgInitSynchronization(dbgPlatformContext * ctx)
{
	//dbgInterruptHax(ctx, INTERRTUPT_DEBUGGER_ID);
	svc_sleepThread(10000000);
}

void dbgSyncDebugger(dbgPlatformContext * ctx, u32 syncID, u32* params)
{
	//nsDbgPrint("cpu num = %d\n", dbgGetCpuNumber());
#if 1
	if (syncID >= 4) return;
	nsDbgPrint("sync debugger with other cores\n");
	if (ctx->argsAddr && ctx->syncStatusAddress)
	{
		u32 cp14ArgsAddr = ctx->argsAddr;
		u32 cp14SyncStatus = ctx->syncStatusAddress;
		static u8 argsLength[4] = {0, 1, 4, 1};
		kmemcpy((void*)cp14ArgsAddr, (void*)&syncID, 4);
		if (params)
			kmemcpy((void*)(cp14ArgsAddr + 4), (void*)params, 4 * argsLength[syncID]);
		dbgInvalidateDataCache(ctx, cp14ArgsAddr, 0x14);
		u32 temp = 0;
		do
		{	kmemcpy((void*)cp14SyncStatus, &temp, 4);//init
			dbgInvalidateDataCache(ctx, cp14SyncStatus, 0x4);
			kmemcpy((void*)&temp, (void*)cp14SyncStatus, 4);
		}
		while (temp != 0);
		u32 i = 0;
		u32 cpuCount = dbgGetCpuNumber();
		u32 currentCpu = svc_GetCurrentProcessorNumber();
		for(; i < cpuCount; i++)
		{
			if (i != currentCpu)
				dbgTriggerInterrupt(SOFTWARE_INTERRUPT_FILTER_CPULIST, 1 << i, INTERRTUPT_DEBUGGER_ID);
			svc_sleepThread(10000000);
		}

		//kFlushCache(cp14SyncStatus, 0x4);
		dbgInvalidateDataCache(ctx, cp14SyncStatus, 0x4);

		kmemcpy((void*)&temp, (void*)cp14SyncStatus, 4);
		nsDbgPrint("result of syncing debugger\n");
		for(i = 0; i < cpuCount; i++)
		{
			nsDbgPrint("cpu%d\t%s\n",i ,*(u8*)((u32)&temp + i) == 1 ? "ok" : "failed");
		}
	}
#endif
}

void dbgSyncInitDebugger(dbgPlatformContext * ctx)
{
	//dbgInitSynchronization(ctx);
	dbgSyncDebugger(ctx, 0, 0);
}

void dbgSyncContextID(dbgPlatformContext * ctx, u32 id)
{
	dbgSyncDebugger(ctx, 1, &id);
}

void dbgSyncSetHardwareWatchpoint(dbgPlatformContext * ctx, u8 bpid, u8 flagLoadStore, u8 ByteAddressSelect, u32 address)
{
	u32 args[4] = {bpid, flagLoadStore, ByteAddressSelect, address};
	dbgSyncDebugger(ctx, 2, &args[0]);
}

void dbgSyncDisableHardwareWatchpoint(dbgPlatformContext * ctx, u32 bpid)
{
	dbgSyncDebugger(ctx, 3, &bpid);
}