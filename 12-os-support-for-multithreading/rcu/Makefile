obj-m += my-rcu-module.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

load-module: all
	sudo insmod my-rcu-module.ko

unload-module:
	sudo rmmod my-rcu-module.ko
