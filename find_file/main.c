#include <stdio.h>
#include "find_file.h"

int main(void)
{
	char source_path[] = {"test_root"};
	struct file_list root = {source_path, NULL, NULL};
	get_file_path(&root, "kernel_log");
	print_file_list(&root,NULL);
	destory_file_list(&root);
	return 0;
}
