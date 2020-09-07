#ifndef _HELLO_1_H_
#define _HELLO_1_H_

/*https://tldp.org/LDP/lkmpg/2.4/html/x856.html*/

#include <linux/ioctl.h>

#define MAJOR_NUM 100

/* set the message of the device driver*/
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char*)
/*get the message of the device driver*/
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char*)
/*get the n'th byte of the message*/
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, char*)

/*The name of the device file*/
#define DEVICE_FILE_NAME "char_dev"

#endif
