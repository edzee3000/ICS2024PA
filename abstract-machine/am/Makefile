NAME     := am
SRCS      = $(addprefix src/, $(AM_SRCS))
INC_PATH += $(AM_HOME)/am/src

# $(SRCS)：这是一个变量，包含了所有源代码文件的列表。这些文件需要被编译成目标文件
#在am的makefile当中将abstract-machine中的makefile包含进来
include $(AM_HOME)/Makefile
