KERNEL_BUILD:= /home/ubuntu/linux-5.10.10/
MODULE_PATH:=$(shell pwd)

module:
	$(MAKE) -C $(KERNEL_BUILD) M=$(MODULE_PATH) modules

clean:
	$(MAKE) -C $(KERNEL_BUILD) M=$(MODULE_PATH) clean

obj-m:= m74hc595.o

