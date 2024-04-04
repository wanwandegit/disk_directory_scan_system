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

char path[256] = "C:/Windows";  // 需扫描目录

int main()
{
	FILE* fp = fopen("disk_directory_scan_system.log", "a");  // 新建或打开日志文件，注意日志文件需正常退出后才能生成
	
	// 若日志文件无法打开，则报错并结束程序
    if(fp == NULL)
	{
		printf("line%d : The log file open fail.\n", __LINE__);
        exit(1);
    }
	
	// 初始化需扫描目录的各项属性
	struct total_info info;
	info.subdir_count = 0;  // 子目录数量
    info.file_count = 0;  // 文件总数量
    info.dir_layer_count = 1;  // 目录层数
    info.max_filename_length = 0;  // 最长文件名长度
    info.total_size = 0;  // 文件总大小
    int tree_depth = 0;  // 目录树深度
    
    // 创建目录树根结点
	struct tree* root = (struct tree*)malloc(sizeof(struct tree));
	if(root == NULL)
	{
		printf("line%d : failed to create a root node\n", __LINE__);
        fprint_time(fp);
        fprintf(fp, "line%d : failed to create a root node\n", __LINE__);
        exit(1);
	}
	root->index = 0;
	strcpy(root->path, path);
	root->depth = 0;
	root->node_depth = 1;
	
	printf("Scanning the directory meow! (=^-ω-^=)\n");
	read_dir(fp, &info, root, &tree_depth);  // 扫描目录并建树
	
	char etime[100];  // 格式化后的最早时间
	char ltime[100];  // 格式化后的最晚时间
	
	// 以"星期,月,日,小时:分:秒,年"的格式生成字符串
	strcpy(etime, ctime(&info.earliest));
	etime[strlen(etime) - 1] = '\0';  // 该字符串末尾('\0'前)有一个'\n'，此处将其替换为'\0'
	strcpy(ltime, ctime(&info.latest));
	ltime[strlen(ltime) - 1] = '\0';  // 该字符串末尾('\0'前)有一个'\n'，此处将其替换为'\0'
	
	printf("\n\n");
	printf("Depth of directory tree: %d\n", tree_depth);
	printf("Number of subdirectories: %d\n", info.subdir_count);
    printf("Number of files: %d\n", info.file_count);
    printf("Directory level: %d\n", info.dir_layer_count);
    printf("Maximum filename length: %d\n", info.max_filename_length);
    printf("Longest filename: %s\n", info.max_filename);
    printf("Total size: %ld byte\n", info.total_size);
    printf("Earliest modification time: %s\n", etime);
    printf("Earliest modified file: %s\n", info.earliest_file_name);
    printf("Size of the earliest modified file: %ld byte\n", info.earliest_size);
    printf("Latest modification time: %s\n", ltime);
    printf("Latest modified file: %s\n", info.latest_file_name);
    printf("Size of the latest modified file: %ld byte\n", info.latest_size);
    printf("\n");
    
    FILE* fstat = fopen("mystat.txt", "r");  // 记录需统计信息的目录的文件
    FILE* fret_1 = fopen("stat_info_1.txt", "w");  // 记录内层函数返回的各项统计信息
    
    // 若文件无法打开，则报错并结束程序
    if(fstat == NULL)
	{
		printf("line%d : The stat file open fail.\n", __LINE__);
		fprint_time(fstat);
        fprintf(fstat, "line%d : error : stat error!\n", __LINE__);
        exit(1);
    }
    
    generate_stat_file(root, fstat, fret_1);  // 生成统计信息文件
    
    FILE* fret_2;  // 记录内层函数返回的各项统计信息
    FILE* fop_1;  // 指向修改前统计信息文件的只读指针
    FILE* fop_2;  // 指向修改后统计信息文件的只读指针
    
    while(1)
    {
    	char operation[300];  // 单条文件操作
    	printf("\n");
    	printf("Now please input your operation meow: (Enter 0 to exit) (=^·ω·^=)\n");
    	scanf("%s", operation);
    	char_replace(operation);  // 将操作语句中的'\'替换为'/'
    	if(strcmp(operation, "0") == 0) break;  // 若输入为"0"，则退出
    	struct file_operation ope = split_string(operation);  // 将标准格式的字符串按逗号拆分
    	modify(root, ope);  // 进行模拟文件操作
    	printf("\n");
    	printf("Operation have completed meow! (=^●ω●^=)\n");
    	printf("\n");
    	
    	// 再次统计文件信息
    	fstat = fopen("mystat.txt", "r");  // 将fstat指针指向文件头部
    	fret_2 = fopen("stat_info_2.txt", "w");  // 将fstat指针指向文件头部并覆写文件
    	generate_stat_file(root, fstat, fret_2);  // 生成统计信息文件
    	fop_1 = fopen("stat_info_1.txt", "r");  // 以只读方式打开
    	fop_2 = fopen("stat_info_2.txt", "r");  // 以只读方式打开
    	compare_stat_file(fop_1, fop_2);  // 比较统计信息
	}
    
	return 0;
}