
#define FUNC(name) \
	.global name; \
	.type name, %function; \
name:

.section .text
.arm
	b entry
.global dbgkSetexceptionHandler;
.global dbgkProcessContextIDOffset;
dbgkSetexceptionHandler:
	mrc p14, 0, r1, c0, c1, 0
	tst r1, #0x8000
	beq _dbgkSetexceptionHandler_end
1:
	mrc p14, 0, r1, c0, c1, 0
	tst r1, #0x4000
	bne 1b
	ldr r4, =0xFFFF9004
	ldr r4, [r4]
	ldr r5, dbgkProcessContextIDOffset
	ldrb r4, [r4, r5]
	ldr r5, =0xFFFF0FF8
	ldrb r1, [r5, #0x0]
	cmp r4, r1
	bne _dbgkSetexceptionHandler_end
	mrc p14, 0, r1, c0, c4, 4
	and r1, r1, #0xff
	cmp r4, r1
	bne _dbgkSetexceptionHandler_end
	ldr r2, [r5, #0x4]
	str r2, [r6, #0x48]
_dbgkSetexceptionHandler_end:
	ldmfd sp!, {r4-r11, pc}
dbgkProcessContextIDOffset:
	.word 0

FUNC(dbgkGetCurrentProcessorNumber)
	mrc p15, 0, r0, c0, c0, 5
	and r0, r0, #0x3
	bx lr

FUNC(dbgkCheckCP14Accessable)
	mrc p14, 0, r0, c0, c1, 0 @ enable debugger
	tst r0, #0x8000
	orreq r0, r0, #0x8000
	mcreq p14, 0, r0, c0, c1, 0
_wait_halt:
	mrc p14, 0, r0, c0, c1, 0
	tst r0, #0x4000 @ if this bit is set , access other cp14 registers will occur exception
	bne _wait_halt
	bx lr

FUNC(dbgkInitHardwareDebugger)
	push {r4, lr}
	mrc p14, 0, r0, c0, c1, 0 @ enable debugger
	orr r0, r0, #0x8000
	mcr p14, 0, r0, c0, c1, 0
	mov r4, #0
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c0, 7
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c1, 7
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c0, 6
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c1, 6
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c0, 5
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c1, 5
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c2, 5
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c3, 5
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c4, 5
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c0, 4
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c1, 4
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c2, 4
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c3, 4
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c4, 4
	pop {r4, pc}

FUNC(dbgkSetHardwareWatchpoint)
	cmp r0, #2
	bcs _dbgSetHardwareWatchpoint_end
	push {r4-r6, lr} @ r0 = id r1 = flagLoadStore r2 = ByteAddressSelect r3 = address
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
	bl dbgkCheckCP14Accessable
	mrc p14, 0, r6, c0, c0, 7 @disable watchpoint
	bic r6, r6, #1
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r6, c0, c0, 7
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c0, 6
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r5, c0, c0, 7
	b _dbgSetHardwareWatchpoint_end
_dbgSetHardwareWatchpoint_another:
	bl dbgkCheckCP14Accessable
	mrc p14, 0, r6, c0, c1, 7 @disable watchpoint
	bic r6, r6, #1
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r6, c0, c1, 7
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c1, 6
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r5, c0, c1, 7
_dbgSetHardwareWatchpoint_end:
	pop {r4-r6, pc}

FUNC(dbgkDisableHardwareWatchpoint)
	push {r4-r5, lr}
	mov r4, r0
	cmp r0, #2
	bcs _dbgDisableHardwareWatchpoint_end
	bl dbgkCheckCP14Accessable
	cmp r4, #0
	mrceq p14, 0, r5, c0, c0, 7 @disable watchpoint
	mrcne p14, 0, r5, c0, c1, 7
	bic r5, r5, #1
	bl dbgkCheckCP14Accessable
	cmp r4, #0
	mcreq p14, 0, r5, c0, c0, 7
	mcrne p14, 0, r5, c0, c1, 7
_dbgDisableHardwareWatchpoint_end:
	pop {r4-r5, pc}

FUNC(dbgkSetContextIDToBRP)
	push {r4-r5, lr}
	mov r4, r0
	bl dbgkCheckCP14Accessable
	mrc p14, 0, r5, c0, c4, 5
	bic r5, r5, #1
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r5, c0, c4, 5
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r4, c0, c4, 4
	ldr r5, =0x003001e7
	bl dbgkCheckCP14Accessable
	mcr p14, 0, r5, c0, c4, 5
	pop {r4-r5, pc}

FUNC(dbgkDataSynchronizationBarrier)
	mov r0, #0
	mcr p15, 0, r0, c7, c10, 4 @Data Synchronization Barrier
	bx lr

FUNC(dbgkGetTranslationTableBase)
	mrc p15, 0, r0, c2, c0, 1
	bx lr

FUNC(dbgkHandlePrefetchAbort)
	ldr sp, dbgkExceptionModeStack
	push {r0-r12,lr}
	sub r0, lr, #4
	bl dbgkCheckException
	cmp r0,#0
	pop {r0-r12,lr}
	ldreq pc, dbgkGateway3DSExceptionHandler
	ldrne pc, dbgkOriginalPrefetchAbortHandler

FUNC(dbgkHandleDataAbort)
	ldr sp, dbgkExceptionModeStack
	push {r0-r12,lr}
	sub r0, lr, #8
	bl dbgkCheckException
	cmp r0,#0
	pop {r0-r12,lr}
	ldreq pc, dbgkGateway3DSExceptionHandler
	ldrne pc, dbgkOriginalDataAbortHandler

.global dbgkExceptionModeStack
dbgkExceptionModeStack:
	.word 0

.global dbgkGateway3DSExceptionHandler
dbgkGateway3DSExceptionHandler:
	.word 0

.global dbgkOriginalPrefetchAbortHandler
dbgkOriginalPrefetchAbortHandler:
	.word 0

.global dbgkOriginalDataAbortHandler
dbgkOriginalDataAbortHandler:
	.word 0