# 我们需要在GNU/Linux下根据AM的运行时环境编译出能够在$ISA-nemu这个新环境中运行的可执行文件. 
# 为了不让链接器ld使用默认的方式链接, 我们还需要提供描述$ISA-nemu的运行时环境的链接脚本.
# AM的框架代码已经把相应的配置准备好了, 上述编译和链接选项主要位于abstract-machine/Makefile 
# 以及abstract-machine/scripts/目录下的相关.mk文件中

#nemu的地位应当是和qemu、spike等等是一样的
#这里的AM_SRCS是在nemu模拟机中运行的时候要传进来的一些src资源    路径在abstract-machine/am/src/platform/nemu当中
AM_SRCS := platform/nemu/trm.c \
           platform/nemu/ioe/ioe.c \
           platform/nemu/ioe/timer.c \
           platform/nemu/ioe/input.c \
           platform/nemu/ioe/gpu.c \
           platform/nemu/ioe/audio.c \
           platform/nemu/ioe/disk.c \
           platform/nemu/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
CFLAGS    += -I$(AM_HOME)/am/src/platform/nemu/include
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
NEMUFLAGS += -l $(shell dirname $(IMAGE).elf)/nemu-log.txt 
# NEMUFLAGS += -b #注意这里添加了-b作为批处理参数
NEMUFLAGS += -e $(IMAGE).elf
#这里的$(IMGAE)其实是二进制可执行文件的镜像  比如riscv32-nemu-interpreter这个可执行文件

# 请你通过RTFSC理解这个参数是如何从make命令中传递到hello程序中的, $ISA-nemu和native采用了不同的传递方法, 都值得你去了解一下.
MAINARGS_MAX_LEN = 64    #在这里定义了mainargs的最大长度为64
MAINARGS_PLACEHOLDER = The insert-arg rule in Makefile will insert mainargs here.
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=\""$(MAINARGS_PLACEHOLDER)"\"  
#给出一个宏定义MAINARGS, 其值为$(mainargs)
insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) "$(MAINARGS_PLACEHOLDER)" "$(mainargs)"

image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

#这里是在各种test当中make ARCH=$ISA-nemu run 中的命令
run: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin

gdb: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) gdb ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin

.PHONY: insert-arg
