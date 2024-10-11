include $(AM_HOME)/scripts/isa/riscv.mk

AM_SRCS := riscv/spike/trm.c \
           riscv/spike/ioe.c \
           riscv/spike/timer.c \
           riscv/spike/start.S \
           riscv/spike/htif.S \
           platform/dummy/cte.c \
           platform/dummy/vme.c \
           platform/dummy/mpe.c \

CFLAGS    += -fdata-sections -ffunction-sections
LDSCRIPTS += $(AM_HOME)/am/src/riscv/spike/linker.ld
LDFLAGS   += --gc-sections -e _start

CFLAGS += -DMAINARGS=\"$(mainargs)\"  #如果在spike中使用（开启difftest选项）的话这个就是mainargs的值
.PHONY: $(AM_HOME)/am/src/riscv/spike/trm.c

image: image-dep
