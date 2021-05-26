obj-m := 7seg_display.o

SRC := $(shell pwd)

ifndef $(KERNEL_SRC)
	KERNEL_SRC = /lib/modules/$(shell uname -r)/build/
endif

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean
