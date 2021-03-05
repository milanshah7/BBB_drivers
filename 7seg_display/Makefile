obj-m := 7seg_display.o

KDIR := /lib/modules/$(shell uname -r)/build
SRC := $(shell pwd)
DT_NAME := seg-display
MAKEFLAGS += --quiet

all: clean build modules_install

build:
	echo Build Kernel Modules...
	$(MAKE) -C $(KDIR) M=$(SRC)
	echo Compiling Device Tree Overlay...
	dtc -I dts -O dtb -o ${DT_NAME}.dtbo ${DT_NAME}.dts

modules_install:
	echo Installing Module...
	$(MAKE) -C $(KDIR) M=$(SRC) modules_install

clean:
	echo Cleaning all the stuff...
	rm -f *.o *.a *~ core .depend .*.cmd *.ko *.mod.c *.mod
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
	rm -rf *.dtb *.dtb*
