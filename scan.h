#pragma once
#ifndef _SCAN_H_
#define _SCAN_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <winsock.h>
#include "tree.h"
#include "queue.h"
#include "scan.h"

// 需扫描目录的各项属性
struct total_info
{
	int subdir_count;  // 子目录数量
    int file_count;  // 文件总数量
    int dir_layer_count;  // 目录层数
    int max_filename_length;  // 最长文件名长度
    char max_filename[256];  // 最长文件名的文件路径
    long int total_size;  // 文件总大小
    time_t latest;  // 最晚修改的文件时间
    time_t earliest;  // 最早修改的文件时间
    char latest_file_name[256];  // 最晚修改的文件名称（带全路径）
    char earliest_file_name[256];  // 最早修改的文件名称（带全路径）
    long int latest_size;  // 最晚修改的文件大小
    long int earliest_size;  // 最早修改的文件大小
};

// 文件操作的各项参数
struct file_operation
{
	char path[256];  // 文件路径
	char operation;  // 操作类型
	time_t time;  // 时间
	long int size;  // 大小
};

// 若路径中存在'\'，则替换为'/'
extern void char_replace(char* p);

// 判断一个字符串是否以另一个字符串开头
extern int start_with(const char *origin_string, char *prefix);

// 将标准格式的字符串按逗号拆分
extern struct file_operation split_string(char* p);

// 向日志文件中写入当前时间
extern void fprint_time(FILE* fp);

// 广度优先搜索算法遍历目录中文件并构建目录树，同时生成sql文件
extern int read_dir(FILE* fp, struct total_info* info, struct tree* root, int* tree_depth);

// 统计目录信息
extern int dir_stat(struct tree* p, int depth, struct total_info* info);

// 生成统计信息文件
extern void generate_stat_file(struct tree* root, FILE* fstat, FILE* fret);

// 比较统计文件
extern void compare_stat_file(FILE* fp_1, FILE* fp_2);

// 模拟文件操作
extern void modify(struct tree* root, struct file_operation ope);

#endif