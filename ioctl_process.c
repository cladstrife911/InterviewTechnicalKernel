
/* https://tldp.org/LDP/lkmpg/2.4/html/x856.html
* ioctl.c */

#include "hello-1.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

ioctl_set_msg(int file_desc, char *message)
{
	int ret;
	ret = ioctl(file_desc, IOCTL_SET_MSG, message);
	if(ret<0){
		printf("ioctl_set_msg failed: %d\n", ret);
		exit(-1);
	}
}

ioctl_get_msg(int file_desc)
{
	int ret;
	char message[100];

	ret = ioctl(file_desc, IOCTL_GET_MSG, message);
	if(ret<0){
		printf("ioctl_get_msg failed: %d\n", ret);
		exit(-1);
	}
	printf("get_msg message:%s\n", message);
}

ioctl_get_nth_byte(int file_desc)
{
	int i;
	char ch;
	i=0;
	while(ch!=0){
		ch=ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++);
		if(ch<0){
			printf("ioctl_get_nth_byte failed at %d'th byte\n", i);
			exit(-1);
		}
		putchar(ch);
	}
	putchar('\n');
}

void main(void)
{
	int file_desc;
	int ret;
	char *msg="Message passed by ioctl to kernel\n";

	file_desc = open(DEVICE_FILE_NAME, 0);
	if(file_desc < 0){
		printf("can't open device file: %s\n",  DEVICE_FILE_NAME);
		exit(-1);
	}

	ioctl_get_nth_byte(file_desc);
	ioctl_get_msg(file_desc);
	ioctl_set_msg(file_desc, msg);

	close(file_desc);
}
