obj-m += chdx.o

SUBDIRS = /lib/modules/$(shell uname -r)/build

CFLAGS_chdx.o := -DDEBUG

chdx:
	make -C $(SUBDIRS) $(CFLAGS) M=$(PWD) modules;
clean:
	make -C $(SUBDIRS) $(CFLAGS) M=$(PWD) clean;
