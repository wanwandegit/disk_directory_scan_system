#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

// 创建孩子节点
struct tree* creat_first_child(struct tree* root, int index, char* path, int depth, int node_depth, time_t time, long int size)
{
	root->frist_child = (struct tree*)malloc(sizeof(struct tree));
	root->frist_child->index = index;
	strcpy(root->frist_child->path, path);
	root->frist_child->depth = depth;
	root->frist_child->node_depth = node_depth;
	root->frist_child->time = time;
	root->frist_child->size = size;
	return root->frist_child;
}

// 创建兄弟节点
struct tree* creat_next_sibling(struct tree* root, int index, char* path, int depth, int node_depth, time_t time, long int size)
{
	root->next_sibling = (struct tree*)malloc(sizeof(struct tree));
	root->next_sibling->index = index;
	strcpy(root->next_sibling->path, path);
	root->next_sibling->depth = depth;
	root->next_sibling->node_depth = node_depth;
	root->next_sibling->time = time;
	root->next_sibling->size = size;
	return root->next_sibling;
}

// 在目录树中查找目录或文件对应的节点
struct tree* tree_search(struct tree* root, char* path)
{
	struct tree* p = root;  // 目录树中的移动指针
	int n = 11;  // 路径中移动指针，从"C:/Windows/"后开始
	char t[256];  // 临时路径
	strcpy(t, "C:/Windows/");
	while(strcmp(p->path, path) != 0)
	{
		// 若移动指针指向'/'或'\0'，说明此时t为某一层目录或文件的路径
		if(path[n] == '/' || path[n] == '\0')
		{
			p = p->frist_child;
			while(strcmp(p->path, t) != 0)
			{
				p = p->next_sibling;
			}
		}
		
		t[n] = path[n];  // 更新临时路径
		n++;
		t[n] = '\0';
	}
	return p;
}

// 在目录树中查找目录或文件对应的节点及其父节点
struct tree* tree_search_parent(struct tree* root, char* path, int* type)
{
	struct tree* p = root;  // 目录树中的移动指针
	struct tree* f;  // 当前节点的父节点
	int n = 11;  // 路径中移动指针，从"C:/Windows/"后开始
	char t[256];  // 临时路径
	strcpy(t, "C:/Windows/");
	while(strcmp(p->path, path) != 0)
	{
		// 若移动指针指向'/'或'\0'，说明此时t为某一层目录或文件的路径
		if(path[n] == '/' || path[n] == '\0')
		{
			f = p;
			p = p->frist_child;
			*type = 0;
			while(strcmp(p->path, t) != 0)
			{
				f = p;
				p = p->next_sibling;
				*type = 1;
			}
		}
		
		t[n] = path[n];  // 更新临时路径
		n++;
		t[n] = '\0';
	}
	return f;
}

// 删除下属节点
void delete_sub_node(struct tree* p)
{
	p = p->frist_child;  // 读取当前目录下第一个节点
	
    // 循环读取目录中文件的信息
    while(p != NULL)
	{
        // 若是常规文件，直接删除
        if(p->frist_child == NULL)
		{
			free(p);
        }
        
        // 若是目录文件, 就递归后删除
        else if(p->frist_child != NULL)
		{
            delete_sub_node(p);
            free(p);
        }
        
        p = p->next_sibling;
    }
    
    return;
}