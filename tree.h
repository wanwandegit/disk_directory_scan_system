#pragma once
#ifndef _TREE_H_
#define _TREE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

// 目录树节点（孩子兄弟表示法）
struct tree
{
	int index;  // 索引
	char path[256];  // 路径
	int depth;  // 节点在目录中的深度
	int node_depth;  // 节点在树中的深度
	time_t time;  // 时间（仅针对文件有效，即叶子节点）
	long int size;  // 大小（仅针对文件有效，即叶子节点）
	struct tree *frist_child;  // 孩子指针
	struct tree *next_sibling;  // 兄弟指针
};

// 创建孩子节点
extern struct tree* creat_first_child(struct tree* root, int index, char* path, int depth, int node_depth, time_t time, long int size);

// 创建兄弟节点
extern struct tree* creat_next_sibling(struct tree* root, int index, char* path, int depth, int node_depth, time_t time, long int size);

// 在目录树中查找目录或文件对应的节点
extern struct tree* tree_search(struct tree* root, char* path);

// 在目录树中查找目录或文件对应的节点及其父节点
extern struct tree* tree_search_parent(struct tree* root, char* path, int* type);

// 删除下属节点
extern void delete_sub_node(struct tree* p);

#endif