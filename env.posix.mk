# Define the root
ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCLUDES=$(ROOT)includes
DOSBIN=$(ROOT)dos/bin/
ENVDIR=posix

ifeq ($(OS),Windows_NT)
	SUFFIX=.exe
else
	SUFFIX=
endif

CC=gcc
CFLAGS=-I$(INCLUDES)
LD=ld
OBJCOPY=objcopy
RM=rm -rf
FASM=fasm
NASM=nasm
MKDIR=mkdir -p