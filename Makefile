# ==============================================================================
# Makefile for pkg_killer Kernel Module
# ==============================================================================

MODULE_NAME := pkg_killer

# Multi-file module object definition
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := src/main.o src/killer_proc.o src/killer_hook.o

# Default kernel source directory (for host Linux or native build)
KDIR ?= /lib/modules/$(shell uname -r 2>/dev/null || echo "unsupported")/build
PWD  := $(shell pwd)

# Include directory & compiler flags
EXTRA_CFLAGS += -I$(PWD)/include -Wall -Wno-declaration-after-statement

all:
	@echo "Building $(MODULE_NAME) kernel module..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Cross-compilation helper for Android ARM64
# Example usage:
# make android KDIR=/path/to/android/kernel/out CROSS_COMPILE=aarch64-linux-gnu-
android:
	@echo "Cross-compiling for Android ARM64..."
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	@echo "Cleaning up generated build artifacts..."
	$(MAKE) -C $(KDIR) M=$(PWD) clean 2>/dev/null || true
	rm -f src/*.o src/.*.cmd *.o *.ko *.mod *.mod.c *.mod.o *.order *.symvers
	rm -rf .tmp_versions

.PHONY: all android clean
