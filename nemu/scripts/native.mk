#***************************************************************************************
# Copyright (c) 2014-2022 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

-include $(NEMU_HOME)/../Makefile
#这行代码又把同一个build目录下的build.mk的markdown文件包含进来
include $(NEMU_HOME)/scripts/build.mk
#那在native.mk里面又包含进来了difftest.mk文件，这个就是在PA2中进行difftest的部分的makefile文件
include $(NEMU_HOME)/tools/difftest.mk

#如果把$(call git_commit, "compile NEMU")这条语句注释掉的话那么每次make编译的时候就不会自动帮我commit了  $(call git_commit, "compile NEMU")
compile_git:
	
$(BINARY):: compile_git

# Some convenient rules

override ARGS ?= --log=$(BUILD_DIR)/nemu-log.txt
override ARGS += $(ARGS_DIFF)

# Command to execute NEMU
IMG ?=
NEMU_EXEC := $(BINARY) $(ARGS) $(IMG)

run-env: $(BINARY) $(DIFF_REF_SO)

#如果把	$(call git_commit, "run NEMU")  这一行注释掉的话每一次make run的时候就不会帮我trace跟踪记录了   $(call git_commit, "run NEMU")  
#但是这也挺危险的  毕竟需要自己手动commit……万一哪里出了问题emmm……
run: run-env
	
	$(NEMU_EXEC)

gdb: run-env
	$(call git_commit, "gdb NEMU")
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean-tools = $(dir $(shell find ./tools -maxdepth 2 -mindepth 2 -name "Makefile"))
$(clean-tools):
	-@$(MAKE) -s -C $@ clean
clean-tools: $(clean-tools)
clean-all: clean distclean clean-tools

.PHONY: run gdb run-env clean-tools clean-all $(clean-tools)
