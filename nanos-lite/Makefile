# MAKEFLAGS+= -j 32  #在这里我修改了-j参数让多个cpu同时去跑makefile  编译速度会更快
HAS_NAVY = 1
# 然后运行make ARCH=$ISA-nemu update就会自动编译Navy中的程序, 
# 并把navy-apps/fsimg/目录下的所有内容整合成ramdisk镜像navy-apps/build/ramdisk.img, 
# 同时生成这个ramdisk镜像的文件记录表navy-apps/build/ramdisk.h, Nanos-lite的Makefile会通过软连接把它们链接到项目中.
# 如果你修改了Navy中的内容, 请记得通过上述命令来更新镜像文件
RAMDISK_FILE = build/ramdisk.img

NAME = nanos-lite
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")
include $(AM_HOME)/Makefile

ifeq ($(ARCH),native)
ISA = am_native
else
INC_PATH += include $(NAVY_HOME)/libs/libc/include
endif

./src/resources.S: $(RAMDISK_FILE)
	@touch $@

ifeq ($(HAS_NAVY),)
files = $(RAMDISK_FILE) src/files.h src/syscall.h
# create an empty file if it does not exist
$(foreach f,$(files),$(if $(wildcard $f),, $(shell touch $f)))
else

ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
  $(error $$NAVY_HOME must be a Navy-apps repo)
endif

update:
	$(MAKE) -s -C $(NAVY_HOME) ISA=$(ISA) ramdisk
	@ln -sf $(NAVY_HOME)/build/ramdisk.img $(RAMDISK_FILE)
	@ln -sf $(NAVY_HOME)/build/ramdisk.h src/files.h
	@ln -sf $(NAVY_HOME)/libs/libos/src/syscall.h src/syscall.h

.PHONY: update
endif
