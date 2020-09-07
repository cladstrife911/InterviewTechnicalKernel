/*  
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "hello-1.h"

#define DRIVER_AUTHOR "Antoine Monmarché  <antoine.monmarche@yahoo.fr>"
#define DRIVER_DESC   "Sevenhungs_test_driver"

#define DEVICE_NAME "char_dev"
#define BUF_LEN 80

#define SUCCESS 0

/*used to prevent concurrent access*/
static int Device_Open = 0;
static char  Message[BUF_LEN];
static char *Message_Ptr;

struct file_operations Fops ={
	NULL, /*seek*/
	device_read,
	device_write,
	NULL, /*readdir*/
	NULL, /*select*/
	device_ioctl,
	NULL, /*nmap*/
	device_open,
	NULL, /*flush*/
	device_release
};

/************** Module declarations ********************/
int init_module(void)
{
	int ret_val;
	printk(KERN_INFO "Hello world 1.\n");

	ret_val = module_register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);
	if(ret_val < 0){
		printk("failed with %d\n'=", ret_val);
	}

	printk("To talk to the device you can use:\n");
	printk("mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);

	return 0;
}

void cleanup_module(void)
{
	int ret;
	printk(KERN_INFO "Goodbye world 1.\n");

	ret = module_unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	if(ret<0){
		printk("Error in module_unregister_chrdev: %d\n", ret);
	}
}

/* 
 * Get rid of taint message by declaring code as GPL. 
 */
MODULE_LICENSE("GPL");

/*
 * Or with defines, like this:
 */
MODULE_AUTHOR(DRIVER_AUTHOR);	/* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC);	/* What does this module do */

/************ DEVICE FONCTIONS ************/

/*Fonction called when a process trys to open the device file*/
static int device_open(struct inode *inode, struct file *file)
{
	if(inode!=NULL && file != NULL)
	{
		#ifdef DEBUG
		printk("device_open(%p)\n", file);
		#endif
		if(Device_Open){
			return -EBUSY;
		}
		Device_Open++;
		Message_Ptr = Message;
	}else{
		return -1;
	}
	MOD_INC_USE_COUNT;

	return SUCCESS;
}

/*Function called when the process close the device file*/
static int device_release(struct inode *inode, struct  file *file)
{
	if(inode!=NULL && file!=NULL)
	{
		#ifdef DEBUG
			printk("device_release(%p, %p)\n", inode, file);
		#endif
	}
	Device_Open--;
	MOD_DEC_USE_COUNT;
	return 0;
}

/*Function called when the process tryes to read the device file*/
static int device_read(struct file *file, char *buffer, size_t lenght, loff_t *offset)
{
	int bytes_read = 0;
	if(file!=NULL && buffer!=NULL)
	{
		#ifdef DEBUG
			printk("device_read(%p, %p, %p)\n", file, buffer, lenght);
		#endif
		/*if we are at the end of the string*/
		if(*Message_Ptr==0){
			return 0;
		}

		while(lenght && *Message_Ptr){
			put_user(*(Message_Ptr++), buffer++);
			lenght--;
			bytes_read++; 
		}
		#ifdef DEBUG
			printk("Read %d bytes, %d lenght\n", bytes_read, lenght);
		#endif

	}
	else{
		return -1;
	}

	return bytes_read;
}

/*Function called when the process tryes to write on the device file*/
static ssize_t device_write(struct inode *inode, struct file *file, const char *buffer, int lenght)
{
	int i;
	if((inode!=NULL) && (file!=NULL) && (buffer!=NULL))
	{
		#ifdef DEBUG
			printk("device_write(%p, %s, %d)\n", file, buffer, lenght);
		#endif

		for(i=0; i<lenght && i<BUF_LEN;i++)
		{
			get_user(Message[i], buffer+i);
		}
		Message_Ptr = Message;
		return i;
	}else{
		return 0;
	}
}

/*Function called when the process tries to do in ioctl on the device file*/
int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int i;
	char *temp;
	char ch;
	if(inode!=NULL && file!=NULL)
	{
		switch(ioctl_num){
			case IOCTL_SET_MSG:
				temp = (char*) ioctl_param;
				get_user(ch, temp);
				for(i=0; ch&&i<BUF_LEN;i++, temp++)
				{
					get_user(ch, temp);
				}
				device_write(file, (char*)ioctl_param, i, 0);
			break;

			case IOCTL_GET_MSG:
				i = device_read(file, (char*)ioctl_param, 99,0);
				put_user('\0', (char*)ioctl_param+i);
			break;

			case IOCTL_GET_NTH_BYTE:
				return Message[ioctl_param];
			break;

			default:
				return -1;
			break;
		}
	}
	return SUCCESS;
}
