REPO_PATH = repo
ifeq ($(wildcard repo/include/BirdGame.h),)
  $(shell git clone --depth=1 git@github.com:NJU-ProjectN/sdlbird.git $(REPO_PATH))
endif

NAME = bird
SRCS = $(shell find $(REPO_PATH)/src/ -name "*.cpp")
INC_PATH = $(REPO_PATH)/include
LIBS += libminiSDL libSDL_image libfixedptc
include $(NAVY_HOME)/Makefile

install: install-file
init:;

#mkdir -p $(NAVY_HOME)/fsimg/share/games/这行代码是我自己添加的  不然会报链接错误
install-file:
	mkdir -p $(NAVY_HOME)/fsimg/share/games/
	ln -sf -T $(abspath $(REPO_PATH)/res) $(NAVY_HOME)/fsimg/share/games/bird

.PHONY: install-file init
 