# Define the root
ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCLUDES=$(ROOT)includes
DOSBIN=$(ROOT)dos/bin/
ENVDIR=i386

ifeq ($(OS),Windows_NT)
	SUFFIX=.exe
else
	SUFFIX=
endif

CC=/opt/cross/bin/i386-elf-gcc
CFLAGS=-m32 -O -ffreestanding -nostdlib -I$(INCLUDES) -Wall -Wextra
LD=/opt/cross/bin/i386-elf-ld
OBJCOPY=/opt/cross/bin/i386-elf-objcopy
RM=rm -rf
FASM=fasm
NASM=nasm
MKDIR=mkdir -p
DOSBOX=dosbox
QEMU=qemu-system-i386