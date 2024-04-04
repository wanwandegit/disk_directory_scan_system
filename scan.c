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

// 若路径中存在'\'，则替换为'/'
void char_replace(char* p)
{
	int n = 0;
	for(; p[n] != '\0'; n++)
	{
		if(p[n] == '\\') p[n] = '/';
	}
}

// 判断一个字符串是否以另一个字符串开头
int start_with(const char *origin_string, char *prefix)
{
    // 参数校验
    if (origin_string == NULL || prefix == NULL || strlen(prefix) > strlen(origin_string))
	{
        printf("参数异常，请重新输入！\n");
        return -1;
    }
    
    int n = strlen(prefix);
    int i;
    for (i = 0; i < n; i++) {
        if (origin_string[i] != prefix[i])
		{
            return 1;
        }
    }
    return 0;
}

// 将标准格式的字符串按逗号拆分
struct file_operation split_string(char* p)
{
	int n = 0;  // 字符串p中的移动指针
	int cnt = 0;  // 其他字符串中的临时指针
	struct file_operation ret;
	char t[32];  // 临时存放字符串
	
	// 复制文件路径
	for(; p[n] != ','; n++)
	{
		ret.path[n] = p[n];
	}
	ret.path[n] = '\0';
	ret.path[0] = 'C';  // 处理不规范大小写
	ret.path[3] = 'W';  // 处理不规范大小写
	if(ret.path[n - 1] == '/')
	{
		ret.path[n - 1] = '\0';  // 去掉目录路径结尾的'/'
	}
	
	ret.operation = p[n + 1];  // 复制操作类型
	
	// 复制时间
	n = n + 3;
	for(; p[n] != ','; n++, cnt++)
	{
		t[cnt] = p[n];
	}
	t[cnt] = '\0';
	ret.time = atoi(t);
	
	// 复制大小
	n++;
	cnt = 0;
	for(; p[n] != '\0'; n++, cnt++)
	{
		t[cnt] = p[n];
	}
	t[cnt] = '\0';
	ret.size = atoi(t);
	
	return ret;
}

// 向日志文件中写入当前时间
void fprint_time(FILE* fp)
{
	time_t t;
	time(&t);  // 取得当前时间
	char stime[100];
	
	// 以"星期,月,日,小时:分:秒,年"的格式生成字符串
	strcpy(stime, ctime(&t));
	stime[strlen(stime) - 1] = '\0';  // 该字符串末尾('\0'前)有一个'\n'，此处将其替换为'\0'
	
	fprintf(fp, "[%s]", stime);
	return;
}

// 广度优先搜索算法遍历目录中文件并构建目录树，同时生成SQL文件
int read_dir(FILE* fp, struct total_info* info, struct tree* root, int* tree_depth)
{
	printf("Scanning progress: ");
	// 创建循环队列以存放待访问文件夹
	struct queue* nodes = queue_create();
	if(nodes == NULL)
	{
		printf("line%d : failed to create a queue\n", __LINE__);
        fprint_time(fp);
        fprintf(fp, "line%d : failed to create a queue\n", __LINE__);
        return -1;
	}
	
	DIR* dir;  // 定义目录指针
    char new_path[256];  // 子目录的全路径
    struct dirent* name_info;  // 用于记录当前指向文件的名称信息
    struct stat type_info;  // 用于记录当前指向文件的类型信息
    struct tree* p;  // 队列中的移动指针
    struct tree* t;  // 队列中的移动指针
    int cnt = 0;  // 记录内层循环次数
    int depth = 0;  // 当前节点在目录中的深度
    int node_depth = 0;  // 当前节点在目录树中的深度
    int index = 0;  // 索引
    FILE* sql;  // sql文件指针
    char sql_filename[20] = "insert__.sql";  // 文件表sql文件的文件名
    int sql_n = 0;  // sql文件个数
    long int dir_size = 0;  // 子目录文件大小
    time_t dir_mtime = 0;  // 子目录的修改时间
    char name_num[3];  // sql文件名标号
	
	push(nodes, root);  // 将需扫描目录路径放入队列
	index--;  // 需扫描目录无索引
	(info->subdir_count)--;  // 需扫描目录不属于子目录
	
	// 循环遍历直至队列为空
	while(nodes->front != nodes->rear)
	{
		p = pop(nodes);  // 从队列中取出待访问节点
		depth = p->depth + 1;  // 目录深度+1
		node_depth = p->node_depth;  // 取出当前节点在目录树中的深度
		(info->subdir_count)++;  // 子目录数+1
		
		// 获取当前子目录的修改时间
		stat(p->path, &type_info);
		dir_mtime = type_info.st_mtime;
		
		// 若当前节点在目录中的深度超过此前记录的最大层数，则更新
        if(depth > info->dir_layer_count)
        {
            info->dir_layer_count = depth;
		}
		
		// 获取目录中的文件
		if((dir = opendir(p->path)) == 0)  // 打开目录
		{
     	    printf("line%d : failed to open directory %s\n", __LINE__, p->path);
     	    fprint_time(fp);
     	    fprintf(fp, "line%d : failed to open directory %s\n", __LINE__, p->path);
    	    return -1;
    	}
    	
    	cnt = 0;  // 内层循环次数置零
    	dir_size = 0;  // 子目录文件大小置零
    	
    	// 循环读取目录中文件的信息
    	while((name_info = readdir(dir)) != NULL)
		{
        	if(strcmp(name_info->d_name, ".") == 0 || strcmp(name_info->d_name, "..") == 0) continue;  // 若读取到当前目录或父目录，则跳过
        	sprintf(new_path, "%s/%s", p->path, name_info->d_name);  // 形成带全路径的文件名
        	
        	// 获取文件属性
        	if(stat(new_path, &type_info) != 0)
        	{
            	printf("line%d : error : stat error!\n", __LINE__);
            	fprint_time(fp);
            	fprintf(fp, "line%d : error : stat error!\n", __LINE__);
            	return -1;
        	}
        	
        	node_depth++;
        	
        	// 若当前节点在目录树中深度超过此前记录的最大深度，则更新
        	if(node_depth > *tree_depth)
        	{
        		*tree_depth = node_depth;
			}
        	
        	// 创建节点并放入目录树中
    		if(cnt == 0)  // 第一次读取的节点视为孩子节点
    	   	{
        		t = creat_first_child(p, index, new_path, depth, node_depth, type_info.st_mtime, type_info.st_size);
			}
			else  // 其余节点视为兄弟节点
			{
				t = creat_next_sibling(t, index, new_path, depth, node_depth, type_info.st_mtime, type_info.st_size);
			}
        	
        	// 若是常规文件, 就放入文件表中
        	if(S_ISREG(type_info.st_mode) == 1)
			{ 
				// 若当前文件的文件名长度超过此前记录的最大长度，则更新最长文件名长度和文件路径
				if(strlen(new_path) > (info->max_filename_length))
            	{
            	   	(info->max_filename_length) = strlen(new_path);
            	    strcpy(info->max_filename, new_path);
            	}
            	
            	// 读取第一个文件的同时对记录中的文件相关信息进行初始化
            	if(info->file_count == 0)
            	{
            		strcpy(info->earliest_file_name, new_path);
            		info->earliest = type_info.st_mtime;
					info->earliest_size = type_info.st_size;
					strcpy(info->latest_file_name, new_path);
					info->latest = type_info.st_mtime;
					info->latest_size = type_info.st_size; 
				}
				
				// 若当前文件的修改时间早于最早时间或晚于最晚时间，则更新相关信息
				if((unsigned long int)type_info.st_mtime < (unsigned long int)(info->earliest))
				{
					strcpy(info->earliest_file_name, new_path);
					info->earliest = type_info.st_mtime;
					info->earliest_size = type_info.st_size;
				}
				else if((unsigned long int)type_info.st_mtime > (unsigned long int)(info->latest))
				{
					strcpy(info->latest_file_name, new_path);
					info->latest = type_info.st_mtime;
					info->latest_size = type_info.st_size;
				}
				
				(info->file_count)++;  // 文件数+1
				dir_size += type_info.st_size;  // 计算子目录大小
				(info->total_size) += type_info.st_size;  // 计算总文件大小
				index++;  // 索引+1
        	
        		// 每10000条sql语句形成一个sql文件
        		if(index % 10000 == 0)
        		{
        			printf("%c%c", 0xa8, 0x80);  // 每扫描10000个文件或文件夹，打印一个方块
        			sql_n++;  // sql文件数量+1
        			
        			// 设置文件名
        			if(sql_n < 10)
					{
						sql_filename[6] = '0';
						sql_filename[7] = sql_n + 48;
					}
					else
					{
						itoa(sql_n, name_num, 10);
						sql_filename[6] = name_num[0];
						sql_filename[7] = name_num[1];
					}
					
        			sql = fopen(sql_filename, "w");  // 新建文件表sql文件
				}
				
				// 将sql语句放入文件
				fprintf(sql, "INSERT INTO file_table VALUES ('%s', %d, %ld, %ld);\n", new_path, index, type_info.st_mtime, type_info.st_size);
        	}
        	
        	// 若是目录文件, 就放入目录表和队列中
  			else if(S_ISDIR(type_info.st_mode) == 1)
			{
    	        push(nodes, t);  // 将待访问子目录加入队列中
    	    }
    	    
    	    cnt++; // 循环次数+1
    	}
    	
    	index++;  // 索引+1
    	
    	// 每10000条sql语句形成一个sql文件
        if(index % 10000 == 0)
        {
        	printf("%c%c", 0xa8, 0x80);  // 每扫描10000个文件或文件夹，打印一个方块
        	sql_n++;  // sql文件数量+1
        	
       		// 设置文件名
        	if(sql_n < 10)
			{
				sql_filename[6] = '0';
				sql_filename[7] = sql_n + 48;
			}
			else
			{
				itoa(sql_n, name_num, 10);
				sql_filename[6] = name_num[0];
				sql_filename[7] = name_num[1];
			}
			
        	sql = fopen(sql_filename, "w");  // 新建文件表sql文件
		}
    	
    	// 将sql语句放入文件
		fprintf(sql, "INSERT INTO dir_table VALUES ('%s', %d, %ld, %ld);\n", p->path, index, dir_mtime, dir_size);
		
	}
    
    printf(" Finished!");
    closedir(dir);  // 关闭目录指针
    return 0;
}

// 统计目录信息
int dir_stat(struct tree* p, int depth, struct total_info* info)
{
	p = p->frist_child;  // 读取当前目录下第一个节点
	depth++;  // 目录深度+1
	
	// 若当前层数超过此前记录的文件层数，则更新文件层数
	if(depth > (info->dir_layer_count))
	{
		(info->dir_layer_count) = depth;
	}
    
    // 循环读取目录中文件的信息
    while(p != NULL)
	{
        // 若是常规文件
        if(p->frist_child == NULL)
		{
			// 若当前文件的文件名长度超过此前记录的最大长度，则更新最长文件名长度和文件路径
			if (strlen(p->path) > (info->max_filename_length))
            {
                (info->max_filename_length) = strlen(p->path);
                strcpy(info->max_filename, p->path);
            }
            
            // 读取第一个文件的同时对记录中的文件相关信息进行初始化
           	if(info->file_count == 0)
           	{
           		strcpy(info->earliest_file_name, p->path);
           		info->earliest = p->time;
				info->earliest_size = p->size;
				strcpy(info->latest_file_name, p->path);
				info->latest = p->time;
				info->latest_size = p->size; 
			}
			
			// 若当前文件的修改时间早于最早时间或晚于最晚时间，则更新相关信息
			if((unsigned long int)(p->time) < (unsigned long int)(info->earliest))
			{
				strcpy(info->earliest_file_name, p->path);
				info->earliest = p->time;
				info->earliest_size = p->size;
			}
			else if((unsigned long int)(p->size) > (unsigned long int)(info->latest))
			{
				strcpy(info->latest_file_name, p->path);
				info->latest = p->time;
				info->latest_size = p->size;
			}
			
			(info->file_count)++;  // 总文件个数+1
			(info->total_size) += p->size;  // 计算总文件大小
        }
        
        // 若是目录文件, 就递归
        else if(p->frist_child != NULL)
		{			
			(info->subdir_count)++;
            dir_stat(p, depth, info);
        }
        
        p = p->next_sibling;
    }
    
    return 0;
}

// 生成统计信息文件
void generate_stat_file(struct tree* root, FILE* fstat, FILE* fret)
{
	struct total_info dir_info;  // 需扫描目录的各项属性
    char dir_path[256];  // 需统计信息的目录的路径
    struct tree* p;  // 临时指针
    
    // 按行读取文件内容
    while (fscanf(fstat, "%[^\n] ", dir_path) != EOF)
	{
		if(strcmp(dir_path, "stat dirs") == 0 ||  strcmp(dir_path, "end of dirs") == 0) continue;
		
		char_replace(dir_path);  // 将路径中的'\'替换为'/'
		dir_path[strlen(dir_path) - 1] = '\0';  // 去掉路径结尾的'/'
		dir_path[0] = 'C';  // 处理不规范大小写
		dir_path[3] = 'W';  // 处理不规范大小写
		
		// 初始化需扫描目录的各项属性
		dir_info.subdir_count = 0;  // 子目录数量
    	dir_info.file_count = 0;  // 文件总数量
    	dir_info.dir_layer_count = 1;  // 目录层数
    	dir_info.max_filename_length = 0;  // 最长文件名长度
    	dir_info.total_size = 0;  // 文件总大小
		
		p = tree_search(root, dir_path);  // 找到目录对应节点
		dir_stat(p, 0, &dir_info);  // 扫描目录树并统计文件信息
		
		char etime[100];  // 格式化后的最早时间
		char ltime[100];  // 格式化后的最晚时间
		
		// 以"星期,月,日,小时:分:秒,年"的格式生成字符串
		strcpy(etime, ctime(&dir_info.earliest));
		etime[strlen(etime) - 1] = '\0';  // 该字符串末尾('\0'前)有一个'\n'，此处将其替换为'\0'
		strcpy(ltime, ctime(&dir_info.latest));
		ltime[strlen(ltime) - 1] = '\0';  // 该字符串末尾('\0'前)有一个'\n'，此处将其替换为'\0'
		
		// 将统计信息放入文件
		fprintf(fret, "Dir_path: %s", dir_path);
		if(p == NULL)  // 若查找不到该结点，则说明该结点已被删除
		{
			fprintf(fret, "[This dir has been deleted]");
		}
		fprintf(fret, "\n");
    	fprintf(fret, "Number of files: %d\n", dir_info.file_count);
    	fprintf(fret, "Directory level: %d\n", dir_info.dir_layer_count);
    	fprintf(fret, "Maximum filename length: %d\n", dir_info.max_filename_length);
    	fprintf(fret, "Longest filename: %s\n", dir_info.max_filename);
    	fprintf(fret, "Total size: %ld byte\n", dir_info.total_size);
    	fprintf(fret, "Earliest modification time: %s\n", etime);
    	fprintf(fret, "Earliest modified file: %s\n", dir_info.earliest_file_name);
    	fprintf(fret, "Size of the earliest modified file: %ld byte\n", dir_info.earliest_size);
    	fprintf(fret, "Latest modification time: %s\n", ltime);
    	fprintf(fret, "Latest modified file: %s\n", dir_info.latest_file_name);
    	fprintf(fret, "Size of the latest modified file: %ld byte\n", dir_info.latest_size);
    	fprintf(fret, "\n");
    }
}

// 比较统计文件
void compare_stat_file(FILE* fp_1, FILE* fp_2)
{
	char path_info[300];  // 当前目录对应路径信息
	char info_1[300];  // 当前行对应信息
	char info_2[300];  // 当前行对应信息
	int flag = 0;  // 标志两个文件是否存在差异
	
	// 按行读取文件内容
    while(fscanf(fp_1, "%[^\n] ", info_1) != EOF && fscanf(fp_2, "%[^\n] ", info_2) != EOF)
    {
    	// 若开头为"Dir_path"，则当前行对应信息为路径信息
    	if(start_with(info_1, "Dir_path") == 0)
    	{
    		strcpy(path_info, info_1);
		}
		if(start_with(info_2, "Dir_path") == 0)
    	{
    		strcpy(path_info, info_2);
		}
		
		// 若两个文件存在差异，则打印差异
		if(strcmp(info_1, info_2) != 0)
		{
			flag = 1;
			printf("%s\n", path_info);
			printf("Before operation: \n");
			printf("%s\n", info_1);
			printf("After operation: \n");
			printf("%s\n", info_2);
			printf("\n");
		}
	}
	
	if(flag == 0)
	{
		printf("There is no change after operation!\n");
		printf("\n");
	}
}

// 模拟文件操作
void modify(struct tree* root, struct file_operation ope)
{
	struct tree* p;  // 临时指针
	struct tree* m;  // 临时指针
	char t[256];  // 临时路径
	int n = 0;  // 临时路径中的指针
	int type = 0;  // 用于记录父节点查找函数返回的节点类型（孩子或兄弟）
	
	// 根据操作类型分类处理
	if(ope.operation == 'D')
	{
		p = tree_search_parent(root, ope.path, &type);  // 此时p为父节点
		if(type == 0)  // type为0说明为孩子节点
		{
			m = p->frist_child;
			delete_sub_node(m);
			p->frist_child = p->frist_child->next_sibling;
			free(m);
		}
		else  // 否则为兄弟节点
		{
			delete_sub_node(p->next_sibling);
			m = p->next_sibling;
			p->next_sibling = p->next_sibling->next_sibling;
			free(m);
		}
	}
	else if(ope.operation == 'M')
	{
		p = tree_search(root, ope.path);
		printf("\n");
		printf("Old time: %ld\n", p->time);
		printf("New time: %ld\n", ope.time);
		printf("Old size: %ld\n", p->size);
		printf("New size: %ld\n", ope.time);
		p->time = ope.time;
		p->size = ope.size;
	}
	else
	{
		// 找到文件上层目录对应的节点
		strcpy(t, ope.path);
		n = strlen(ope.path) - 1;
		for(; t[n] != '/'; n--)
		{
			t[n] = '\0';
		}
		t[n] = '\0'; 
		p = tree_search(root, t);
		
		if(p->frist_child == NULL)
		{
			p = creat_first_child(p, -1, ope.path, p->depth + 1, p->node_depth + 1, ope.time, ope.size);
		}
		else
		{
			p = p->frist_child;
			while((p->next_sibling) != NULL)
			{
				p = p->next_sibling;
			}
			p = creat_next_sibling(p, -1, ope.path, p->depth, p->node_depth + 1, ope.time, ope.size);
		}
	}
}