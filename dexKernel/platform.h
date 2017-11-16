#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))


typedef struct{
	u8* enableFlag0;//these 2 flags must set 1
	u8* enableFlag1;
	
	u32 hookAddr;
	u32 hookAddrK;
	
	u32 offestCheckPatchAddr;
	u32 offestCheckPatchAddrK;
} dbgKernelExceptionHandlerContext;

typedef struct{
	void (*invalidateDataCache)(void *, u32);
	void (*storeDataCache)(void *, u32);
	void (*flushDataCache)(void *, u32);
	void (*flushInstructionCache)(void *, u32);
} dbgKernelCacheInterfaceContext;

typedef struct{
	u32 interruptTableAddr;
	u32 schedulerObjectVtableAddr;
	u32 funcBindInterruptAddr;
} dbgKernelSyncCp14Context;

typedef struct{
	u32 interruptTableAddr;
	u32* schedulerObjectVirtualTable;
	s32 (*bindInterrupt)(void*, void*, u32, u32, u32, u32, u32, u32);
} dbgKernelInterruptContext;

typedef struct{
	u32 prefetchAbortHandler;
	u32 dataAbortHandler;
	u32 exceptionModeStack;
	u32 patchBase;
	u32 patchBaseK;
} dbgConflictResolutionContext;

typedef struct{
	u32 isNew3DS;
	u32 firmVersion;
	
	dbgKernelExceptionHandlerContext * exceptionHandler;
	dbgKernelCacheInterfaceContext * cache;
	dbgKernelInterruptContext * interrupt;
	dbgConflictResolutionContext * conflictResolution;
	
	u32 regInterruptBaseAddr;
	u32 (*dbgGetMainThreadTLS)(void);
	u64 (*dbgGetCurrentProcessName)(void);
} dbgPlatformContext;

extern u32 IoBaseScu;

void dbgInitPlatformContext(dbgPlatformContext * ctx, u32 isNew3DS, u32 firmVersion);
void kInterruptHax(dbgPlatformContext * ctx);
void kExceptionHandlerHax(dbgPlatformContext * ctx);

void kFlushDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
void kFlushInstructionCache(dbgPlatformContext * ctx, u32 address , u32 size);
void kStoreDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
void kInvalidateDataCache(dbgPlatformContext * ctx, u32 address , u32 size);

u32 dbgkGetMainThreadTLS_New3ds();
u32 dbgkGetMainThreadTLS_Old3ds();
u32 dbgkGetMainThreadTLS_Old3ds80();

u64 dbgGetCurrentProcessName_New3ds();
u64 dbgGetCurrentProcessName_Old3ds();
u64 dbgGetCurrentProcessName_Old3ds80();