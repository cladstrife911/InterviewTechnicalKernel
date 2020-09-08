#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
 
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/icmp.h>
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
 
int32_t value = 0;
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 

static struct nf_hook_ops *nfho = NULL;
static int icmp_counter = 0;

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};

/* Netfilter hook callback */
static unsigned int hfunc(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *iph;
	struct icmphdr *icmph;
    //int file_desc;
    //char buffer[10];

	if (!skb)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph->protocol == IPPROTO_ICMP) {
        /*file_desc = open("etx_device", 0);
        if (file_desc < 0) {
            printk("Can't open device file: %s\n", DEVICE_FILE_NAME);
            exit(-1);
        }else{
            //sprintf(buffer, "%d",icmp_counter);
            //ioctl(file_desc, IOCTL_SET_MSG, message);
            // etx_ioctl()
        }*/
    		icmph = icmp_hdr(skb);
    		icmp_counter++;
    		printk(KERN_INFO "ICMP packet intercepted: %d!!\n", icmp_counter);

	}

	return NF_ACCEPT;
}

static void init_netfilter(void)
{
	printk(KERN_INFO "init_netfilter()\n");
	nfho = (struct nf_hook_ops*)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);

	nfho->hook = (nf_hookfn*)hfunc;
	nfho->hooknum = NF_INET_PRE_ROUTING; /*all the received packets*/
	nfho->pf = PF_INET;
	nfho->priority = NF_IP_PRI_FIRST;

	nf_register_net_hook(&init_net, nfho);
}

static void release_netfilter(void)
{
	nf_unregister_net_hook(&init_net, nfho);
}
 
static int etx_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Opened...!!!\n");
        return 0;
}
 
static int etx_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Device File Closed...!!!\n");
        return 0;
}
 
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Read Function\n");
        return 0;
}
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write function\n");
        return 0;
}
 
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        copy_from_user(&value ,(int32_t*) arg, sizeof(value));
                        printk(KERN_INFO "Value = %d\n", value);
                        break;
                case RD_VALUE:
                        copy_to_user((int32_t*) arg, &icmp_counter, sizeof(icmp_counter));
                        break;
        }
        return 0;
}
 
 
static int __init etx_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                printk(KERN_INFO "Cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&etx_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&etx_cdev,dev,1)) < 0){
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
            printk(KERN_INFO "Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
            printk(KERN_INFO "Cannot create the Device 1\n");
            goto r_device;
        }
        printk(KERN_INFO "Device Driver Insert...Done!!!\n");

	init_netfilter();

    return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
void __exit etx_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        release_netfilter();
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Antoine Monmarché <antoine.monmarche@yahoo.fr>");
MODULE_DESCRIPTION("A simple device driver");
MODULE_VERSION("1.0");
