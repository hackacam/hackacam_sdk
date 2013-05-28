GNU_TOOLS   ?= /usr/bin
CXX         := $(GNU_TOOLS)/g++
CC          := $(GNU_TOOLS)/gcc
LD          := $(GNU_TOOLS)/ld
AR          := $(GNU_TOOLS)/ar

LINUX_VERSION       := $(shell uname -r)
KERNEL_BUILD_DIR     = /lib/modules/$(LINUX_VERSION)/build/
