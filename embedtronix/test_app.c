/** 
* This is the test application for the kernel driver
* cmd line to compile:
*       gcc -o test_app test_app.c
*
* In order to be able to intercept an ICMP request you shall run the following command:
*       ping localhost
*    or 
*       ping www.google.com
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

int main()
{
        int fd;
        int32_t value;

        printf("*********************************\n");
	printf("\nOpening Driver\n");
        fd = open("/dev/etx_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
 
        ioctl(fd, RD_VALUE, (int32_t*) &value);
        printf("CMP Counter value retrieved from ioctl: %d\n", value);
 
        /*wait for an input key before closing the device*/
        /*to test that only one client is active at a time*/
	scanf("%d", &value);

        printf("Closing Driver\n");
        close(fd);
}
