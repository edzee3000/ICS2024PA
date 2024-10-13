# Makefile for AbstractMachine Kernels and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown)  
# 因为我的ubuntu上面的python环境好像有点问题，没有办法使用markdown_py因此我考虑使用了markdown命令 cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown > Makefile.html
.PHONY: html

## 1. Basic Setup and Checks  基本设置和检查

### Default to create a bare-metal kernel image  默认目标：如果没有指定目标，make 将使用 image 作为默认目标。
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

### Override checks when `make clean/clean-all/html`  当执行 make clean/clean-all/html 时覆盖检查  这句话通常出现在条件判断语句之前，意味着在执行 make clean、make clean-all 或 make html 这些目标时，将忽略后续的一些检查步骤。
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### Print build info message 打印构建信息：在构建开始时打印构建信息。
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### Check: environment variable `$AM_HOME` looks sane 检查环境变量 $AM_HOME 是否正确设置，确保它指向了 AbstractMachine 仓库。
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

### Check: environment variable `$ARCH` must be in the supported list 检查 $ARCH 是否在支持的架构列表中  为下一步从$ARCH变量中提取ISA和PLATFORM做准备
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)")
endif

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
### 从$ARCH变量中提取ISA和PLATFORM。例如，如果ARCH=x86_64-qemu，则ISA=x86_64，PLATFORM=qemu。   使用ARCH_SPLIT参数与subst去检查判断-与空格
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

### Check if there is something to build      检查SRCS  比如am-kernels/tests/am-tests/Makefile当中的$(shell find src/ -name "*.[cS]")寻找所有的.c文件
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif

### Checks end here
endif

## 2. General Compilation Targets  通用编译目标

### Create the destination directory (`build/$ARCH`)   创建目标文件夹build/$ARCH
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

### Compilation targets (a binary image or archive)   定义编译目标，如二进制镜像或归档文件    最终的可执行文件（即镜像image）的路径被定义在变量 IMAGE 中
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

### Collect the files to be linked: object files (`.o`) and libraries (`.a`)   选择链接文件  这里SRC就起作用了
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib) # lazy evaluation ("=") causes infinite recursions
LINKAGE   = $(OBJS) \
  $(addsuffix -$(ARCH).a, $(join \
    $(addsuffix /build/, $(addprefix $(AM_HOME)/, $(LIBS))), \
    $(LIBS) ))

## 3. General Compilation Flags 通用编译标志

### (Cross) compilers, e.g., mips-linux-gnu-g++  设置交叉编译器，例如mips-linux-gnu-g++。
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf
 
### Compilation flags   定义编译标志，如优化级别、警告级别、宏定义等。        比如说CFLAGS就是在这里定义的(?)
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
INCFLAGS += $(addprefix -I, $(INC_PATH))

ARCH_H := arch/$(ARCH).h
CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -MMD $(INCFLAGS)
LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))

## 4. Arch-Specific Configurations 架构特定配置

### Paste in arch-specific configurations (e.g., from `scripts/x86_64-qemu.mk`)  包含架构特定的配置文件，例如scripts/x86_64-qemu.mk。   又比如说scripts/riscv32-nemu.mk文件
-include $(AM_HOME)/scripts/$(ARCH).mk
### 这里markdown文件的逻辑应该是(<-表示的包含进)   abstract-machine/am/Makefile   <-   abstract-machine/Makefile  <-  abstract-machine/scripts/riscv32-nemu.mk   <-  abstract-machine/scripts/isa/riscv.mk  &&  abstract-machine/scripts/platform/nemu.mk

## 5. Compilation Rules  编译规则

### Rule (compile): a single `.c` -> `.o` (gcc)      定义了如何从.c、.cc、.cpp和.S文件编译成.o对象文件。
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

### Rule (recursive make): build a dependent library (am, klib, ...)
$(LIBS): %:
	@$(MAKE) -s -C $(AM_HOME)/$* archive

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)   定义了如何链接对象文件和库文件生成最终的ELF二进制文件。
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo \# Creating image [$(ARCH)]
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif

### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar)
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^

### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`  它处理编译过程中生成的 .d 文件，这些文件包含了编译器生成的头文件依赖信息
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous 杂项

### Build order control    控制构建顺序的规则。
image: image-dep
archive: $(ARCHIVE)
image-dep: $(LIBS) $(IMAGE).elf
.NOTPARALLEL: image-dep
.PHONY: image image-dep archive run $(LIBS)

### Clean a single project (remove `build/`)  清理一个单独项目build
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors) 清理构建产物的规则，包括 clean 和 clean-all。
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)


### 当ARCH变量设置为native时，链接器会使用C++编译器而不是LD链接器来链接对象文件，因为native环境可能需要C++运行时支持。   
### CFLAGS和LDFLAGS中添加了特定的标志，以支持native环境的编译和链接。
### insert-arg目标用于在生成的二进制文件中插入参数。
### run目标用于运行生成的二进制文件，它调用了NEMU模拟器，并传递了相应的参数。
### gdb目标用于在GDB调试器中启动模拟器。