/*
 *       Filename:  chdx.c
 *    Description:  A character driver that displays "Hello World!".
 *         Author:  Jaime Arrocha 
 */

#include <linux/module.h>		/* for modules			*/
#include <linux/init.h>			/* module_init, module_exit	*/
#include <linux/moduleparam.h>		/* module parameters		*/
#include <linux/fs.h>			/* file operations		*/
#include <linux/slab.h>			/* kernel memory allocation	*/
#include <linux/cdev.h>			/* device registration		*/
#include <linux/device.h>		/* udev class creation		*/
#include <asm/uaccess.h>		/* copy_to/from_user		*/
#define MOD_NAME "chdx"

/*
 * global variables
 */
char *kbuf;				/* kernel buffer for module	*/
static dev_t first;			/* device registration numbers	*/
static struct device *chdx_dev;
static struct cdev *chdx_cdev;		/* char device structure	*/
static struct class *chdx_cl;		/* device class			*/
int qty = 0;				/* paramenter variable		*/
static unsigned int count = 1;
static size_t kbuf_size = 1024;

/*
 * function prototypes 
 */
static int chdx_open(struct inode *, struct file *);
static int chdx_close(struct inode *, struct file *);
static ssize_t chdx_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t chdx_write(struct file *, const char __user *, size_t, loff_t *);
//static ssize_t chdx_release(struct inode *, struct file *);

/* 
 * module paramenter prototype
 */
module_param(qty, int, S_IRUGO|S_IWUSR);

/*
 * file operations structure initialization
 */
struct file_operations chdx_fops = {
	.owner = THIS_MODULE,
	.open = chdx_open,
	.release = chdx_close,
	.read = chdx_read,
	.write = chdx_write,
	//.unlocked_ioctl = chdx_ioctl
};

static int __init chdx_init(void)
{
	int ret_val = -1;
	int x;

	/* dynamic allocation of major number */
	if((ret_val = alloc_chrdev_region(&first, 0, count, MOD_NAME)) < 0) {
		dev_err(chdx_dev,"Registration error");
		return ret_val;
	}

	/* device registration */
	if(!(chdx_cdev = cdev_alloc())) {
		dev_err(chdx_dev, "cdev_alloc() failed");
		goto unreg1;
	}

	cdev_init(chdx_cdev, &chdx_fops); 

	/* dynamic allocation of kernel buffer */	
	if(!(kbuf = kmalloc(kbuf_size, GFP_KERNEL)))
		goto unreg2;

	if((cdev_add(chdx_cdev, first, count)) < 0) {
		dev_err(chdx_dev, "cdev_add() failed");
		goto unreg3;
	}
	
		
	if(!(chdx_cl = class_create(THIS_MODULE, MOD_NAME)))
		goto unreg3;
	if((chdx_dev = device_create(chdx_cl, NULL, first, NULL, "%s", 
				     MOD_NAME)) == NULL)
		goto unreg4;

	/* module registration test */	
	if (qty == 0)
		qty = 10;	
	for(x = 0; x < qty; x++)
		dev_info(chdx_dev, "Hello World!");
	dev_info(chdx_dev, "Major: %d, Minor: %d", MAJOR(first), MINOR(first));

	return 0;
	
	unreg4: device_destroy(chdx_cl, first);
	unreg3: kfree(kbuf);
	unreg2: cdev_del(chdx_cdev);
	unreg1: unregister_chrdev_region(first, count);
	return ret_val;
}

static void __exit chdx_exit(void)
{
	device_destroy(chdx_cl, first);
	class_destroy(chdx_cl);
	kfree(kbuf);
	cdev_del(chdx_cdev);
	unregister_chrdev_region(first, count);
	dev_info(chdx_dev, "chdx unloaded");
}


static int chdx_open(struct inode *ind, struct file *fl)
{
	dev_info(chdx_dev, "open()");
	return 0;
}


static int chdx_close(struct inode *ind, struct file *fl)
{
	dev_info(chdx_dev, "close()");
	return 0;
}


static ssize_t chdx_read(struct file *fl, char __user *ubuf, size_t ubuf_len, 
			 loff_t *off)
{
	int rbytes, maxbytes, bytes_to_rd; 
	
	maxbytes = kbuf_size - *off;
	bytes_to_rd = maxbytes > ubuf_len? ubuf_len : maxbytes;
	if (bytes_to_rd == 0)
		dev_info(chdx_dev,"End of buffer");
	rbytes = bytes_to_rd - copy_to_user(ubuf, kbuf + *off, bytes_to_rd);
	*off += rbytes;
	dev_info(chdx_dev, "read() %d bytes\n", rbytes);

	return rbytes;
}


static ssize_t chdx_write(struct file *fl, const char __user *ubuf,
			  size_t ubuf_len, loff_t *off)
{
	int wbytes, maxbytes, bytes_to_do;                                      
        
	maxbytes = kbuf_size - *off;                                        
        bytes_to_do = maxbytes > ubuf_len ? ubuf_len : maxbytes;
        if (bytes_to_do == 0)                                                   
                dev_info(chdx_dev, "Reached end of the device on a write");
        wbytes = bytes_to_do - copy_from_user(kbuf + *off, ubuf, bytes_to_do);
        *off += wbytes;                                                        
        dev_info(chdx_dev, "Leaving chdk_write(), wbytes=%d, pos=%d\n", wbytes,
		 (int)*off);                                            

        return wbytes; 
}

module_init(chdx_init);
module_exit(chdx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jaime Arrocha");
MODULE_DESCRIPTION("A character driver");
