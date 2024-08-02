KERNELDIR := /lib/modules/5.15.0-118-generic/build
CURRENT_PATH := $(shell pwd)

obj-m := mydev.o

build:kernel_modules 

kernel_modules: 
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean: 
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean