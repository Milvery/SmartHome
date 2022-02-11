modname ?=demo
arch ?=x86

ifeq ($(arch),arm)
KERNELDIR:=/home/ubuntu/linux-5.10.10/
else
KERNELDIR:=/lib/modules/$(shell uname -r)/build/
endif

PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

obj-m:=$(modname).o

