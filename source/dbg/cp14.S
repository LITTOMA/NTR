#define FUNC(name) \
	.global name; \
	.type name, %function; \
name:

.arm

_dbgGetMainThreadTLS_New3ds:
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	ldr r0, =0xFFFF9004
	ldr r0, [r0]
	ldr r0, [r0, #0xc8]
	ldr r0, [r0, #0x94]
	str r0, [r1]
	bx lr

FUNC(dbgGetMainThreadTLS_New3ds)
	push {r0, lr}
	adr r0, _dbgGetMainThreadTLS_New3ds
	svc 0x7b
	pop {r0, pc}

_dbgGetMainThreadTLS_Old3ds:
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	ldr r0, =0xFFFF9004
	ldr r0, [r0]
	ldr r0, [r0, #0xb8]
	ldr r0, [r0, #0x94]
	str r0, [r1]
	bx lr

FUNC(dbgGetMainThreadTLS_Old3ds)
	push {r0, lr}
	adr r0, _dbgGetMainThreadTLS_Old3ds
	svc 0x7b
	pop {r0, pc}

_dbgGetMainThreadTLS_Old3ds80:
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	ldr r0, =0xFFFF9004
	ldr r0, [r0]
	ldr r0, [r0, #0xc0]
	ldr r0, [r0, #0x94]
	str r0, [r1]
	bx lr

FUNC(dbgGetMainThreadTLS_Old3ds80)
	push {r0, lr}
	adr r0, _dbgGetMainThreadTLS_Old3ds80
	svc 0x7b
	pop {r0, pc}


FUNC(dbgStubHandleException)
	ldr sp, [r1, #0x34]
	sub sp, sp, #0xA0
	mov r2, sp
	add r3, sp, #0x18
_copy_exception_info:
	ldr r4, [r0], #4
	str r4, [r2], #4
	cmp r2, r3
	bcc _copy_exception_info
	add r2, sp, #0x5C
_copy_exception_context:
	ldr r4, [r1], #4
	str r4, [r3], #4
	cmp r3, r2
	bcc _copy_exception_context
	mov r0, sp
	add r1, sp, #0x18
	ldr r2, =dbgHandleException
	bx r2

dbgCheckCP14RegisterAccessible:
	push {lr}
	mrc p14, 0, r0, c0, c1, 0 @ enable debugger
	tst r0, #0x8000
	orreq r0, r0, #0x8000
	mcreq p14, 0, r0, c0, c1, 0
1:
	mov r0, #0x100
	bl dbgDelay
	mrc p14, 0, r0, c0, c1, 0
	tst r0, #0x4000 @ if this bit is set , access other cp14 registers will occur exception
	bne 1b
	pop {pc}

_dbgInitHardwareDebugger:
	cpsid aif
	push {r4, lr}
	mrc p14, 0, r0, c0, c1, 0 @ enable debugger
	orr r0, r0, #0x8000
	mcr p14, 0, r0, c0, c1, 0
	mov r4, #0
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 7
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 7
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 6
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 6
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c2, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c3, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c4, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c2, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c3, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c4, 4
	pop {r4, pc}

FUNC(dbgInitHardwareDebugger)
	push {lr}
	adr r0, _dbgInitHardwareDebugger
	svc 0x7b
	pop {pc}

_dbgSetHardwareWatchpoint:
	cpsid aif
	push {r4-r6, lr}
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4, {r0-r3} @ r0 = id r1 = flagLoadStore r2 = ByteAddressSelect r3 = address
	cmp r0, #2
	bcs _dbgSetHardwareWatchpoint_end
	mov r4, r3 @set r4 = address
	and r1, r1, #3
	mov r1, r1, lsl #3
	and r2, r2, #15
	mov r2, r2, lsl #5
	orr r1, r1, r2
	ldr r2, =0x140005
	orr r5, r1, r2 @set r5 = wvr
	cmp r0, #0
	bne _dbgSetHardwareWatchpoint_another
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r6, c0, c0, 7 @disable watchpoint
	bic r6, r6, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r6, c0, c0, 7
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 6
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c0, 7
	b _dbgSetHardwareWatchpoint_end
_dbgSetHardwareWatchpoint_another:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r6, c0, c1, 7 @disable watchpoint
	bic r6, r6, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r6, c0, c1, 7
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 6
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c1, 7
_dbgSetHardwareWatchpoint_end:
	pop {r4-r6, pc}

FUNC(dbgSetHardwareWatchpoint)
	push {r0-r3, lr}
	adr r0, _dbgSetHardwareWatchpoint
	svc 0x7b
	pop {r0-r3, pc}

_dbgDisableHardwareWatchpoint:
	cpsid aif
	push {r4, lr}
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r0, [sp], #4
	ldr r0, [r0]
	cmp r0, #2
	bcs _dbgDisableHardwareWatchpoint_end
	cmp r0, #0
	bne _dbgDisableHardwareWatchpoint_another
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r4, c0, c0, 7 @disable watchpoint
	bic r4, r4, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 7
	b _dbgDisableHardwareWatchpoint_end
_dbgDisableHardwareWatchpoint_another:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r4, c0, c1, 7
	bic r4, r4, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 7
_dbgDisableHardwareWatchpoint_end:
	pop {r4, pc}

FUNC(dbgDisableHardwareWatchpoint)
	push {r0, lr}
	adr r0, _dbgDisableHardwareWatchpoint
	svc 0x7b
	pop {r0, pc}

_dbgSetContextIDToBRP:
	cpsid aif
	push {r4-r5, lr}
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	ldr r4, [r1]
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r5, c0, c4, 5
	bic r5, r5, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c4, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c4, 4
	ldr r5, =0x003001e7
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c4, 5
	pop {r4-r5, pc}

FUNC(dbgSetContextIDToBRP)
	push {r0, lr}
	adr r0, _dbgSetContextIDToBRP
	svc 0x7b
	pop {r0, pc}

_dbgSetHardwareBreakpoint:
	cpsid aif
	push {r4-r6, lr}
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4, {r0-r2} @r0 = id r1 = Byte address select r2 = address
	bic r4, r2, #3 @set r4 = address
	ldr r3, =0x140005
	and r1, r1, #0xF
	orr r5, r3, r1, lsl #5@set r5 = bcr
	cmp r0, #0
	beq _dbgSetHWBKBP_0
	cmp r0, #1
	beq _dbgSetHWBKBP_1
	cmp r0, #2
	beq _dbgSetHWBKBP_2
	cmp r0, #3
	beq _dbgSetHWBKBP_3
_dbgSetHWBKPT_end:
	pop {r4-r6, pc}
_dbgSetHWBKBP_0:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r6, c0, c0, 5
	bic r6, r6, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r6, c0, c0, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c0, 5
	b _dbgSetHWBKPT_end
_dbgSetHWBKBP_1:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r6, c0, c1, 5
	bic r6, r6, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r6, c0, c1, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c1, 5
	b _dbgSetHWBKPT_end
_dbgSetHWBKBP_2:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r6, c0, c2, 5
	bic r6, r6, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r6, c0, c2, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c2, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c2, 5
	b _dbgSetHWBKPT_end
_dbgSetHWBKBP_3:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r6, c0, c3, 5
	bic r6, r6, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r6, c0, c3, 5
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c3, 4
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r5, c0, c3, 5
	b _dbgSetHWBKPT_end

FUNC(dbgSetHardwareBreakpoint)
	push {r0-r2, lr}
	adr r0, _dbgSetHardwareBreakpoint
	svc 0x7b
	pop {r0-r2, pc}

_dbgDisableHardwareBreakpoint:
	cpsid aif
	push {r4, lr}
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r0, [sp], #4
	ldr r0, [r0]
	cmp r0, #0
	beq _dbgDisableHWBKBP_0
	cmp r0, #1
	beq _dbgDisableHWBKBP_1
	cmp r0, #2
	beq _dbgDisableHWBKBP_2
	cmp r0, #3
	beq _dbgDisableHWBKBP_3
_dbgDisableHWBKBP_end:
	pop {r4, pc}
_dbgDisableHWBKBP_0:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r4, c0, c0, 5
	bic r4, r4, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c0, 5
	b _dbgDisableHWBKBP_end
_dbgDisableHWBKBP_1:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r4, c0, c1, 5
	bic r4, r4, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c1, 5
	b _dbgDisableHWBKBP_end
_dbgDisableHWBKBP_2:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r4, c0, c2, 5
	bic r4, r4, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c2, 5
	b _dbgDisableHWBKBP_end
_dbgDisableHWBKBP_3:
	bl dbgCheckCP14RegisterAccessible
	mrc p14, 0, r4, c0, c3, 5
	bic r4, r4, #1
	bl dbgCheckCP14RegisterAccessible
	mcr p14, 0, r4, c0, c3, 5
	b _dbgDisableHWBKBP_end

FUNC(dbgDisableHardwareBreakpoint)
	push {r0, lr}
	adr r0, _dbgDisableHardwareBreakpoint
	svc 0x7b
	pop {r0, pc}

_dbgGetCurrentContextID:
	stmdb sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	mrc p15, 0, r0, c13, c0, 1
	str r0, [r1]
	bx lr

FUNC(dbgGetCurrentContextID)
	push {r0, lr}
	adr r0, _dbgGetCurrentContextID
	svc 0x7b
	pop {r0, pc}

_dbgGetDebugStatusAndControlRegister:
	stmdb sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	mrc p14, 0, r0, c0, c1, 0
	str r0, [r1]
	bx lr

FUNC(dbgGetDebugStatusAndControlRegister)
	push {r0, lr}
	adr r0, _dbgGetDebugStatusAndControlRegister
	svc 0x7b
	pop {r0, pc}

FUNC(dbgReturnFromExceptionDirectly)
	ldr sp, [r0,#0x34] @sp
	ldr r1, [r0, #0x3c] @pc
	str r1, [sp, #-4]!
	ldr r1, [r0, #0x38] @lr
	str r1, [sp, #-4]!
	mov r2, #0x30
_store_reg_loop:
	ldr r1, [r0, r2]
	str r1, [sp, #-4]!
	sub r2, r2, #4
	cmp r2, #0
	bge _store_reg_loop
	ldr r1, [r0, #0x40]
	msr cpsr, r1
	ldmfd sp!, {r0-r12, lr, pc}

_dbgGetWatchpointFaultAddressRegister:
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	mrc p15, 0, r0, c6, c0, 1
	str r0, [r1]
	bx lr

FUNC(dbgGetWatchpointFaultAddressRegister)
	push {r0, lr}
	adr r0, _dbgGetWatchpointFaultAddressRegister
	svc 0x7b
	pop {r0, pc}

FUNC(dbgGetAllFloatRegister)
	vstmia r0,{s0-s31}
	bx lr

FUNC(dbgGetCurrentTLS)
	mrc p15, 0, r0, c13, c0, 3
	bx lr

_dbgFlushDataCache:
	ldr r3, [sp] @r3 is new stack pointer
	push {r4, lr}
	str sp, [r3, #-0x4]!
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4!, {r0-r2}
	mov sp, r3
	bl kFlushDataCache
	ldr r3, [sp], #4
	mov sp, r3
	pop {r4, pc}

FUNC(dbgFlushDataCache)
	push {r0-r2, lr}
	adr r0, _dbgFlushDataCache
	svc 0x7b
	pop {r0-r2, pc}

_dbgFlushInstructionCache:
	ldr r3, [sp] @r3 is new stack pointer
	push {r4, lr}
	str sp, [r3, #-0x4]!
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4!, {r0-r2}
	mov sp, r3
	bl kFlushInstructionCache
	ldr r3, [sp], #4
	mov sp, r3
	pop {r4, pc}

FUNC(dbgFlushInstructionCache)
	push {r0-r2, lr}
	adr r0, _dbgFlushInstructionCache
	svc 0x7b
	pop {r0-r2, pc}

_dbgStoreDataCache:
	ldr r3, [sp] @r3 is new stack pointer
	push {r4, lr}
	str sp, [r3, #-0x4]!
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4!, {r0-r2}
	mov sp, r3
	bl kStoreDataCache
	ldr r3, [sp], #4
	mov sp, r3
	pop {r4, pc}

FUNC(dbgStoreDataCache)
	push {r0-r2, lr}
	adr r0, _dbgStoreDataCache
	svc 0x7b
	pop {r0-r2, pc}

_dbgInvalidateDataCache:
	ldr r3, [sp] @r3 is new stack pointer
	push {r4, lr}
	str sp, [r3, #-0x4]!
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4!, {r0-r2}
	mov sp, r3
	bl kInvalidateDataCache
	ldr r3, [sp], #4
	mov sp, r3
	pop {r4, pc}

FUNC(dbgInvalidateDataCache)
	push {r0-r2, lr}
	adr r0, _dbgInvalidateDataCache
	svc 0x7b
	pop {r0-r2, pc}

FUNC(dbgDataSynchronizationBarrier)
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 @Data Synchronization Barrier
	bx lr

_dbgGetCpuNumber:
	push {lr}
	bl ipiGetCpuNumber
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r1, [sp], #4
	str r0, [r1]
	pop {pc}

FUNC(dbgGetCpuNumber)
	push {r0, lr}
	adr r0, _dbgGetCpuNumber
	svc 0x7b
	pop {r0, pc}

_dbgTriggerInterrupt:
	push {lr}
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r3, [sp], #4
	ldmia r3!, {r0-r2}
	bl ipiTriggerInterrupt
	pop {pc}

FUNC(dbgTriggerInterrupt)
	push {r0-r2, lr}
	adr r0, _dbgTriggerInterrupt
	svc 0x7b
	add sp, sp, #0xc
	pop {pc}

#if 0

_dbgMemoryControl:
	ldr r3, [sp] @r3 is new stack pointer
	push {r4-r5, lr}
	str sp, [r3, #-0x4]!
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	mov sp, r3
	sub sp, sp, #0x10
	ldmia r4, {r0-r3}
	ldr r5, [r4, #0x14]
	str r5, [sp, #0]
	ldr r5, [r4, #0x18]
	str r5, [sp, #4]
	ldr r5, [r4, #0x1C]
	str r5, [sp, #8]
	ldr r5, [r4, #0x20]
	str r5, [sp, #0xc]
	bl kMemoryControl
	str r0, [r4]
	add sp, sp, #0x10
	ldr r3, [sp], #4
	mov sp, r3
	pop {r4-r5, pc}

FUNC(dbgMemoryControl)
	push {r0-r3, lr}
	adr r0, _dbgMemoryControl
	svc 0x7b
	pop {r0}
	add sp, sp, #0xc
	pop {pc}

#endif

FUNC(dbgDelay)
	subs r0, r0, #1
	nop
	bne dbgDelay
	bx lr

FUNC(dbgGetCurrentProcessorNumber)
	mrc p15, 0, r0, c0, c0, 5
	and r0, r0, #0x3
	bx lr

_dbgBootDEXKernel:
	ldr r3, [sp] @r3 is new stack pointer
	push {r4, lr}
	str sp, [r3, #-0x4]!
	stmfd sp, {sp}^
	sub sp, sp, #4
	ldr r4, [sp], #4
	ldmia r4, {r0}
	mov sp, r3
	bl kBootDEXKernel
	str r0, [r4]
	ldr r3, [sp], #4
	mov sp, r3
	pop {r4, pc}

FUNC(dbgBootDEXKernel)
	push {r0, lr}
	adr r0, _dbgBootDEXKernel
	svc 0x7b
	pop {r0, pc}