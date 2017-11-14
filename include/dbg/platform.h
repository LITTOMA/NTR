#define SYSTEM_VERSION(major, minor, revision) \
	(((major)<<24)|((minor)<<16)|((revision)<<8))


typedef struct{
	void (*invalidateDataCache)(void *, u32);
	void (*storeDataCache)(void *, u32);
	void (*flushDataCache)(void *, u32);
	void (*flushInstructionCache)(void *, u32);
} dbgKernelCacheInterfaceContext;

typedef struct{
	s32 (*alloc)(void*, u32, u32, u32, u32, u32, u32, u32);
	void * kmemBlockObject;
	
	u32 * injectAddr;
	u32 injectAddrK;
	
	u32 interruptTableAddr;
	u32 schedulerObjectVtableAddr;
} dbgKernelMemoryControlContext;

typedef struct{
	u32 isNew3DS;
	u32 firmVersion;
	
	u32 stateAddr;
	u32 argsAddr;
	u32 syncStatusAddress;
	dbgKernelCacheInterfaceContext * cache;
	dbgKernelMemoryControlContext * memory;
	
	u32 regInterruptBaseAddr;
	u32 (*dbgGetMainThreadTLS)(void);
} dbgPlatformContext;

extern u32 IoBaseScu;

void dbgInitPlatformContext(dbgPlatformContext * ctx);

void kFlushDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
void kFlushInstructionCache(dbgPlatformContext * ctx, u32 address , u32 size);
void kStoreDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
void kInvalidateDataCache(dbgPlatformContext * ctx, u32 address , u32 size);
