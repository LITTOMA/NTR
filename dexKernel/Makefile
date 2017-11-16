ifeq ($(strip $(DEVKITARM)),)
$(error "请将DEVKITARM添加到环境变量")
endif

PREFIX = $(DEVKITARM)\\bin\\arm-none-eabi-

CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
bin2c = C:\\devkitPro\\devkitPSP\\bin\\bin2c

# define options for compilation
CFLAGS = -Wall -Os -fno-builtin -march=armv6k -mthumb -fshort-wchar
CFLAGS += -Ilibctru


# define options for linkage. Prevent the inclusion of standard start
# code and libraries.
LDFLAGS = -fno-builtin -nostartfiles -T ld.S -Wl,--use-blx
#LDFLAGS += -fno-builtin -nostartfiles -T ld.S -Wl,--use-blx -Wl,-pie -nodefaultlibs


# define options for the objdump
DUMPFLAGS = -xdsS

# use variables to refer to init code in case it changes
OBJS = head.o main.o platform.o ipi.o

#
# define build targets
#
all: go.bin

clean:
	rm -f *.o *.elf *.bin *.dump


# build s-record with init code and c files linked together
go.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o go.elf $(OBJS) $(LIBS)
	$(OBJDUMP) $(DUMPFLAGS) go.elf > go.dump
	$(OBJCOPY) -O binary go.elf arm11.bin -S
	$(bin2c) arm11.bin dexKernel.h dexKernel
	cp dexKernel.h ../include/dbg
	rm dexKernel.h

# handle compilation of C files
%.o:%.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -c $< -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@


