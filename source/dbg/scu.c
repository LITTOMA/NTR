#include "global.h"

u32 ipiGetCpuNumber()
{
	return (REG(IoBaseScu + 4) & 3) + 1;
}

void ipiTriggerInterrupt(u32 filter, u32 cpuList , u32 id)
{
	WRITEREG((IoBaseScu + 0x1000 + 0xF00), ((filter & 0x3) << 24 | ((cpuList & 0xF) << 16) | (id & 0x3FF)));
}

u32 ipiGetInterruptPriority(u32 id)
{
	return REG8(IoBaseScu + 0x1000 + 0x400 + id) >> 4;
}

void ipiSetInterruptPriority(u32 id, u32 priority)
{
	WRITEREG8(IoBaseScu + 0x1000 + 0x400 + id, (priority & 0xF) << 4);
}
