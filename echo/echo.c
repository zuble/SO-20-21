//#include <linux/kdev_t.h>
#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include<linux/cdev.h>  					// this is for character device, makes cdev avilable*/

#define DRIVER_AUTHOR "Rui Barbosa e Ana Silva"
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("Dual BSD/GPL");

/******************************** VARS ***********************************/
#define ECHO_DEVS 1
#define DEVICE_NAME "echo"

dev_t dev;
int echo_major = 0;
static char message[256] = {0};  ///< Memory for the string that is passed from userspace
static short size_of_message;   ///< Used to remember the size of the string stored
static int numberOpens = 0;  ///< Counts the number of times the device is opened

struct cdev *ccdev;

//FUNCTIONS
static int echo_init(void);
void echo_cleanup(void);


// The prototype functions for the character driver -- must come before the struct definition
static int     echo_open(struct inode *, struct file *);
static int     echo_release(struct inode *, struct file *);
static ssize_t echo_read(struct file *, char *, size_t, loff_t *);
static ssize_t echo_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops ={
   .open = echo_open,
   .read = echo_read,
   .write = echo_write,
   .release = echo_release,
};
/**************************************************************************/



/****************************** OPEN and RELEASE **************************/
/*
 *  The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
int echo_open (struct inode *inode, struct file *filp){

	nonseekable_open( inode , filp);

	filp->private_data = &dev;

	numberOpens++;
	printk(KERN_INFO "\n f:echo_open , device has been opened %d time(s\n", numberOpens);
	return 0;
}

/*
 *  The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
int echo_release (struct inode *inode, struct file *filp){

	printk(KERN_INFO "\n f:echo_release\n\n" );
	return 0;
}
/**************************************************************************/



/******************** READ and WRITE ************************************/
/*
 *	This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t echo_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "\n echo: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "\n echo: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

/*
 *  This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t echo_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "\n f:echo_write , received %zu characters from the user\n", len);
   return len;
}
/************************************************************************/



/******************** INIT and CLEANUP **********************************/
/*
 *  The static keyword restricts the visibility of the function to within this C file.
 *  @return returns 0 if successful
 */
static int echo_init(void){

	/*
	* {1,2}_chrdev_region
	* 1:register... - static way , we chose the a free major number
	* 2:alloc... - dynamic way
	*/

	int result = alloc_chrdev_region(&dev, 0, ECHO_DEVS , DEVICE_NAME);

	if ( result < 0 ){
		printk(KERN_INFO "\n Cannot allocate major number for device %s\n" , DEVICE_NAME);
		return result;
	}
	else{
		echo_major = MAJOR(dev);
		printk(KERN_INFO "\n Major = %d Minor = %d \n\n", MAJOR(dev) , MINOR(dev) );
	}

	ccdev = cdev_alloc(); /*create, allocate and initialize our cdev structure*/
  ccdev->ops = &fops;   /*fops stand for our file operations*/
  ccdev->owner = THIS_MODULE;

  /*we have created and initialized our cdev structure now we need to add it to the kernel*/
  result = cdev_add(ccdev,dev,1);
  if( result < 0 ) {
      printk(KERN_ALERT "\n f:echo_init , adding device to the kerknel failed\n");
      return result;
  }

	printk(KERN_INFO "\n f:echo_init\n\n");
	return 0; //sucesso

}

/*
 *  Similar to the initialization function, it is static.
 */
void echo_cleanup(void){
	cdev_del(ccdev); /*removing the structure that we added previously*/
  printk(KERN_INFO "\n f:echo_cleanup , ccdev removed from kernel\n");
	unregister_chrdev_region( dev , ECHO_DEVS);
  printk(KERN_INFO "\n f:echo_cleanup , device number unregistered \n");
	printk(KERN_INFO "\n f:echo_cleanup , sucess\n");
}
/************************************************************************/



module_init(echo_init);
module_exit(echo_cleanup);
