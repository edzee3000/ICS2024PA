#这里是riscv32和riscv64架构一些共同的参数  比如用什么去交叉编译（都用riscv64-linux-gnu-）
CROSS_COMPILE := riscv64-linux-gnu-
COMMON_CFLAGS := -fno-pic -march=rv64g -mcmodel=medany -mstrict-align
CFLAGS        += $(COMMON_CFLAGS) -static
ASFLAGS       += $(COMMON_CFLAGS) -O0
LDFLAGS       += -melf64lriscv

# overwrite ARCH_H defined in $(AM_HOME)/Makefile  重写在abstract-machine/Makefile中定义的架构
ARCH_H := arch/riscv.h
