.arm
.align(4);

.global sleep
.type sleep, %function	
sleep:
SVC		0xA
BX		LR


.global backdoorHandler
.type backdoorHandler, %function	
backdoorHandler:
cpsid	aif
STMFD   SP!, {R3-R11,LR}
bl		kernelCallback
LDMFD   SP!, {R3-R11,PC}


.global InvalidateEntireInstructionCache
.type InvalidateEntireInstructionCache, %function
InvalidateEntireInstructionCache:
	mov r0, #0
	mcr p15, 0, r0, c7, c5, 0
	bx lr

.global InvalidateEntireDataCache
.type InvalidateEntireDataCache, %function
InvalidateEntireDataCache:
mov r0, #0
mcr p15, 0, r0, c7, c14, 0 @Clean and Invalidate Entire Data Cache
mcr p15, 0, r0, c7, c10, 0
mcr p15, 0, R0,c7,c10, 4 @Data Synchronization Barrier
mcr p15, 0, R0,c7,c5, 4 @Flush Prefetch Buffer
bx lr

.global nsStubHookEntrypoint
.type nsStubHookEntrypoint, %function
nsStubHookEntrypoint:
	stmfd sp!,{r0-r12,lr}
	mrs r0, cpsr
	stmfd sp!,{r0}
	bl nsHookEntrypoint
	ldmfd sp!,{r0}
	msr cpsr, r0
	ldmfd sp!,{r0-r12,lr}
	ldr pc, =0x100000