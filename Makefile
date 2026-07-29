# binhack64 - Dreamcast IP.BIN/BOOT.BIN Selfboot Hacker
# Copyright (C) FamilyGuy and contributors
# Licensed under the GNU General Public License v3 (GPLv3).
# See the LICENSE file for the full license text.

# Toolchain
CC = g++
STRIP = strip
WINDRES = windres
PACKER = upx
RM = rm

# flags
# add the -DDEBUG directive to CPPFLAGS to compile in DEBUG mode.
#CPPFLAGS = -O3 -DDEBUG
CPPFLAGS = -O3 -static
PACKEROPT = -9 --compress-resources=0

# directories
OBJ_DIR = obj/
SRC_DIR = src/
BIN_DIR = bin/

# files
BIN = binhack64
TARGET = $(BIN_DIR)$(BIN)$(OS:Windows_NT=.exe)
LIBS =
OBJS = $(OBJ_DIR)main.o $(OBJ_DIR)cli.o $(OBJ_DIR)commands.o $(OBJ_DIR)binhack.o $(OBJ_DIR)hack.o $(OBJ_DIR)cdda.o $(OS:Windows_NT=$(OBJ_DIR)version.o)

# go ! (same recipe on every platform, no per-OS target needed)
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
	$(CC) $(CPPFLAGS) -c $< -o $@

# include win32 resources
$(OBJ_DIR)%.o: $(SRC_DIR)%.rc | $(OBJ_DIR)
	$(WINDRES) -i $< -o $@

# create output directories on demand (not tracked in git, see .gitignore)
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	$(RM) -f $(OBJS)

clean-all:
	$(RM) -f $(OBJS) $(TARGET)
