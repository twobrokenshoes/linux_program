#include <stdio.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#define	FILE_PATH	"/dev/Epoll_test"
#define MAX_EPOLL_SIZE	1

int main(void)
{
	int fd,epfd;
	int nfds;
	int count = 0;
	int read_buf[30];
	int i;
	struct epoll_event ev;
	fd = open(FILE_PATH, O_RDWR);
	if(fd < 0){
		printf("Open %s failed\n", FILE_PATH);
		return -EINVAL;
	}
	epfd = epoll_create(MAX_EPOLL_SIZE);
	if(epfd < 0){
		printf("create epoll failed\n");
		return -EINVAL;
	}
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
	
	while(1){
		nfds = epoll_wait(epfd, &ev, MAX_EPOLL_SIZE, -1);
		if(nfds){
			if(ev.data.fd == fd){
				count = read(fd, (char *)read_buf, sizeof(read_buf));
				if(count > 0){
					count = count/sizeof(read_buf[0]);
					for(i = 0;i < count; i++ ){
						printf("count is %d\n", read_buf[i]);
					}
				}
			}
		}
	}
	return 0;
}