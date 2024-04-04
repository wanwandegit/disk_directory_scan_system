#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "queue.h"

// 循环队列的创建
struct queue* queue_create()
{
	// 分配空间并初始化
	struct queue* p = (struct queue*)malloc(sizeof(struct queue));
	p->front = 0;
	p->rear = 0; 
	
	return p;
}

// 进队列
void push(struct queue* p, struct tree* t)
{
	p->data[p->rear] = t;  // 存入队列中 
	p->rear++; // 队尾指针加1
	if(p->rear == 50000) p->rear = 1;  // 队尾越界，则循环
}

// 出队列
struct tree* pop(struct queue* p)
{
	if(p->front != p->rear)
	{
		struct tree* t = p->data[p->front];  // 队头内容输出 
		p->front++;  // 队头指针加1
		if(p->front == 50000) p->front = 1;  // 队尾越界，则循环
		return t;
	}
}