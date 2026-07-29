# binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
# Copyright (C) 2010-2026 FamilyGuy and contributors
# Licensed under the GNU General Public License v3 (GPLv3).
# See the LICENSE file for the full license text.

# Toolchain
CC = g++
STRIP = strip
WINDRES = windres
PACKER = upx
RM = rm

# Version. Single source of truth: bump it here and both the CLI banner and
# the Win32 version resource follow. Passed unquoted and stringified in
# version.hpp, so no shell-dependent \" escaping is involved.
VERSION = 2.0.0.0
comma := ,
VERSION_RC := $(subst .,$(comma),$(VERSION))
VERSIONFLAGS = -DBINHACK64_VERSION_RAW=$(VERSION) -DBINHACK64_VERSION_RC=$(VERSION_RC)

# flags
# add the -DDEBUG directive to CPPFLAGS to compile in DEBUG mode.
#CPPFLAGS = -O3 -DDEBUG
CPPFLAGS = -O3 -static
PACKEROPT = -9 --compress-resources=0

# directories
OBJ_DIR = obj/
SRC_DIR = src/
BIN_DIR = bin/

# Files
BIN = binhack64
TARGET = $(BIN_DIR)$(BIN)$(OS:Windows_NT=.exe)
LIBS =
OBJ_NAMES = main.o\
	cli.o\
	commands.o\
	binhack.o\
	hack.o\
	cdda.o\
	bincon.o\
	unprotect.o\
	wince_cdda_fix.o\
	$(OS:Windows_NT=version.o)
OBJS = $(addprefix $(OBJ_DIR),$(OBJ_NAMES))

# Let's go!
all : $(OBJS) | $(BIN_DIR)
	$(CC) -o $(TARGET) $(OBJS) $(CPPFLAGS) $(LIBS)

# Strip and compress the already-built $(TARGET) for distribution. Build
# first, then package it, e.g.:
#   make && make dist
# Packing is best-effort: if upx isn't installed, dist still leaves you
# with a stripped (just uncompressed) binary.
dist :
	$(STRIP) $(TARGET)
	-$(PACKER) $(PACKEROPT) $(TARGET)

# Build, then run the black-box test suite against it.
test : all
	bash tests/run.sh $(TARGET)

# compile source
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(VERSIONFLAGS) -c $< -o $@

# include win32 resources (-I so the .rc can include version.hpp)
$(OBJ_DIR)%.o: $(SRC_DIR)%.rc | $(OBJ_DIR)
	$(WINDRES) -I$(SRC_DIR) $(VERSIONFLAGS) -i $< -o $@

# create output directories on demand
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	$(RM) -f $(OBJS)

clean-all:
	$(RM) -f $(OBJS) $(TARGET)
