//Kernel related includes
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
 
// IP and ICMP related includes
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/icmp.h>

/******************* LOCAL MACROS ***************************/ 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
 
#define DEVICE_NAME "my_dev"

/******************* LOCAL FUNCTIONS PROTOTYPES ***************************/ 
static int __init my_driver_init(void);
static void __exit my_driver_exit(void);
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t my_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
/******************* LOCAL VARIABLES ***************************/ 
dev_t dev = 0;
static struct class *LOC_my_dev_class;
static struct cdev LOC_my_cdev;
static struct nf_hook_ops *LOC_nfho = NULL;
static int LOC_icmp_counter = 0;
/*true when a user has already opened the device file*/
static bool LOC_bAlreadyOpened = false;

static struct file_operations LOC_fops =
{
        .owner          = THIS_MODULE,
        .read           = my_read,
        .write          = my_write,
        .open           = my_open,
        .unlocked_ioctl = my_ioctl,
        .release        = my_release,
};

/******************* LOCAL FUNCTIONS DEFINITIONS ***************************/ 

/* Netfilter hook callback */
static unsigned int hfunc(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *iph;
	struct icmphdr *icmph;

	if (!skb)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph->protocol == IPPROTO_ICMP) {
    		icmph = icmp_hdr(skb);
            /* Just increment the icmp counter that will be send through ioctl*/
    		LOC_icmp_counter++;
    		printk(KERN_INFO "ICMP packet intercepted: %d!!\n", LOC_icmp_counter);
	}
	return NF_ACCEPT;
}

/**
* \brief this function inits the netfilter to filter all incomming IP request
*/
static void init_netfilter(void)
{
	printk(KERN_INFO "init_netfilter()\n");
	LOC_nfho = (struct nf_hook_ops*)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);

	LOC_nfho->hook = (nf_hookfn*)hfunc;
	LOC_nfho->hooknum = NF_INET_PRE_ROUTING; /*all the received packets*/
	LOC_nfho->pf = PF_INET;
	LOC_nfho->priority = NF_IP_PRI_FIRST;

	nf_register_net_hook(&init_net, LOC_nfho);
}

static void release_netfilter(void)
{
	nf_unregister_net_hook(&init_net, LOC_nfho);
}
 
static int my_open(struct inode *inode, struct file *file)
{
    /*Don't allow another user process to open the file
    * if an user has already opened it */
    if(!LOC_bAlreadyOpened){
        printk(KERN_INFO "Device File Opened...!!!\n");
        LOC_bAlreadyOpened = true;
        return 0;
    }else{
        printk(KERN_INFO "Device File failed to open because another user is already connected!!!\n");
        return -1;
    }
}
 
static int my_release(struct inode *inode, struct file *file)
{
    if(LOC_bAlreadyOpened){
        printk(KERN_INFO "Device File Closed...!!!\n");
        LOC_bAlreadyOpened = false;
        return 0;
    }else{
        return -1;
    }
}
 
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Read Function\n");
        return 0;
}
static ssize_t my_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write function\n");
        return 0;
}
 
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int32_t value = 0;
    switch(cmd) {
        case WR_VALUE:
            /*nothing to be done with this*/
            copy_from_user(&value ,(int32_t*) arg, sizeof(value));
            printk(KERN_INFO "Value = %d\n", value);
            break;
        case RD_VALUE:
            /*Return the ICMP counter value to the user*/
            copy_to_user((int32_t*) arg, &LOC_icmp_counter, sizeof(LOC_icmp_counter));
            break;
    }
    return 0;
}
 
/**
* \brief this function is called when this Kernel driver is created with insmod
*/
static int __init my_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) <0){
            printk(KERN_INFO "Cannot allocate major number\n");
            return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating char dev structure*/
    cdev_init(&LOC_my_cdev,&LOC_fops);

    /*Adding character device to the system*/
    if((cdev_add(&LOC_my_cdev,dev,1)) < 0){
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if((LOC_my_dev_class = class_create(THIS_MODULE,"my_class")) == NULL){
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if((device_create(LOC_my_dev_class,NULL,dev,NULL,"my_device")) == NULL){
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto r_device;
    }
    printk(KERN_INFO "Device Driver Insert...Done!!!\n");

	init_netfilter();

    return 0;
 
r_device:
        class_destroy(LOC_my_dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

/**
* \brief this function is called when this Kernel driver is destroyed with rmmode
*/
void __exit my_driver_exit(void)
{
    device_destroy(LOC_my_dev_class,dev);
    class_destroy(LOC_my_dev_class);
    cdev_del(&LOC_my_cdev);
    unregister_chrdev_region(dev, 1);
    release_netfilter();

    LOC_bAlreadyOpened= false;

    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(my_driver_init);
module_exit(my_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Antoine Monmarché <antoine.monmarche@yahoo.fr>");
MODULE_DESCRIPTION("A simple device driver");
MODULE_VERSION("1.0");
