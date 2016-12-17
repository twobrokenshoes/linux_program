#include <stdio.h>
#include <linux/fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "find_file.h"

struct file_list* add_child(int left_or_right,struct file_list *parent,char *data)
{
	int length = 0;
	char *pdata;
	struct file_list *child = NULL;
	length = strlen(data);
	pdata = (char *)malloc(length);
	if(NULL == pdata){
		mine_dbg("unable to alloc mem for child'path\n");
		return NULL;
	}
	strcpy(pdata, data);
	child = (struct file_list*)malloc(sizeof(struct file_list));
	if(child == NULL){
		mine_dbg("unable to alloc mem for child\n");
		goto exit;
	}
	child->path = pdata;
	child->right_child = NULL;
	child->left_child = NULL;
	if(left_or_right == RIGHT){
		parent->right_child = child;
	}else if(left_or_right == LEFT){
		parent->left_child = child;
	}
	return child;
exit:
	free(pdata);
	return NULL;
}

void remove_child(int left_or_right,struct file_list *parent)
{
	struct file_list *child = NULL;
	char *pdata;
	if(NULL == parent){
		mine_dbg("parent is NULL\n");
	}
	if(left_or_right == RIGHT){
		child = parent->right_child;
	}else if(left_or_right == LEFT){
		child = parent->left_child;
	}
	if(NULL == child){
		mine_dbg("%s child is NULL\n", left_or_right?"Right":"Left");
		return;
	}
	pdata = child->path;
	if(pdata == NULL){
		mine_dbg("Path is NULL\n");
	} else {
		free(pdata);
		child->path = NULL;
	}
	free(child);
	if(left_or_right == RIGHT){
		parent->right_child = NULL;
	}else if(left_or_right == LEFT){
		parent->left_child = NULL;
	}
	return;
}

void destory_file_list(struct file_list *root)
{
	if(root){
		destory_file_list(root->left_child);
		destory_file_list(root->right_child);
//		if(root->path){
//			free(root->path);
//			root->path = NULL;
//		}
//		free(root);
		printf("%s\n", root->path);
	}
}

int get_file_path(struct file_list* root, char *target_file_name)
{
	DIR *dir;
	struct dirent *ptr;
	struct file_list* temp = root;
	struct file_list* new_file =NULL;
	int final_result = -1;	//0 mean find valid file in this folder; else mean not;
	if(root == NULL){
		mine_dbg("No root director\n");
		return -EINVAL;
	}
	if(root->path == NULL){
		mine_dbg("Cannot find target director\n");
		return -EINVAL;
	}
	if((dir = opendir(root->path)) == NULL){
		mine_dbg("Failed to open dir(%s)\n",root->path);
		return -EINVAL;
	}
	mine_dbg("success to open dir(%s)\n",root->path);
	
	while((ptr = readdir(dir)) != NULL){
		if(strcmp(ptr->d_name,".") == 0||strcmp(ptr->d_name,"..") == 0){
			continue;
		}else if(ptr->d_type == 8){ //file
			if(strstr(ptr->d_name,target_file_name)){
				if(temp == root){
					new_file = add_child(LEFT,temp,ptr->d_name);
				} else {
					new_file = add_child(RIGHT,temp,ptr->d_name);
				}
				if(NULL == new_file){
					mine_dbg("Failed to add node %s\n", ptr->d_name);
					continue;
				} else {
					mine_dbg("success to add node %s\n", ptr->d_name);
					temp = new_file;
					final_result = 0;
				}
			}
		} else if(ptr->d_type == 10){
			mine_dbg("%s just a link file\n", ptr->d_name);
		} else if(ptr->d_type == 4){
			if(temp == root){
				new_file = add_child(LEFT,temp,ptr->d_name);
			} else {
				new_file = add_child(RIGHT,temp,ptr->d_name);
			}
			if(NULL == new_file){
				mine_dbg("Failed to add node %s\n", ptr->d_name);
				continue;
			}
			chdir(root->path);
			mine_dbg("current work director is %s\n", getcwd(NULL, 0));
			if(0 == get_file_path(new_file, target_file_name)){
				temp = new_file;
				final_result = 0;
				mine_dbg("success to find %s in director %s\n", target_file_name, ptr->d_name);
			} else{
				if(temp == root){
					remove_child(LEFT,temp);
				} else {
					remove_child(RIGHT,temp);
				}
				mine_dbg("Failed to find %s in director %s\n", target_file_name, ptr->d_name);
			}
			chdir("..");
		}
	}
	closedir(dir);
	return final_result;
}

void print_file_list(struct file_list *root,char *path)
{
	static char file_path[100];
	char *temp_path = NULL;
	if(NULL == path){
		path = file_path;
	}
	temp_path = path;
	if(root){
		temp_path += sprintf(temp_path,"%s", root->path);
		if(NULL == root->left_child && NULL == root->right_child){
			*temp_path = '\0';
			printf("%s\n",file_path);
			//add code to process target file
		} 
		if(NULL != root->left_child){
			temp_path += sprintf(temp_path,"/");
		}
		print_file_list(root->left_child,temp_path);
		print_file_list(root->right_child,path);
	}
}


