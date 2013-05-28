ARM_TOOLS ?= /tools/codesourcery/Sourcery_G++_Lite_for_ARM_GNU_Linux/2009q3-67/Linux
CXX       := $(ARM_TOOLS)/bin/arm-none-linux-gnueabi-c++
CC        := $(ARM_TOOLS)/bin/arm-none-linux-gnueabi-gcc
LD        := $(ARM_TOOLS)/bin/arm-none-linux-gnueabi-ld
AR        := $(ARM_TOOLS)/bin/arm-none-linux-gnueabi-ar

LINUX_VERSION    := 3.8.3
KERNEL_BUILD_DIR := $(ROOT)/build/obj/kernel
