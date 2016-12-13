#include <linux/init.h>
#include <linux/types.h>
#include <linux/idr.h>
#include <linux/input/mt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/major.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/wait.h>

#define DEVICE_NAME "Epoll_test"

static dev_t epoll_test_devt;
static struct cdev epoll_cdev;
static struct class *epoll_test_class;
static struct workqueue_struct *queue = NULL;
static struct delayed_work test_work;
wait_queue_head_t epoll_test_wait;

bool have_data = false;

struct link_list {
	unsigned int num;
	struct link_list *next;
};
struct link_list *header = NULL,*tail = NULL;
//add data to tail
static int epoll_list_add(int count)
{
	struct link_list *temp = NULL;
	temp = kzalloc(sizeof(struct link_list), GFP_KERNEL);
	if(NULL == temp){
		printk("[EPOLL]failed to alloc mem\n");
		return -ENOMEM;
	}
	temp->num = count;
	if(NULL == header) {
		header = temp;
	}
	if(NULL != tail) {
		tail->next = temp;
	}
	tail = temp;
	return 0;
}
//return 1 mean empty
//return 0 mean not empty
static int epoll_is_empty(void)
{
	if(header == NULL) {
		return 1;
	} else {
		return 0;
	}
}
//pop the head data
static int epoll_list_del(void)
{
	struct link_list *temp = NULL;
	int ret = 0;
	ret = epoll_is_empty();
	if(ret){
		printk("[EPOLL]link_list is empty\n");
		return -EINVAL;
	}
	temp = header;
	if(NULL == header->next){
		header = tail = NULL;
	} else {
		header = header->next;
	}
	ret = temp->num;
	kfree(temp);
	return ret;
}

static int epoll_open(struct inode *inode, struct file *file)
{
	printk("[EPOLL]open successful\n");
	return 0;
}

static ssize_t epoll_write(struct file *file, const char __user *buffer,
			   size_t count, loff_t *ppos)
{
	int ret = 0;
	return ret;
}

static ssize_t epoll_read(struct file *file, char __user *buffer,
			  size_t count, loff_t *ppos)
{
	int ret = 0;
	int temp_buf[30];
	int i = 0;
	printk("[EPOLL] read data\n");
	if(have_data){
		do{
			ret = epoll_list_del();
			if(ret >= 0){
				temp_buf[i++] = ret;
			}
		} while((i<30)&&(ret >= 0));
	}
	if(copy_to_user(buffer, (void *)temp_buf, i*sizeof(temp_buf[0]))){
		ret = -EFAULT;
	}else{
		ret = i*sizeof(temp_buf[0]);
	}
	have_data = false;
	return ret;
}

static unsigned int epoll_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;;
	poll_wait(file, &epoll_test_wait, wait);
	if(!epoll_is_empty()){
		printk("[EPOLL] link is not empty\n");
		mask |= POLLIN|POLLRDNORM;
		have_data = true;
	}
	return mask;
}

static const struct file_operations epoll_fops = {
	.owner = THIS_MODULE,
	.open = epoll_open,
	.read = epoll_read,
	.write = epoll_write,
	.poll = epoll_poll,
};
/**********************************************
*
*this function is called by workqueue
*
**********************************************/

void work_handler(struct work_struct *data)
{
	static unsigned count = 0;
	int ret = 0;
	int empty_flag = 0;
	printk("[EPOLL] the workqueue is running count=%d\n", count);
	queue_delayed_work(queue,&test_work,msecs_to_jiffies(500));
	empty_flag = epoll_is_empty();
	ret = epoll_list_add(count);
	if(ret < 0){
		return;
	}
	count++;
	
	if(empty_flag) {
		have_data = true;
		wake_up(&epoll_test_wait);
	}
}
/*********************END*********************/
/**********************************************
*
* the init and exit function about workqueue
*
**********************************************/
static int workqueue_init(void)
{
	queue = create_singlethread_workqueue("hello_vivo");
//	printk("workqueue create successful.\n");

	if(!queue)
	{
		return -1;
	}
	INIT_DELAYED_WORK(&test_work,work_handler);
	return 0;
}

static void workqueue_exit(void)
{
	destroy_workqueue(queue);
//	printk("my_net_link: self module exited\n");
}
/*********************END*********************/

static int __init epoll_test_init(void)
{
	int ret = 0;
	ret = alloc_chrdev_region(&epoll_test_devt, 0, 1, DEVICE_NAME);
	if (ret < 0){
		printk("[EPOLL] alloc dev num failed\n");
		return ret;
	}
	epoll_test_class = class_create(THIS_MODULE, "epoll_test_class");
	if(IS_ERR(epoll_test_class)) {
		printk("[EPOLL] class create failed\n");
		return -1;
	}
	device_create(epoll_test_class, NULL, epoll_test_devt, NULL, DEVICE_NAME);
	
	cdev_init(&epoll_cdev, &epoll_fops);
	epoll_cdev.owner = THIS_MODULE;
	ret = cdev_add(&epoll_cdev, epoll_test_devt, 1);
	if (ret < 0){
		printk("[EPOLL] cdev add failed\n");
		return ret;
	}
	ret = workqueue_init();
	if (ret < 0){
		printk("[EPOLL]init timer failed\n");
		return ret;
	}
	init_waitqueue_head(&epoll_test_wait);
	
	queue_delayed_work(queue,&test_work,msecs_to_jiffies(5000));
	return 0;
}

static void __exit epoll_test_exit(void)
{
	workqueue_exit();
	cdev_del(&epoll_cdev);
	device_destroy(epoll_test_class, epoll_test_devt);
	class_destroy(epoll_test_class);
	unregister_chrdev_region(epoll_test_devt, 1);
	return;
}

module_init(epoll_test_init);
module_exit(epoll_test_exit);

MODULE_AUTHOR("Vojtech Pavlik <vojtech@suse.cz>");
MODULE_DESCRIPTION("Input core");
MODULE_LICENSE("GPL");