export KDIR ?= /home/eduard/Kernel_ARM
export CROSS_COMPILE ?= arm-opi-linux-gnueabihf-
export ARCH := arm

obj-m := bmp180.o

.PHONY: all clean

all:
	$(MAKE) -C $(KDIR)  M=$$PWD modules

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

	
