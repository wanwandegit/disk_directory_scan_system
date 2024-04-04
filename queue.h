#pragma once
#ifndef _QUEUE_H_
#define _QUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "queue.h"

// 循环队列的结构，用于存放当前路径
struct queue
{
	struct tree* data[50000];  // 队列长度不超过50000
	int front;
	int rear;
};

// 循环队列的创建
extern struct queue* queue_create();

// 进队列
extern void push(struct queue* p, struct tree* t);

// 出队列
extern struct tree* pop(struct queue* p);

#endif