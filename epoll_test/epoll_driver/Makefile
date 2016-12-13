obj-m := epoll_driver.o

default:
	make -C /lib/modules/`uname -r`/build M=`pwd` modules
	
clean:
	rm -rf *.o *.ko *.mod.c *.symvers *.order *~
