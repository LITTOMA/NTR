#define REG8(addr) *(vu8*)(addr)
#define REG16(addr) *(vu16*)(addr)
#define REG(addr) *(vu32*)(addr)

#define WRITEREG8(addr,data) *(vu8*)(addr) = (vu8)(data)
#define WRITEREG16(addr,data) *(vu16*)(addr) = (vu16)(data)
#define WRITEREG(addr,data) *(vu32*)(addr) = (vu32)(data)

#define SOFTWARE_INTERRUPT_FILTER_CPULIST 0
#define SOFTWARE_INTERRUPT_FILTER_OTHERCPU 1
#define SOFTWARE_INTERRUPT_FILTER_OWNCPU 2

#define INTERRTUPT_DEBUGGER_ID 12

u32 ipiGetCpuNumber();
void ipiTriggerInterrupt(u32 filter, u32 cpuList , u32 id);

