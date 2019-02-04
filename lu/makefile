DRIVE:=$(shell stat -c '%m' makefile)
GITHUB:=$(DRIVE)/github
# For precompiled
COMMON:=$(DRIVE)/common

$(info DRIVE=$(DRIVE))

LD=gcc
CC=gcc
CPATH=
LPATH=
_FLAGS=-Wall -Werror
LFLAGS:=$(_FLAGS)
CFLAGS:=$(_FLAGS)

LUA_DIR:=$(COMMON)/lua-5.3.5
OBJ_DIR=obj
SRC:=$(wildcard *.c)
OBJ:=$(SRC:%=$(OBJ_DIR)/%.o)
OBJ_LUA_DIR:=$(OBJ_DIR)/lua
SRC_LUA_DIR:=$(LUA_DIR)/src
SRC_LUA:=$(wildcard $(SRC_LUA_DIR)/*.c)
SRC_LUA:=$(subst $(SRC_LUA_DIR)/lua.c,,$(SRC_LUA))
SRC_LUA:=$(subst $(SRC_LUA_DIR)/luac.c,,$(SRC_LUA))
OBJ_LUA:=$(patsubst $(SRC_LUA_DIR)/%,$(OBJ_LUA_DIR)/%.o,$(SRC_LUA))
CPATH:=$(CPATH) "$(SRC_LUA_DIR)"
LPATH:=$(LPATH) "$(SRC_LUA_DIR)"
SRCS:=$(SRC) $(SRC_LUA)
OBJS:=$(OBJ) $(OBJ_LUA)
INCS:=$(CPATH:%=-I %)
LIBS:=$(LPATH:%=-L %)
CFLAGS:=-ansi $(CFLAGS) $(INCS)
LFLAGS:=-ansi $(LFLAGS)
OUT_DIR:=$(COMMON)/lu
OUT_EXE:=$(OUT_DIR)/lu.elf

$(OUT_EXE): export LPATH:=$(LPATH)
$(OUT_EXE): export LIBRARY_PATH:=$(LPATH)
$(OUT_EXE): export LD_INCLUDE_PATH:=$(LPATH)
$(OUT_EXE): export LIBRARY_INCLUDE_PATH:=$(LPATH)
$(OUT_EXE): export CPATH:=$(CPATH)
$(OUT_EXE): export C_INCLUDE_PATH:=$(CPATH)
$(OUT_EXE): export CPP_INCLUDE_PATH:=$(CPATH)
$(OUT_EXE): export OBJC_INCLUDE_PATH:=$(CPATH)

$(OUT_EXE): $(OBJS)
	@echo Compiling Lu Executable
	$(LD) $(LFLAGS) -o "$(OUT_EXE)" $(OBJS) -lm
	@echo CPATH=$(CPATH)
	@echo LPATH=$(LPATH)

$(OUT_DIR) $(OBJ_LUA_DIR): $(COMMON) $(OBJ_DIR)
	mkdir $@

$(COMMON) $(OBJ_DIR):
	mkdir $@

$(OBJ_DIR)/%.o: %
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_LUA_DIR)/%.o: $(SRC_LUA_DIR)/%
	$(CC) $(CFLAGS) -o $@ -c $<
