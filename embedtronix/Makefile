obj-m += driver.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR)  M=$(shell pwd) modules
	rm -r -f *.mod.c .*.cmd *.symvers *.o

clean:
	make -C $(KDIR)  M=$(shell pwd) clean

