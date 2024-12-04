include $(NAVY_HOME)/scripts/riscv/common.mk
CFLAGS  += -march=rv32g -mabi=ilp32  #overwrite
LDFLAGS += -melf32lriscv


# 我们约定目前用户程序需要被链接到内存位置0x3000000(x86)或0x83000000(mips32或riscv32)附近, 
# Navy已经设置好了相应的选项(见navy-apps/scripts/$ISA.mk中的LDFLAGS变量). 