obj-m += panel-co5300-466x466.o
obj-m += hynitron_cst9217.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
