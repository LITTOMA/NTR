#include "global.h"

dbgDebuggerContext dbgCtx;

static inline s32 dbgGetbpid(dbgDebuggerContext * ctx , u32 address , u32 thumbFlag)
{
	u32 instr = thumbFlag ? *(u16*)address : *(u32*)address;
	if (!thumbFlag && (instr & 0xFFF000F0) != DBG_BKPT_ARM)
	{
		return -1;
	}
	else if (thumbFlag && (instr & 0xFF00) != DBG_BKPT_THUMB)
	{
		return -1;
	}
	u32 bpid = thumbFlag ? (instr & 0xFF) : ((instr & 0xF) | ((instr & 0xFFF00) >> 4));
	if (bpid == 0) return -1;
	return bpid - 1;
}

void dbgEnableBreakpoint(dbgBreakpointContext * ctx)
{
	u32 ret;
	if(!ctx->isActive)
	{
		nsDbgPrint("this breakpoint is not used\n");
		return;
	}
	else if (ctx->isEnabled)
	{
		nsDbgPrint("this breakpoint was already enabled!!\n");
		return;
	}
	rtCheckRemoteMemoryRegionSafeForWrite(getCurrentProcessHandle() , ctx->address , ctx->thumbFlag ? 2 : 4);
	if (ret != 0)
	{
		nsDbgPrint("rtCheckRemoteMemoryRegionSafeForWrite failed ret = %08x\n" , ret);
		return;
	}
	u32 bpid = ctx->id + 1;
	if (ctx->thumbFlag)
		*(u16*)ctx->address = (u16)(DBG_BKPT_THUMB | (bpid & 0xFF));
	else
		*(u32*)ctx->address = (u32)(DBG_BKPT_ARM | (bpid & 0xF) | ((bpid & 0xFFF0) << 4));
	svc_flushProcessDataCache((Handle)0xFFFF8001 , ctx->address , ctx->thumbFlag ? 2 : 4);
	ctx->isEnabled = 1;
}

void dbgDisableBreakpoint(dbgBreakpointContext * ctx)
{
	u32 ret;
	if(!ctx->isActive)
	{
		nsDbgPrint("this breakpoint is not used!!\n");
		return;
	}
	else if (!ctx->isEnabled)
	{
		nsDbgPrint("this breakpoint was already disabled!!\n");
		return;
	}
	ret = rtCheckRemoteMemoryRegionSafeForWrite(getCurrentProcessHandle() , ctx->address , ctx->thumbFlag ? 2 : 4);
	if (ret != 0)
	{
		nsDbgPrint("rtCheckRemoteMemoryRegionSafeForWrite failed ret = %08x\n" , ret);
		return;
	}
	if (ctx->thumbFlag)
		*(u16*)ctx->address = (u16)ctx->actualInstruction;
	else
		*(u32*)ctx->address = ctx->actualInstruction;
	nsDbgPrint("disable the breakpoint\n");
	svc_flushProcessDataCache((Handle)0xFFFF8001 , ctx->address , ctx->thumbFlag ? 2 : 4);
	ctx->isEnabled = 0;
}

void dbgInitBreakpoint(dbgBreakpointContext * ctx , u8 id , u32 address , u8 thumbFlag)
{
	memset(ctx , 0 , sizeof(dbgBreakpointContext));
	ctx->isActive = 1;
	ctx->address = address & ~(thumbFlag ? 1 : 3);
	ctx->id = id;
	ctx->thumbFlag = thumbFlag;
	ctx->actualInstruction = ctx->thumbFlag ? *(u16*)ctx->address : *(u32*)ctx->address;
	//dbgEnableBreakpoint(ctx);
}

s32 dbgFindFreeBreakpoint(dbgDebuggerContext * ctx)
{
	s32 index = 0;
	while(index < DBG_MAX_BREAKPOINT)
	{
		if(!ctx->breakpointList[index].isActive) return index;
		index++;
	}
	return -1;
}

void dbgAddBreakpoint(dbgDebuggerContext * ctx , u32 address , u8 thumbFlag)
{
	address = address & ~(thumbFlag ? 1 : 3);
	u32 i = 0;
	while (i < DBG_MAX_BREAKPOINT)
	{
		if(ctx->breakpointList[i].isActive && ctx->breakpointList[i].address == address)
		{
			nsDbgPrint("you have already add this breakpoint\n");
			return;
		}
		i++;
	}
	s32 freeIndex = dbgFindFreeBreakpoint(ctx);
	if(freeIndex >= 0)
	{
		nsDbgPrint("add breakpoint at %08x\n" , address);
		dbgInitBreakpoint(&ctx->breakpointList[freeIndex] , (u8)freeIndex , address , thumbFlag);
		dbgEnableBreakpoint(&ctx->breakpointList[freeIndex]);
	}
	else
	{
		nsDbgPrint("you have added too much breakpoint!!\n");
	}
}

void dbgRemoveBreakpoint(dbgDebuggerContext * ctx , u32 id)
{
	if(id < DBG_MAX_BREAKPOINT && ctx->breakpointList[id].isActive)
	{
		if(ctx->breakpointList[id].isEnabled) dbgDisableBreakpoint(&ctx->breakpointList[id]);
		ctx->breakpointList[id].isActive = 0;
		nsDbgPrint("the breakpoint %d is removed\n" , id);
	}
}

void dbgEnableHWBreakpoint(dbgBreakpointContext * ctx)
{
	if (ctx->id >= 4 && ctx->isEnabled) return;
	dbgSetHardwareBreakpoint(ctx->id , ctx->thumbFlag ? (0x3 << (ctx->address & 0x3)) : 0xF , ctx->address);
	ctx->isEnabled = 1;
}

void dbgDisableHWBreakpoint(dbgBreakpointContext * ctx)
{
	if (ctx->id >= 4 && !ctx->isEnabled) return;
	dbgDisableHardwareBreakpoint(ctx->id);
	ctx->isEnabled = 0;
}

void dbgEnableWatchpoint(dbgWatchpointContext * ctx)
{
	if (!ctx->isActive)
	{
		nsDbgPrint("this watchpoint is not used!!\n");
		return;
	}
	else if (ctx->isEnabled)
	{
		nsDbgPrint("this watchpoint was already enabled!!\n");
		return;
	}
	dbgSetHardwareWatchpoint(ctx->id , ctx->loadstoreFlag , ctx->byteAddressSelectFlag , ctx->address);
	dbgSyncSetHardwareWatchpoint(ctx->platformCtx,ctx->id , ctx->loadstoreFlag , ctx->byteAddressSelectFlag , ctx->address);
	ctx->isEnabled = 1;
}

void dbgDisableWatchpoint(dbgWatchpointContext * ctx)
{
	if (!ctx->isActive)
	{
		nsDbgPrint("this watchpoint is not used!!\n");
		return;
	}
	else if (!ctx->isEnabled)
	{
		nsDbgPrint("this watchpoint was already disabled!!\n");
		return;
	}
	nsDbgPrint("disable the watchpoint\n");
	dbgDisableHardwareWatchpoint(ctx->id);
	dbgSyncDisableHardwareWatchpoint(ctx->platformCtx, ctx->id);
	ctx->isEnabled = 0;
}

void dbgInitWatchpoint(dbgWatchpointContext * ctx , u8 id , u32 address ,u8 loadstoreFlag , u8 byteAddressSelectFlag)
{
	memset(ctx , 0 , sizeof(dbgWatchpointContext));
	ctx->id = id & 1;
	ctx->address = address & ~3;
	ctx->loadstoreFlag = loadstoreFlag & 0x3;
	ctx->byteAddressSelectFlag = byteAddressSelectFlag & 0xf;
	ctx->isActive = 1;
	//dbgEnableWatchpoint(ctx);
}

s32 dbgFindFreeWatchpoint(dbgDebuggerContext * ctx)
{
	s32 index = 0;
	while (index < DBG_MAX_WATCHPOINT)
	{
		if(!ctx->watchpointList[index].isActive) return index;
		index++;
	}
	return -1;
}

void dbgAddWatchpoint(dbgDebuggerContext * ctx , u32 address , u8 loadstoreFlag , u8 byteAddressSelectFlag)
{
	address = address & ~3;
	u32 i = 0;
	while (i < DBG_MAX_WATCHPOINT)
	{
		if(ctx->watchpointList[i].isActive && ctx->watchpointList[i].address == address)
		{
			nsDbgPrint("you have already add this watchpoint\n");
			return;
		}
		i++;
	}
	s32 freeIndex = dbgFindFreeWatchpoint(ctx);
	if (freeIndex >= 0)
	{
		nsDbgPrint("add watchpoint at %08x\n" , address);
		dbgInitWatchpoint(&ctx->watchpointList[freeIndex] , (u8)freeIndex , address , loadstoreFlag , byteAddressSelectFlag);
		ctx->watchpointList[freeIndex].platformCtx = &ctx->platformCtx;
		dbgEnableWatchpoint(&ctx->watchpointList[freeIndex]);
	}
	else
	{
		nsDbgPrint("you have added too much watchpoint!!\n");
	}
}

void dbgRemoveWatchpoint(dbgDebuggerContext * ctx , u32 id)
{
	if(id < DBG_MAX_WATCHPOINT && ctx->watchpointList[id].isActive)
	{
		if (ctx->watchpointList[id].isEnabled) dbgDisableWatchpoint(&ctx->watchpointList[id]);
		memset(&ctx->watchpointList[id] , 0 , sizeof(dbgWatchpointContext));
		nsDbgPrint("the watchpoint %d is removed\n" , id);
	}
}

void dbgSetUserExceptionHandler(dbgDebuggerContext * ctx , u8 * mainThreadTLS)
{
	*(u32*)(mainThreadTLS + 0x40) = (u32)dbgStubHandleException;
	*(u32*)(mainThreadTLS + 0x44) = (u32)&ctx->exceptionStack + sizeof(dbgCtx.exceptionStack);
	*(u32*)(mainThreadTLS + 0x48) = (u32)&ctx->exceptionBuffer;
}

void dbgInitDebugger(dbgDebuggerContext * ctx)
{
	u32 currentContextID , mainTLS;
	mainTLS = 0;
	//void * pdbgStubHandleException = &dbgStubHandleException;
	nsDbgPrint("init software debugger...\n");
	memset(ctx , 0 , sizeof(dbgDebuggerContext));
	ctx->isInitialized = 1;
	nsDbgPrint("init hardware debugger...\n");
	dbgInitHardwareDebugger();
	dbgInitPlatformContext(&ctx->platformCtx);
	currentContextID = dbgGetCurrentContextID();
	nsDbgPrint("current context id = %08x\n" , currentContextID);
	dbgSetContextIDToBRP(currentContextID);
	if (ctx->platformCtx.dbgGetMainThreadTLS)
		mainTLS = ctx->platformCtx.dbgGetMainThreadTLS();
	nsDbgPrint("address of main thread's TLS is %08x\n" , mainTLS);
	if (mainTLS)
		dbgSetUserExceptionHandler(ctx, (u8*)mainTLS);
	//dbgExceptionHandlerHax(&ctx->platformCtx, currentContextID, (u32)dbgStubHandleException);
	u32 exceptionHandler[2] = {currentContextID, (u32)dbgStubHandleException};
	kmemcpy((void*)ctx->platformCtx.stateAddr, &exceptionHandler, 8);
	dbgFlushDataCache(&ctx->platformCtx, ctx->platformCtx.stateAddr , 8);
	
#if 0
	if (FIRMVersion == 92 || FIRMVersion == 81)
	{
		kmemcpy((void*)(kernelBase + 0x74000 + 0xFFFF0FF8 - 0xFFFF0000) , &currentContextID , 4);
		kmemcpy((void*)(kernelBase + 0x74000 + 0xFFFF0FFC - 0xFFFF0000) , &pdbgStubHandleException , 4);
		kFlushCache(kernelBase + 0x74000 + 0xFFFF0FF8 - 0xFFFF0000 , 8);
	}
#endif
	rtInitLock(&ctx->cmdLock);
	rtInitLock(&ctx->handleExceptionLock);
	dbgSyncInitDebugger(&ctx->platformCtx);
	dbgSyncContextID(&ctx->platformCtx, currentContextID);
}

void dbgHandleDebuggerControl(NS_PACKET * pac)
{
	u32 id , address , thumbFlag , loadstoreFlag , byteAddressSelectFlag , cmd , method;
	method = pac->args[2] & ~0x100;
	if(!dbgCtx.isInitialized && method != 0xFF)
	{
		dbgInitDebugger(&dbgCtx);
	}
	if (method == 1)
	{
		address = pac->args[0];
		thumbFlag = pac->args[1];
		dbgAddBreakpoint(&dbgCtx , address , thumbFlag);
	}
	else if (method == 2)
	{
		id = pac->args[0];
		if(id < DBG_MAX_BREAKPOINT)
			dbgEnableBreakpoint(&dbgCtx.breakpointList[id]);
	}
	else if (method == 3)
	{
		id = pac->args[0];
		if(id < DBG_MAX_BREAKPOINT)
			dbgDisableBreakpoint(&dbgCtx.breakpointList[id]);
	}
	else if (method == 4)
	{
		if (dbgCtx.handleExceptionLock.value)
		{
			nsDbgPrint("please resume the process before you delete the breakpoint\n");
			return;
		}
		id = pac->args[0];
		if(id < DBG_MAX_BREAKPOINT && !dbgCtx.handleExceptionLock.value)
			dbgRemoveBreakpoint(&dbgCtx , id);
	}
	else if (method == 0x21)
	{
		address = pac->args[0];
		loadstoreFlag = pac->args[1];
		byteAddressSelectFlag = pac->args[3];
		dbgAddWatchpoint(&dbgCtx , address , loadstoreFlag , byteAddressSelectFlag);
	}
	else if (method == 0x22)
	{
		id = pac->args[0];
		if (id < DBG_MAX_WATCHPOINT)
			dbgEnableWatchpoint(&dbgCtx.watchpointList[id]);
	}
	else if (method == 0x23)
	{
		id = pac->args[0];
		if (id < DBG_MAX_WATCHPOINT)
			dbgDisableWatchpoint(&dbgCtx.watchpointList[id]);
	}
	else if (method == 0x24)
	{
		if (dbgCtx.handleExceptionLock.value)
		{
			nsDbgPrint("please resume the process before you delete the watchpoint\n");
			return;
		}
		id = pac->args[0];
		if (id < DBG_MAX_WATCHPOINT && !dbgCtx.handleExceptionLock.value)
			dbgRemoveWatchpoint(&dbgCtx , id);
	}
	else if (method == 0x41)
	{
		cmd = pac->args[0];
		rtAcquireLock(&dbgCtx.cmdLock);
		dbgCtx.cmd = cmd;
		rtReleaseLock(&dbgCtx.cmdLock);
	}
	else if (method == 0x61 || method == 0x62)
	{
		if (dbgCtx.isfloatRegisterBufferHandled)
		{
			pac->dataLen = 4 * 32;
			nsSendPacketHeader();
			nsSendPacketData((u8*)dbgCtx.floatRegisterBuffer , 4 * 32);
		}
	}
	else if (method == 0x63)
	{
		u32 i = 0;
		while (i < DBG_MAX_BREAKPOINT)
		{
			if (dbgCtx.breakpointList[i].isActive) break;
			i++;
		}
		if (i >= DBG_MAX_BREAKPOINT) return;
		nsDbgPrint("list of breakpoint\n");
		nsDbgPrint("bpid\taddress  \tstatus\n");
		i = 0;
		while (i < DBG_MAX_BREAKPOINT)
		{
			if (dbgCtx.breakpointList[i].isActive)
			{
				dbgBreakpointContext * bkptctx = &dbgCtx.breakpointList[i];
				nsDbgPrint("%d\t%08x\t%s\n" , i , bkptctx->address , bkptctx->isEnabled ? "enabled" : "disabled");
			}
			i++;
		}
	}
	else if (method == 0x64)
	{
		u32 i = 0;
		while (i < DBG_MAX_WATCHPOINT)
		{
			if (dbgCtx.watchpointList[i].isActive) break;
			i++;
		}
		if (i >= DBG_MAX_WATCHPOINT) return;
		nsDbgPrint("list of watchpoint\n");
		nsDbgPrint("wpid\taddress  \tstatus\n");
		i = 0;
		while (i < DBG_MAX_WATCHPOINT)
		{
			if (dbgCtx.watchpointList[i].isActive)
			{
				dbgWatchpointContext * wpctx = &dbgCtx.watchpointList[i];
				nsDbgPrint("%d\t%08x\t%s\n" , i , wpctx->address , wpctx->isEnabled ? "enabled" : "disabled");
			}
			i++;
		}
	}
	else if (method == 0xFF)
	{
		//dummy command
		//nsDbgPrint("ControlMemoryUltra test\n");
		//u32 * args = &pac->args;
		//u32 result = dbgMemoryControl(args[0], args[1], args[3], args[4], args[5], args[6], args[7], args[8]);
		/*
		dbgPlatformContext ctx;
		dbgInitPlatformContext(&ctx);
		u32 result = dbgBootDEXKernel(&ctx);
		nsDbgPrint("result = %08x\n", result);
		u32 * args = &pac->args;
		s32 ret = FSUSER_SetContentSeed(fsUserHandle, args[0], args[1], (u8*)&args[3]);
		nsDbgPrint("result = %08x\n", ret);
		*/
	}
}

void dbgPrintExceptionContext(dbgExceptionContext * exceptionContext)
{
	s32 i = 0;
	while (i <= 12)
	{
		if (i && (i & 3) == 0)
			nsDbgPrint("\n");
		nsDbgPrint("r%d = %08x ", i , exceptionContext->regs[i]);
		i++;
	}
	nsDbgPrint("sp = %08x " , exceptionContext->regs[i++]);
	nsDbgPrint("lr = %08x " , exceptionContext->regs[i++]);
	nsDbgPrint("pc = %08x \n" , exceptionContext->regs[i++]);
	nsDbgPrint("cpsr = %08x \n" , exceptionContext->cpsr);
}

void dbgGetFloatRegister(dbgDebuggerContext * ctx)
{
	dbgGetAllFloatRegister(ctx->floatRegisterBuffer);
	ctx->isfloatRegisterBufferHandled = 1;
}

void dbgUpdateR15Callback(dbgExceptionContext * ctx , u32 flagUpdatePC)
{
	if (flagUpdatePC == DBG_UPDATE_R15_METHOD_ARM)
	{
		ctx->regs[15] = ctx->regs[15] + 4;
	}
	else if (flagUpdatePC == DBG_UPDATE_R15_METHOD_THUMB)
	{
		ctx->regs[15] = ctx->regs[15] + 3;
	}
}

static inline void dbgRecompilerLoadCPSR(dbgActualInstructionRecompilerContext * ctx)
{
	*ctx->pos++ = ARM$LDR_RD_$RN_IMM$(0xE , ctx->regCpsr , ctx->regBase , 0x40);//ldr r10,[r11, #0x40]
	*ctx->pos++ = 0xE129F000 | ctx->regCpsr;//msr cpsr , r10
}

static inline void dbgRecompilerStoreCPSR(dbgActualInstructionRecompilerContext * ctx)
{
	*ctx->pos++ = 0xE10F0000 | (ctx->regCpsr << 12);//mrs r10 , cpsr
	*ctx->pos++ = ARM$STR_RD_$RN_IMM$(0xE , ctx->regCpsr , ctx->regBase , 0x40);//str r10,[r11, #0x40]
}

static inline void dbgRecompilerAddLocalValue(dbgActualInstructionRecompilerContext * ctx , u32 reg , u32 num)
{
	*ctx->pos++ = ARM$ADD_RD_RN_IMM(0xE , 0 , reg , reg , num);
}

static inline void dbgRecompilerLoadLocalValue(dbgActualInstructionRecompilerContext * ctx , u32 reg)
{
	*ctx->pos++ = ARM$LDR_RD_$RN_IMM$(0xE , reg , ctx->regBase , ctx->localValueOffset[reg]);
}

static inline void dbgRecompilerStoreLocalValue(dbgActualInstructionRecompilerContext * ctx , u32 reg)
{
	*ctx->pos++ = ARM$STR_RD_$RN_IMM$(0xE , reg , ctx->regBase , ctx->localValueOffset[reg]);
}

static inline void dbgRecompilerInitLocalValue(dbgActualInstructionRecompilerContext * ctx , u8 reg , u8 offset)
{
	ctx->localValueOffset[reg] = offset * 4; 
}

static inline void dbgRecompilerInjectInstr(dbgActualInstructionRecompilerContext * ctx , u32 actualIns , u32 rd , u32 rn , u32 rm , u32 rs , u32 type)
{
	u32 flagUpdatePC , cond , opcode;
	flagUpdatePC = 0;
	cond = BITS(28 , 31);
	if (type == ARM_ALU_TYPE_RD_RN_IMM || type == ARM_ALU_TYPE_RD_RN_RM_SHIFT_IMM || type == ARM_ALU_TYPE_RD_RN_RM_SHIFT_RS)
	{
		opcode = BITS(21 , 24);
		if (opcode < 8 || opcode > 0xB)
		{
			flagUpdatePC = BITS(12 , 15) == 0xF ? 1 : 0; 
		}
	}
	else if (type == ARM_SDT_TYPE_RD_RN_IMM || type == ARM_SDT_TYPE_RD_RN_RM_SHIFT_IMM)
	{
		flagUpdatePC = BITS(12 , 15) == 0xF ? 1 : 0;
	}
	dbgRecompilerLoadCPSR(ctx);
	if (flagUpdatePC)
	{
		*ctx->pos++ = ARM$MOV_RD_IMM(cond , 0 , ctx->regUpdateR15Method , DBG_UPDATE_R15_METHOD_NOPE);//mov{cond} r12, #1
	}
	if (type == ARM_ALU_TYPE_RD_RN_IMM || type == ARM_SDT_TYPE_RD_RN_IMM)
	{
		actualIns = actualIns & ~0xFF000;
		actualIns = actualIns | (rd << 12) | (rn << 16);
	}
	else if (type == ARM_ALU_TYPE_RD_RN_RM_SHIFT_IMM || type == ARM_SDT_TYPE_RD_RN_RM_SHIFT_IMM)
	{
		actualIns = actualIns & ~0xFF00F;
		actualIns = actualIns | (rd << 12) | (rn << 16) | rm;
	}
	else if (type == ARM_ALU_TYPE_RD_RN_RM_SHIFT_RS)
	{
		actualIns = actualIns & ~0xFFF0F;
		actualIns = actualIns | (rd << 12) | (rn << 16) | (rs << 8) | rm;
	}
	else if (type == ARM_LDC_TYPE_CRD_RN_IMM)
	{
		actualIns = actualIns & ~0xF0000;
		actualIns = actualIns | (rn << 16);
	}
	*ctx->pos++ = actualIns;
	dbgRecompilerStoreCPSR(ctx);
}

static inline void dbgRecompilerCallFunction(dbgActualInstructionRecompilerContext * ctx , u32 * parameters , u32 num , u32 funcAddr)
{
	u32 i = 0;
	while (i < num)
	{
		if (i < 4)
		{
			*ctx->pos++ = ARM$MOV_RD_RM(0xE , 0 , i , parameters[i]);
		}
		else if (i >= 4)
		{
			*ctx->pos++ = ARM$STR_RD_$RN_IMM$(0xE , parameters[i] , 0x13 , (i - 4) * 4);
		}
		i++;
	}
	u32 currentPC = (u32)ctx->pos;
	*ctx->pos++ = ARM$BL_IMM(currentPC , funcAddr);
}

static inline void dbgRecompilerFinish(dbgActualInstructionRecompilerContext * ctx)
{
	u32 parameters[2];
	parameters[0] = ctx->regBase;
	parameters[1] = ctx->regUpdateR15Method;
	dbgRecompilerCallFunction(ctx , parameters , 2 , (u32)dbgUpdateR15Callback);
	parameters[0] = ctx->regBase;
	dbgRecompilerCallFunction(ctx , parameters , 1 , (u32)dbgReturnFromExceptionDirectly);
}

static inline void dbgInitRecompiler(dbgActualInstructionRecompilerContext * ctx , u32 * pos , u8 thumbFlag)
{
	memset(ctx , 0 , sizeof(dbgActualInstructionRecompilerContext));
	ctx->regCpsr = 10;
	ctx->regBase = 11;
	ctx->regUpdateR15Method = 12;
	ctx->thumbFlag = thumbFlag;
	ctx->pos = pos;
	*ctx->pos++ = ARM$MOV_RD_RM(0xE , 0 , ctx->regBase , 0);//mov r11, r0
	if(ctx->thumbFlag)
		*ctx->pos++ = ARM$MOV_RD_IMM(0xE , 0 , ctx->regUpdateR15Method , DBG_UPDATE_R15_METHOD_THUMB);//mov r12, #2
	else
		*ctx->pos++ = ARM$MOV_RD_IMM(0xE , 0 , ctx->regUpdateR15Method , DBG_UPDATE_R15_METHOD_ARM);//mov r12, #1
}

void dbgExecuteActualInstruction(dbgBreakpointContext * ctx , dbgExceptionContext * exceptionContext)
{
	/* in common
			ldr sp, [r0, #0x34] @load sp
			ldr r1, [r0, #0x38] @load lr
			str r1, [sp, #-4]!
			mov r2, #0x30
		_store_regs:
			ldr r1, [r0, r2]
			str r1, [sp, #-4]!
			sub r2, r2, #4
			cmp r2, #0
			bge _store_regs
			ldr r1, [r0, #0x40]
			msr cpsr, r1
			ldmfd sp!, {r0-r12, lr}*/
	static const u8 stubCodeCommon[] = {0x34, 0xD0, 0x90, 0xE5, 0x38, 0x10, 0x90, 0xE5, 0x04, 0x10, 0x2D, 0xE5, 0x30, 0x20, 0xA0, 0xE3,
			0x02, 0x10, 0x90, 0xE7, 0x04, 0x10, 0x2D, 0xE5, 0x04, 0x20, 0x42, 0xE2, 0x00, 0x00, 0x52, 0xE3,
			0xFA, 0xFF, 0xFF, 0xAA, 0x40, 0x10, 0x90, 0xE5, 0x01, 0xF0, 0x29, 0xE1, 0xFF, 0x5F, 0xBD, 0xE8};
	u32 actualIns , addrActualInsT , currentPC , *pos , rd , rn , rm , rs , nextIns , opcode;
	s32 imm;
	dbgActualInstructionRecompilerContext recompilerCtx;
	void (*returnFromException)(dbgExceptionContext *) = (void*)&ctx->stubCode;
	actualIns = ctx->actualInstruction;
	currentPC = exceptionContext->regs[15];
	pos = (u32*)(ctx->stubCode);
	nsDbgPrint("current PC = %08x\n" , currentPC);
	nsDbgPrint("pos = %08x\n" , (u32)pos);
	if (ctx->isEncoded) goto executeCode;
	if (ctx->thumbFlag) // for thmub
	{
		if (BITS(11 , 15) == 9)
		{
			/*
			LDR Rd,[PC,#imm8]
			ldr rd,=(current + 4) & ~2 + imm8 * 4
			ldr rd,[rd]
			ldr pc,=nextPC
			(current + 4) & ~2 + imm8 * 4
			nextPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			exceptionContext->cpsr &= ~0x20; // clear T flag
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , BITS(8 , 10) , 15 , 4);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , BITS(8 , 10) , BITS(8 , 10) , 0);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 ,15 , 0);
			*pos++ = ((currentPC + 4) & ~2) + BITS(0 , 7) * 4;
			*pos++ = currentPC + 3;
		}
		else if (BITS(11 , 15) == 20)
		{
			/*
			ADD Rd,PC,#imm8
			same as above
			ldr rd,=(current + 4) & ~2 + imm8 * 4
			ldr pc,=nextPC
			(current + 4) & ~2 + imm8 * 4
			nextPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			exceptionContext->cpsr &= ~0x20; // clear T flag
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , BITS(8 , 10) , 15 , 0);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 0);
			*pos++ = ((currentPC + 4) & ~2) + BITS(0 , 7) * 4;
			*pos++ = currentPC + 3;
		}
		else if (BITS(12 , 15) == 0xD && BITS(8 , 11) < 0xE)
		{
			/*
			conditional branch
			ldr{cond} pc,=branchPC
			ldr pc,=nextPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			exceptionContext->cpsr &= ~0x20; // clear T flag
			*pos++ = ARM$LDR_RD_$RN_IMM$(BITS(8 , 11) , 15 , 15 , 0);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 0);
			*pos++ = currentPC + 4 + ((s32)(BITS(0 , 7) << 24) >> 23) + 1;
			*pos++ = currentPC + 3;
		}
		else if (BITS(11 , 15) == 0x1C)
		{
			/*
			unconditional branch
			ldr pc,=branchPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			exceptionContext->cpsr &= ~0x20; // clear T flag
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , -4);
			*pos++ = currentPC + 4 + ((s32)(BITS(0 , 10) << 21) >> 20) + 1;
		}
		else if (BITS(11 , 15) == 0x1E)
		{
			/*
			first instruction of bl_imm or blx_imm
			ldr lr,=currentPC + 5
			ldr pc,=branchPC
			*/
			nextIns = *(u16*)(ctx->address + 2);
			opcode = nextIns >> 11;
			if (opcode == 0x1F || (opcode == 0x1D && (nextIns & 1) == 0))
			{
				memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
				pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
				exceptionContext->cpsr &= ~0x20; // clear T flag
				*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 14 , 15 , 0);
				*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 0);
				*pos++ = currentPC + 5;
				imm = (s32)(((BITS(0 , 10) << 11) | (nextIns & 0x7FF)) << 10) >> 9;
				if (opcode == 0x1F)
					*pos++ = currentPC + 5 + imm;
				else
					*pos++ = (currentPC + 4 + imm) & ~2;
			}
			else
			{
				//ERROR("unknown instruction\n");
			}
		}
		else if (BITS(8 , 15) == 0x44 && BITS(3 , 6) == 15)
		{
			/*
			add rd, pc, rd
			*/
			exceptionContext->cpsr &= ~0x20; // clear T flag
			rd = BITS(0 , 2) | (GETBIT(7) << 3);
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 1);
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , rd);
			dbgRecompilerInitLocalValue(&recompilerCtx , 1 , 15);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 1);
			dbgRecompilerAddLocalValue(&recompilerCtx , 1 , 4);
			actualIns = ARM$ADD_RD_RN_RM(0xE , 0 , rd , 15 , rd);//convert instruction
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 1 , 0 , 0 , ARM_ALU_TYPE_RD_RN_RM_SHIFT_IMM);
			dbgRecompilerStoreLocalValue(&recompilerCtx , 0);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else if (BITS(8 , 15) == 0x46 && BITS(3 , 6) == 15)
		{
			/*
			mov rd, pc
			*/
			exceptionContext->cpsr &= ~0x20; // clear T flag
			rd = BITS(0 , 2) | (GETBIT(7) << 3);
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , rd , 15 , 0);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 0);
			*pos++ = currentPC + 4;
			*pos++ = currentPC + 3;
		}
		else if (actualIns == 0x4778)
		{
			/*
			bx pc
			*/
			exceptionContext->cpsr &= ~0x20; // clear T flag
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , -4);
			*pos++ = (currentPC + 4) & ~2;
		}
		else if (BITS(7 , 15) == 0x8F)
		{
			/*
			blx reg
			*/
			rs = BITS(3 , 6);
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			exceptionContext->cpsr &= ~0x20; // clear T flag
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 14 , 15 , 0);
			*pos++ = 0xE12FFF10 | rs;
			*pos++ = currentPC + 3;
		}
		else
		{
			/*
			ldr pc, =_actualInstructionT + 1 0
			.thumb
			_actualInstructionT:
			actualInstruction 4
			bx pc
			.arm
			ldr pc,=nextPC 8
			_actualInstructionT + 1 12
			nextPC 16
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			exceptionContext->cpsr &= ~0x20; // clear T flag
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 4);
			addrActualInsT = (u32)pos;
			*pos++ = actualIns | (0x4778 << 16);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 0);
			*pos++ = addrActualInsT + 1;
			*pos++ = currentPC + 3;
		}
	}
	else // for arm
	{
		if ((BITS(24 , 27) == 0xa || BITS(24 , 27) == 0xb) && TOPBITS(4) < 0xF)
		{
			/* 
			branch
			ldr{cond} pc, =branchPC
			ldr pc, =nextPC
			.dword branchPC
			.dword nextPC
			branch link
			ldr{cond} lr, =nextPC
			ldr{cond} pc, =branchPC
			ldr pc, =nextPC
			.dword branchPC
			.dword nextPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			if (BITS(24 , 27) == 0xb) 
				*pos++ = ARM$LDR_RD_$RN_IMM$(TOPBITS(4) , 14 , 15 , 8);
			*pos++ = ARM$LDR_RD_$RN_IMM$(TOPBITS(4) , 15 , 15 , 0);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 ,15 , 0);
			*pos++ = currentPC + 8 + (s32)((s32)(BITS(0 , 23) << 8) >> 6);//sign extend
			*pos++ = currentPC + 4;
		}
		else if (BITS(25 , 31) == 0x7D)
		{		
			/*
			BLX_imm
			ldr lr, =nextPC
			ldr pc, =branchPC
			.dword nextPC.
			.dword branchPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 14 , 15 , 0);
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , 0);
			*pos++ = currentPC + 4;
			*pos++ = currentPC + 8 + (s32)((s32)(BITS(0 , 23) << 8) >> 6) + GETBIT(24) * 2 + 1;//switch to thumb mode
		}
		else if (BITS(4 , 27) == 0x12FFF3)
		{
			/*
			BLX_reg
			ldr{cond} lr, =nextPC
			bx{cond} reg
			ldr sp,=nextPC
			*/
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			*pos++ = ARM$LDR_RD_$RN_IMM$(TOPBITS(4) , 14 , 15 , 4);
			*pos++ = actualIns & ~0x20;
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , -4);
			*pos++ = currentPC + 4;
		}
		else if (BITS(16 , 27) == 0x28F || BITS(16 , 27) == 0x29F || BITS(16 , 27) == 0x24F || BITS(16 , 27) == 0x25F)
		{
			/*
			add rd, pc, #imm
			sub rd, pc, #imm
			*/
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 0);//using r11
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , BITS(12 , 15));
			dbgRecompilerInitLocalValue(&recompilerCtx , 1 , BITS(16 , 19));
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);//ldr r0,[r11 , rd * 4]
			dbgRecompilerLoadLocalValue(&recompilerCtx , 1);//ldr r1,[r11 , rn * 4] in this case r1 loads value of pc 
			dbgRecompilerAddLocalValue(&recompilerCtx , 1 , 8);//add r1,r1,#8
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 1 , 0 , 0 , ARM_ALU_TYPE_RD_RN_IMM);
			dbgRecompilerStoreLocalValue(&recompilerCtx , 0);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else if ((BITS(16 , 27) == 0x8F || BITS(16 , 27) == 0x9F) && GETBIT(4) == 0)
		{
			//add rd , pc , rm shift imm
			//if shift type is 'lsl' and 'imm' is 0 then
			//add rd , pc , rm
			//add rd , pc , pc shift imm is not supported
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 0);
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , BITS(12 , 15));
			dbgRecompilerInitLocalValue(&recompilerCtx , 1 , BITS(16 , 19));
			dbgRecompilerInitLocalValue(&recompilerCtx , 2 , BITS(0 , 3));
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 1);
			dbgRecompilerAddLocalValue(&recompilerCtx , 1 , 8);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 2);
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 1 , 2 , 0 , ARM_ALU_TYPE_RD_RN_RM_SHIFT_IMM);
			dbgRecompilerStoreLocalValue(&recompilerCtx , 0);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else if (BITS(16 , 27) == 0x1A0 && BITS(0 , 4) == 0xF)
		{
			//mov rd , pc
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 0);
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , BITS(12 , 15));
			dbgRecompilerInitLocalValue(&recompilerCtx , 1 , BITS(0 , 3));
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 1);
			dbgRecompilerAddLocalValue(&recompilerCtx , 1 , 8);
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 0 , 1 , 0 , ARM_ALU_TYPE_RD_RN_RM_SHIFT_IMM);//for mov rn is always 0
			dbgRecompilerStoreLocalValue(&recompilerCtx , 0);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else if (BITS(16 , 27) == 0x59F || BITS(16 , 27) == 0x51F)
		{
			/*
			ldr rd, [pc, #imm]
			*/
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 0);
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , BITS(12 , 15));
			dbgRecompilerInitLocalValue(&recompilerCtx , 1 , BITS(16 , 19));
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 1);
			dbgRecompilerAddLocalValue(&recompilerCtx , 1 , 8);
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 1 , 0 , 0 , ARM_SDT_TYPE_RD_RN_IMM);
			dbgRecompilerStoreLocalValue(&recompilerCtx , 0);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else if (BITS(16 , 27) == 0x79F && GETBIT(4) == 0)
		{
			//ldr rd, [pc , rm , shift , imm]
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 0);
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , BITS(12 , 15));//rd
			dbgRecompilerInitLocalValue(&recompilerCtx , 1 , BITS(16 , 19));//rn
			dbgRecompilerInitLocalValue(&recompilerCtx , 2 , BITS(0 , 3));//rm
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 1);
			dbgRecompilerAddLocalValue(&recompilerCtx , 1 , 8);
			dbgRecompilerLoadLocalValue(&recompilerCtx , 2);
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 1 , 2 , 0 , ARM_SDT_TYPE_RD_RN_RM_SHIFT_IMM);
			dbgRecompilerStoreLocalValue(&recompilerCtx , 0);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else if (BITS(24 , 27) == 0xD && (BITS(8 , 11) == 0xa || BITS(8 , 11) == 0xb) && BITS(20 , 21) == 1)
		{
			//FLDS FLDD
			dbgInitRecompiler(&recompilerCtx , (u32*)ctx->stubCode , 0);
			dbgRecompilerInitLocalValue(&recompilerCtx , 0 , BITS(16 , 19));//rn
			dbgRecompilerLoadLocalValue(&recompilerCtx , 0);
			dbgRecompilerAddLocalValue(&recompilerCtx , 0 , 8);
			dbgRecompilerInjectInstr(&recompilerCtx , actualIns , 0 , 0 , 0 , 0 , ARM_LDC_TYPE_CRD_RN_IMM);
			dbgRecompilerFinish(&recompilerCtx);
		}
		else
		{
			//nsDbgPrint("generate stub-code normal\n");
			//nsDbgPrint("pos = %08x\n" , (u32)pos);
			memcpy(pos , &stubCodeCommon , sizeof(stubCodeCommon));
			pos = (u32*)((u32)pos + sizeof(stubCodeCommon));
			*pos++ = actualIns;
			*pos++ = ARM$LDR_RD_$RN_IMM$(0xE , 15 , 15 , -4);
			*pos++ = currentPC + 4;
		}
	}
	ctx->isEncoded = 1;
	svc_flushProcessDataCache(0xFFFF8001 , (u32)&ctx->stubCode , DBG_MAX_STUBCODE_LEN);
executeCode:
	rtReleaseLock(&dbgCtx.handleExceptionLock);
	returnFromException(exceptionContext);
}

u32 dbgCheckCondition(dbgCMDStepContext * ctx , u32 cond , u32 cpsr)
{
	if (cond == 0xF) return 1;
	ctx->checkCondCodeBuffer[2] &= ~0xF0000000;
	ctx->checkCondCodeBuffer[2] = ctx->checkCondCodeBuffer[2] | (cond << 28);
	svc_flushProcessDataCache(0xFFFF8001 , (u32)&ctx->checkCondCodeBuffer , sizeof(ctx->checkCondCodeBuffer));
	u32 (*checkCond)(u32) = (void*)&ctx->checkCondCodeBuffer;
	return checkCond(cpsr);
}

u32 dbgCalcNextPC(dbgCMDStepContext * ctx , dbgExceptionContext * exceptionContext , u32 actualIns , u32 * thumbFlag)
{
	u32 tflag = (exceptionContext->cpsr & 0x20) >> 5;
	u32 nextPC = exceptionContext->regs[15] + (tflag ? 2 : 4);
	u32 currentPC = exceptionContext->regs[15];
	*thumbFlag = tflag;
	nsDbgPrint("calc next pc...\n");
	if (tflag)
	{
		if (BITS(12 , 15) == 0xD && BITS(8 , 11) < 0xE)
		{
			//b{cond}
			if (!dbgCheckCondition(ctx , BITS(8 , 11) , exceptionContext->cpsr & ~0x20)) goto __end;
			nextPC = currentPC + 4 + ((s32)(BITS(0 , 7) << 24) >> 23);
		}
		else if (BITS(11 , 15) == 0x1C)
		{
			//branch
			nextPC = currentPC + 4 + ((s32)(BITS(0 , 10) << 21) >> 20);
		}
		else if (BITS(7 , 15) == 0x8F)
		{
			//blx reg
			u32 rm = BITS(3 , 6);
			nextPC = exceptionContext->regs[rm];
			if (!(nextPC & 1)) *thumbFlag = 0;
		}
		else if (BITS(7 , 15) == 0x8E)
		{
			//bx reg
			u32 rm = BITS(3 , 6);
			nextPC = exceptionContext->regs[rm];
			if (!(nextPC & 1)) *thumbFlag = 0;
		}
		else if (BITS(11 , 15) == 0x1E)
		{
			//first instruction of bl_imm or blx_imm
			u32 nextIns = *(u16*)(currentPC + 2);
			u32 opcode = nextIns >> 11;
			if (opcode == 0x1F || (opcode == 0x1D && (nextIns & 1) == 0))
			{
				u32 imm = (s32)(((BITS(0 , 10) << 11) | (nextIns & 0x7FF)) << 10) >> 9;
				if (opcode == 0x1F)
					nextPC = currentPC + 5 + imm;
				else
				{
					nextPC = (currentPC + 4 + imm) & ~2;
					*thumbFlag = 0;
				}
			}
		}
		else if (BITS(8 , 15) == 0x46 && BITS(0 , 2) == 7 && GETBIT(7))
		{
			//mov pc , rm
			u32 rm = BITS(3 , 6);
			nextPC = exceptionContext->regs[rm];
		}
		else if (BITS(8 , 15) == 0xBD)
		{
			//pop {registerlist,pc}
			u32 off = 0;
			int i = 0;
			while (i < 8)
			{
				if (GETBIT(i)) off += 4;
				i++;
			}
			nextPC = *(u32*)(exceptionContext->regs[13] + off);
			if (!(nextPC & 1)) *thumbFlag = 0;
		}
	}
	else
	{
		if (!dbgCheckCondition(ctx , BITS(28 , 31) , exceptionContext->cpsr & ~0x20))
			goto __end;
		if ((BITS(24 , 27) == 0xa || BITS(24 , 27) == 0xb) && TOPBITS(4) < 0xF)
		{
			//b bl
			nextPC = currentPC + 8 + (s32)((s32)(BITS(0 , 23) << 8) >> 6);//sign extend
			//nsDbgPrint("branch or branch link\n");
		}
		else if (BITS(25 , 31) == 0x7D)
		{
			//blx_imm
			nextPC = currentPC + 8 + (s32)((s32)(BITS(0 , 23) << 8) >> 6) + GETBIT(24) * 2;
			*thumbFlag = 1;// switch to thumb mode
			//nsDbgPrint("branch link exchange imm\n");
		}
		else if (BITS(4 , 27) == 0x12FFF3 || BITS(4 , 27) == 0x12FFF1)
		{
			//blx_reg bx_reg
			u32 reg = BITS(0 , 3);
			nextPC = exceptionContext->regs[reg];
			if (nextPC & 1) *thumbFlag = 1;// switch to thumb mode
			//nsDbgPrint("branch link exchange register\n");
		}
		else if ((BITS(20 , 27) == 0x8 || BITS(20 , 27) == 0x9) && BITS(4 , 6) == 0 && BITS(12 , 15) == 0xF)
		{
			//add pc , rd , rm lsl imm
			u32 rn = BITS(16 , 19);
			u32 rm = BITS(0 , 3);
			u32 imm = BITS(7 , 11);
			if (imm == 0)
			{
				nextPC = exceptionContext->regs[rn] + (rn == 15 ? 8 : 0) + exceptionContext->regs[rm] + (rm == 15 ? 8 : 0);
			}
			else
			{
				nextPC = exceptionContext->regs[rn] + (rn == 15 ? 8 : 0) + ((exceptionContext->regs[rm] + (rm == 15 ? 8 : 0)) << imm);
			}
			//nsDbgPrint("add pc, rd, rm, lsl imm\n");
		}
		else if (BITS(12 , 27) == 0x79FF && BITS(4 , 6) == 0)
		{
			//ldr pc, [pc , rm , lsl , imm]
			u32 rm = BITS(0 , 3);
			u32 imm = BITS(7 , 11);
			if (imm == 0)
			{
				nextPC = *(u32*)(currentPC + 8 + exceptionContext->regs[rm] + (rm == 15 ? 8 : 0));
			}
			else
			{
				nextPC = *(u32*)(currentPC + 8 + ((exceptionContext->regs[rm] + (rm == 15 ? 8 : 0)) << imm));
			}
			if (nextPC & 1) *thumbFlag = 1;// switch to thumb mode
			//nsDbgPrint("ldr pc, [pc, rm, lsl imm]\n");
		}
		else if (BITS(20 , 27) == 0x49 && BITS(12 , 15) == 0xF)
		{
			//ldr pc, [rn],#imm!
			u32 rn = BITS(16 , 19);
			nextPC = *(u32*)(exceptionContext->regs[rn] + (rn == 15 ? 8 : 0));//
			if (nextPC & 1) *thumbFlag = 1;// switch to thumb mode
			//nsDbgPrint("ldr pc, [rn], #imm\n");
		}
		else if ((BITS(20 , 27) == 0x59 || BITS(20 , 27) == 0x51) && BITS(12 , 15) == 0xF)
		{
			//ldr pc, [rn,#imm]
			u32 imm = BITS(0 , 11);
			u32 rn = BITS(16 , 19);
			nextPC = *(u32*)(exceptionContext->regs[rn] + (rn == 15 ? 8 : 0) + (GETBIT(23) ? imm : -imm));
			if (nextPC & 1) *thumbFlag = 1;// switch to thumb mode
			//nsDbgPrint("ldr pc, [rn, #imm]\n");
		}
		else if (BITS(20 , 27) == 0x8B && GETBIT(15))
		{
			//ldmfd rn!, {regList , pc}
			u32 rn = BITS(16 , 19);
			u32 off = 0;
			int i = 0;
			while (i < 15)
			{
				if (GETBIT(i)) off += 4;
				i++;
			}
			nextPC = *(u32*)(exceptionContext->regs[rn] + (rn == 15 ? 8 : 0) + off);
			if (nextPC & 1) *thumbFlag = 1;// switch to thumb mode
			//nsDbgPrint("pop {register-list}\n");
		}
		else if (BITS(4 , 27) == 0x1A0F00)
		{
			u32 rm = BITS(0 , 3);
			nextPC = exceptionContext->regs[rm];
			//nsDbgPrint("mov pc, rm\n");
		}
	}
__end:
	nsDbgPrint("nextPC = %08x\n" , nextPC);
	return nextPC;
}

u32 dbgCheckStepCmdIsAvailable(dbgDebuggerContext * ctx , dbgExceptionContext * exceptionContext)
{
	u32 ret = 1;
	if (ctx->cmdStep.isActive)
	{
		u32 currentTLS = dbgGetCurrentTLS();
		dbgCMDStepContext * stepCtx = &ctx->cmdStep;
		if (stepCtx->currentTLS != currentTLS)//if another thread try to step
		{
			if (exceptionContext->regs[15] == stepCtx->nextInstrBKPT.address)
				dbgExecuteActualInstruction(&stepCtx->nextInstrBKPT , exceptionContext);
			else
				ret = 0;
		}
	}
	return ret;
}

void dbgInitCMDStep(dbgDebuggerContext * ctx , dbgExceptionContext * exceptionContext)
{
	u32 address = exceptionContext->regs[15];
	u32 thumbFlag = (exceptionContext->cpsr & 0x20) >> 5;
	s32 bpid = dbgGetbpid(ctx , address , thumbFlag);
	if (bpid >= 0)//disable breakpoint before using step command
	{
		dbgDisableBreakpoint(&ctx->breakpointList[bpid]);
		nsDbgPrint("disable breakpoint before using step command \nbpid = %08x\n" , bpid);
	}
	u32 instr = thumbFlag ? *(u16*)address : *(u32*)address;
	dbgCMDStepContext * stepCtx = &ctx->cmdStep;
	memset(stepCtx , 0 , sizeof(dbgCMDStepContext));
	stepCtx->currentTLS = dbgGetCurrentTLS();
	stepCtx->isActive = 1;
	stepCtx->checkCondCodeBuffer[0] = 0xE129F000;//msr cpsr, r0
	stepCtx->checkCondCodeBuffer[1] = 0xE3A00000;//mov r0, #0
	stepCtx->checkCondCodeBuffer[2] = 0xE3A00001;//mov r0, #1
	stepCtx->checkCondCodeBuffer[3] = 0xE12FFF1E;//bx lr
	svc_flushProcessDataCache(0xFFFF8001 , (u32)&stepCtx->checkCondCodeBuffer , sizeof(stepCtx->checkCondCodeBuffer));
	u32 nextThumbFlag = 0;
	u32 nextPC = dbgCalcNextPC(stepCtx ,exceptionContext , instr , &nextThumbFlag);
	dbgInitBreakpoint(&stepCtx->nextInstrBKPT , 0 , nextPC , nextThumbFlag);
	dbgEnableHWBreakpoint(&stepCtx->nextInstrBKPT);
}

void dbgHandleCMDStep(dbgDebuggerContext * ctx , dbgExceptionContext * exceptionContext)
{
	if (!dbgCheckStepCmdIsAvailable(ctx , exceptionContext)) return;
	dbgInitCMDStep(ctx , exceptionContext);
	if (exceptionContext->cpsr & 0x20)
	{
		exceptionContext->cpsr &= ~0x20;//clear T flag
		exceptionContext->regs[15] += 1;
	}
	rtReleaseLock(&ctx->handleExceptionLock);
	dbgReturnFromExceptionDirectly(exceptionContext);
}

u32 dbgWaitForDebugCmd(dbgDebuggerContext * ctx)
{
	rtAcquireLock(&ctx->cmdLock);
	ctx->cmd = 0;
	rtReleaseLock(&ctx->cmdLock);
	u32 debuggerCmd;
	while (1)
	{
		rtAcquireLock(&ctx->cmdLock);
		debuggerCmd = ctx->cmd;
		ctx->cmd = 0;
		rtReleaseLock(&ctx->cmdLock);
		if (debuggerCmd == DBG_CMD_RESUME || debuggerCmd == DBG_CMD_STEP)
			break;
		else
			svc_sleepThread(1000000000);
	}
	ctx->isfloatRegisterBufferHandled = 0;
	return debuggerCmd;
}

void dbgHandleDebugEventBKPT(dbgDebuggerContext * ctx , dbgExceptionInfo * exceptionInfo , dbgExceptionContext * exceptionContext)
{
#if 0
	u32 tflag = (exceptionContext->cpsr & 0x20) >> 5;
	u32 instr = tflag ? *(u16*)exceptionContext->regs[15] : *(u32*)exceptionContext->regs[15];
	if (!tflag && (instr & 0xFFF000F0) != DBG_BKPT_ARM)
	{
		rtReleaseLock(&ctx->handleExceptionLock);
		dbgReturnFromExceptionDirectly(exceptionContext);
	}
	else if (tflag && (instr & 0xFF00) != DBG_BKPT_THUMB)
	{
		exceptionContext->cpsr &= ~0x20; // clear T flag
		exceptionContext->regs[15] += 1;
		rtReleaseLock(&ctx->handleExceptionLock);
		dbgReturnFromExceptionDirectly(exceptionContext);
	}
	//u32 id = tflag ? (instr & 0xFF) : ((instr & 0xF) | ((instr & 0xFFF00) >> 4));
#endif
	u32 address = exceptionContext->regs[15];
	u32 thumbFlag = (exceptionContext->cpsr & 0x20) >> 5;
	s32 id = dbgGetbpid(ctx , address , thumbFlag);
	if (id < 0)
	{
		if (!thumbFlag)
		{
			rtReleaseLock(&ctx->handleExceptionLock);
			dbgReturnFromExceptionDirectly(exceptionContext);
		}
		else
		{
			exceptionContext->cpsr &= ~0x20; // clear T flag
			exceptionContext->regs[15] += 1;
			rtReleaseLock(&ctx->handleExceptionLock);
			dbgReturnFromExceptionDirectly(exceptionContext);
		}
	}
	
	u32 debuggerCmd;
	if (id < DBG_MAX_BREAKPOINT && ctx->breakpointList[id].address == exceptionContext->regs[15])
	{
		nsDbgPrint("breakpoint %d is triggered\n" , id);
		dbgPrintExceptionContext(exceptionContext);
		nsDbgPrint("processor that triggered the breakpoint is cpu%d\n", svc_GetCurrentProcessorNumber());
		dbgGetFloatRegister(ctx);
		//TODO: wait for debug command
		debuggerCmd = dbgWaitForDebugCmd(ctx);
		if (debuggerCmd == DBG_CMD_STEP)
		{
			nsDbgPrint("try to step\n");
			if (dbgCheckStepCmdIsAvailable(ctx , exceptionContext))
			{
				dbgHandleCMDStep(ctx , exceptionContext);
			}
			nsDbgPrint("the step command is not available\n");
			debuggerCmd = DBG_CMD_RESUME;
		}
		if (debuggerCmd == DBG_CMD_RESUME)
		{
			nsDbgPrint("try to resume\n");
			dbgExecuteActualInstruction(&ctx->breakpointList[id] , exceptionContext);
		}
	}
	else
	{
		nsDbgPrint("an unknown breakpoint is triggered!!\n");
		dbgPrintExceptionContext(exceptionContext);
		while(true)
			svc_sleepThread(1000000000);
	}
}

void dbgHandleDebugEventWatchpoint(dbgDebuggerContext * ctx , dbgExceptionInfo * exceptionInfo , dbgExceptionContext * exceptionContext)
{
	u32 tflag = (exceptionContext->cpsr & 0x20) >> 5;
	u32 debuggerCmd , wfar;
	wfar = dbgGetWatchpointFaultAddressRegister();
	s32 wpid = 0;
	while (wpid < DBG_MAX_WATCHPOINT)
	{
		if (ctx->watchpointList[wpid].isActive && ctx->watchpointList[wpid].address == exceptionInfo->far) break;
		wpid++;
	}
	if (wpid < DBG_MAX_WATCHPOINT)
		nsDbgPrint("watchpoint %d is triggered\n" , wpid);
	else
		nsDbgPrint("a watchpoint is triggered\n");
	dbgPrintExceptionContext(exceptionContext);
	nsDbgPrint("address of the instruction that hits the watchpoint is %08x\n" , wfar - 8 + tflag * 4);
	nsDbgPrint("processor that triggered the watchpoint is cpu%d\n", svc_GetCurrentProcessorNumber());
	dbgGetFloatRegister(ctx);
	debuggerCmd = dbgWaitForDebugCmd(ctx);
	if (debuggerCmd == DBG_CMD_STEP)
	{
		if (dbgCheckStepCmdIsAvailable(ctx , exceptionContext))
		{
			dbgHandleCMDStep(ctx , exceptionContext);
		}
		else
		nsDbgPrint("the step command is not available\n");
		debuggerCmd = DBG_CMD_RESUME;
	}
	if (debuggerCmd == DBG_CMD_RESUME)
	{
		if (tflag)
		{
			exceptionContext->cpsr &= ~0x20;
			exceptionContext->regs[15] += 1;
		}
		rtReleaseLock(&ctx->handleExceptionLock);
		dbgReturnFromExceptionDirectly(exceptionContext);
	}
}

void dbgHandleDebugEventBreakpoint(dbgDebuggerContext * ctx , dbgExceptionInfo * exceptionInfo , dbgExceptionContext * exceptionContext)
{
	u32 tflag = (exceptionContext->cpsr & 0x20) >> 5;
	u32 debuggerCmd;
	if (!dbgCheckStepCmdIsAvailable(ctx , exceptionContext))
	{
		nsDbgPrint("an error occurred!!\n");
		while(1)
			svc_sleepThread(1000000000);
	}
	dbgPrintExceptionContext(exceptionContext);
	debuggerCmd = dbgWaitForDebugCmd(ctx);
	dbgGetFloatRegister(ctx);
	if (debuggerCmd == DBG_CMD_STEP)
	{
		dbgHandleCMDStep(ctx , exceptionContext);
	}
	else if (debuggerCmd == DBG_CMD_RESUME)
	{
		ctx->cmdStep.isActive = 0;
		dbgDisableHWBreakpoint(&ctx->cmdStep.nextInstrBKPT);
		if (tflag)
		{
			exceptionContext->cpsr &= ~0x20;
			exceptionContext->regs[15] += 1;
		}
		rtReleaseLock(&ctx->handleExceptionLock);
		dbgReturnFromExceptionDirectly(exceptionContext);
	}
}

void dbgHandleException(dbgExceptionInfo * exceptionInfo , dbgExceptionContext * exceptionContext)
{
	rtAcquireLock(&dbgCtx.handleExceptionLock);
	u32 dscr = dbgGetDebugStatusAndControlRegister();
	u32 entry = (dscr >> 2) & 0xf;
	if (entry == DBG_EXCEPTION_ENTRY_BKPT)
	{
		dbgHandleDebugEventBKPT(&dbgCtx , exceptionInfo ,exceptionContext);
	}
	else if (entry == DBG_EXCEPTION_ENTRY_WATCHPOINT)
	{
		dbgHandleDebugEventWatchpoint(&dbgCtx , exceptionInfo , exceptionContext);
	}
	else if (entry == DBG_EXCEPTION_ENTRY_BREAKPOINT)
	{
		dbgHandleDebugEventBreakpoint(&dbgCtx , exceptionInfo , exceptionContext);
	}
	else
	{
		nsDbgPrint("an exception occurred!!\n");
		dbgPrintExceptionContext(exceptionContext);
		while(1)
			svc_sleepThread(1000000000);
	}
}