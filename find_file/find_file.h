#ifndef	_FIND_FILE_H_
#define	_FIND_FILE_H_
#include <linux/errno.h>

#define LEFT 0
#define	RIGHT	1

#define mine_dbg(format,arg...)	printf("%s:"format,__func__,##arg)

struct file_list {
	char *path;
	struct file_list *left_child;	//show the director or file in this folder 
	struct file_list *right_child;	//show the director in the same folder
};

extern int get_file_path(struct file_list* root, char *target_file_name);

#endif
