#define DBG_MAX_BREAKPOINT 32
#define DBG_MAX_HW_BREAKPOINT 4
#define DBG_MAX_WATCHPOINT 2

#define DBG_BKPT_ARM 0xE1200070
#define DBG_BKPT_THUMB 0xBE00

#define DBG_EXCEPTION_ENTRY_BREAKPOINT 1
#define DBG_EXCEPTION_ENTRY_WATCHPOINT 2
#define DBG_EXCEPTION_ENTRY_BKPT 3

#define DBG_MAX_STUBCODE_LEN 128

#define DBG_CMD_RESUME 0x100
#define DBG_CMD_STEP 0x101

//these three structure type from NINTENDO-CTR-SDK
typedef struct
{
	u32 regs[16];
    u32 cpsr;
} dbgExceptionContext;

typedef struct
{
	u32 type;
	u32 fsr;
	u32 far;
	u32 fpexc;
	u32 fpinst;
	u32 fpinst2;
} dbgExceptionInfo;

typedef struct
{
	dbgExceptionInfo info;
	dbgExceptionContext context;
} dbgExceptionBuffer;

typedef struct
{
	u32 address;
	u32 actualInstruction;
	u8 id;
	u8 thumbFlag;
	u8 isEnabled;
	u8 isActive;
	u8 isEncoded;
	u8 stubCode[DBG_MAX_STUBCODE_LEN] __attribute__ ((aligned (4)));
} dbgBreakpointContext;

typedef struct
{
	u32 address;
	u8 id;
	u8 loadstoreFlag;
	u8 byteAddressSelectFlag;
	u8 isEnabled;
	u8 isActive;
	dbgPlatformContext * platformCtx;
} dbgWatchpointContext;

typedef struct
{
	u32 isActive;
	dbgBreakpointContext nextInstrBKPT;
	u32 currentTLS;
	u32 checkCondCodeBuffer[4];
} dbgCMDStepContext;

typedef struct
{
	u8 isInitialized;
	RT_LOCK cmdLock;
	u32 cmd;
	RT_LOCK handleExceptionLock;
	dbgBreakpointContext breakpointList[DBG_MAX_BREAKPOINT];
	dbgWatchpointContext watchpointList[DBG_MAX_WATCHPOINT];
	u8 exceptionStack[0];
	dbgExceptionBuffer exceptionBuffer;
	u8 isfloatRegisterBufferHandled;
	vu32 floatRegisterBuffer[32] __attribute__ ((aligned (4)));
	dbgCMDStepContext cmdStep;
	dbgPlatformContext platformCtx;
} dbgDebuggerContext;

typedef struct
{
	u32 localValueOffset[12];
	u32 * pos;
	u8 regBase;
	u8 regCpsr;
	u8 regUpdateR15Method;
	u8 thumbFlag;
} dbgActualInstructionRecompilerContext;


#define ABS(imm) ((imm) > 0 ? (imm) : - (imm))
#define GETBIT(n) ((u32)(actualIns >> (n)) & 1)    /* bit n of instruction */
#define BITS(m,n) ((u32)(actualIns << (31 - (n))) >> ((31 - (n)) + (m)))    /* bits m to n of instr */
#define TOPBITS(n) (actualIns >> (32 - n))    /* bits 31 to n of instr */
#define ARM$LDR_RD_$RN_IMM$(cond, rd, rn, imm) /* ldr{cond} rd, [rn, #imm] */ \
	(0x5100000 | (cond << 28) | ((imm) < 0 ? 0 : 1 << 23) | ((rn) << 16) | ((rd) << 12) | (ABS(imm) & 0xFFF))
#define ARM$STR_RD_$RN_IMM$(cond, rd, rn, imm) /* str{cond} rd, [rn, #imm] */ \
	(0x5000000 | (cond << 28) | ((imm) < 0 ? 0 : 1 << 23) | ((rn) << 16) | ((rd) << 12) | (ABS(imm) & 0xFFF))
#define ARM$MOV_RD_RM(cond, s, rd, rm) \
	(0x1A00000 | (cond << 28) | (s << 20) | (rd << 12) | rm)
#define ARM$MOV_RD_IMM(cond , s , rd , imm) \
	(0x3A00000 | (cond << 28) | (s << 20) | (rd << 12) | imm)
#define ARM$ADD_RD_RN_IMM(cond , s , rd , rn , imm) \
	(0x2800000 | (cond << 28) | (s << 20) | (rn << 16) | (rd << 12) | imm)
#define ARM$ADD_RD_RN_RM(cond , s , rd ,rn ,rm) \
	(0x0800000 | (cond << 28) | (s << 20) | (rn << 16) | (rd << 12) | rm)
#define ARM$BL_IMM(current, target) \
	(0xeb000000 | ((((int)(target) - (int)((current) + 8)) / 4) & 0x00ffffff))

#define ARM_ALU_TYPE_RD_RN_IMM 0
#define ARM_ALU_TYPE_RD_RN_RM_SHIFT_IMM 1
#define ARM_ALU_TYPE_RD_RN_RM_SHIFT_RS 2
#define ARM_SDT_TYPE_RD_RN_IMM 0x10
#define ARM_SDT_TYPE_RD_RN_RM_SHIFT_IMM 0x11
#define ARM_LDC_TYPE_CRD_RN_IMM 0x20

#define DBG_UPDATE_R15_METHOD_NOPE 0
#define DBG_UPDATE_R15_METHOD_ARM 1
#define DBG_UPDATE_R15_METHOD_THUMB 2

//#define __inline __attribute__((always_inline)) 
void dbgHandleDebuggerControl(NS_PACKET * pac);

void dbgHandleException(dbgExceptionInfo * exceptionInfo , dbgExceptionContext * exceptionContext);
void dbgStubHandleException(dbgExceptionInfo * exceptionInfo , dbgExceptionContext * exceptionContext);
void dbgReturnFromExceptionDirectly(dbgExceptionContext * exceptionContext);

u32 dbgGetCurrentContextID();
void dbgSetContextIDToBRP(u32 contextID);
//u32 dbgGetMainThreadTLS();
u32 dbgGetMainThreadTLS_Old3ds80();
u32 dbgGetMainThreadTLS_Old3ds();
u32 dbgGetMainThreadTLS_New3ds();
u32 dbgGetCurrentTLS();

void dbgInitHardwareDebugger();
void dbgSetHardwareWatchpoint(u8 id , u8 loadstoreFlag , u8 byteAddressSelectFlag , u32 address);
void dbgDisableHardwareWatchpoint(u8 id);
void dbgSetHardwareBreakpoint(u8 id , u8 byteAddressSelectFlag , u32 address);
void dbgDisableHardwareBreakpoint(u8 id);

void dbgGetAllFloatRegister(vu32 * buffer);
u32 dbgGetWatchpointFaultAddressRegister();
u32 dbgGetDebugStatusAndControlRegister();

void dbgSyncInitDebugger(dbgPlatformContext * ctx);
void dbgSyncContextID(dbgPlatformContext * ctx, u32 id);
void dbgSyncSetHardwareWatchpoint(dbgPlatformContext * ctx, u8 bpid, u8 flagLoadStore, u8 ByteAddressSelect, u32 address);
void dbgSyncDisableHardwareWatchpoint(dbgPlatformContext * ctx, u32 bpid);

void dbgExceptionHandlerHax(dbgPlatformContext * ctx, u32 contextID, u32 funcHandleException);
void dbgInterruptHax(dbgPlatformContext * ctx, u32 interruptID);

void dbgFlushDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
void dbgFlushInstructionCache(dbgPlatformContext * ctx, u32 address , u32 size);
void dbgStoreDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
void dbgInvalidateDataCache(dbgPlatformContext * ctx, u32 address , u32 size);

u32 dbgGetCurrentProcessorNumber();