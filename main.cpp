#include <stdio.h>
#include <conio.h>
#include <graphics.h>
#include <time.h>

#define MAX_X 8
#define MAX_Y 8

FILE *path_data;
int path_x[8] = {-2, -1, +1, +2, +2, +1, -1, -2};	//下一个探索路径点的x坐标
int path_y[8] = {+1, +2, +2, +1, -1, -2, -2, -1};	//下一个探索路径点的y坐标
int map[MAX_X][MAX_Y];	//棋盘

IMAGE knight[2];
IMAGE temp_block;
long runtime;	//运行时间
int flag_way = 0;	//算法选择标志

typedef struct horse_stack
{
	int	x;
	int y;
	int flag_check;
	int flag_sort;
}horse_stack;				//栈

void init_map();
void set_start(int *set_x, int *set_y);
void init_stack(horse_stack *stack, int set_x, int set_y);	//初始化堆栈
unsigned int explore_path_dfs(horse_stack *stack, int set_x, int set_y);	//探索路径 DFS
unsigned int explore_path_hungry(horse_stack *stack, int set_x, int set_y);
int draw_map();
void horse_run(horse_stack *stack, int set_x, int sey_y);

int main(void)
{
	int set_x = 0, set_y = 0;
	unsigned int num_path;
	horse_stack stack[120];
	char ch = 0;

	initgraph(480, 480);
	loadimage(&knight[0], "images\\knight_1.bmp");
	loadimage(&knight[1], "images\\knight_2.bmp");
	if(!(path_data = fopen("path-data.txt", "wt")))
	{
		exit(1);
	}
	while (ch != 27)
	{
		init_map();
		set_start(&set_x, &set_y);
		init_stack(stack, set_x, set_y);
		runtime = clock();
		if (flag_way)
		{
			num_path = explore_path_hungry(stack, set_x, set_y);
		}
		else
		{
			num_path = explore_path_dfs(stack, set_x, set_y);
		}
		if (!num_path)
		{
			outtextxy(340, 5, "Not Found...");
		}
		outtextxy(240, 460, "Push ESC Quit,Other Key Reload!");
		fflush(stdin);
		ch = getch();
	}
	fclose(path_data);
	closegraph();
	return 0;
}

void init_map()
{
	int i, j, k = 0;

	setbkcolor(RGB(153, 153, 51));
	cleardevice();
	setcolor(RGB(255, 153, 0));
	for (i = 40; i <= 440; i += 50)
	{
		line(40, i, 440, i);
		line(i, 40, i, 440);
	}
	for (i = 44; i < 440; i += 50)
	{
		for(j = 44; j < 440; j += 50)
		{
			if (k % 2 == 1)
			{
				setfillcolor(RGB(102, 51, 0));
				floodfill(i, j, RGB(255, 153, 0));
			}
			else
			{
				setfillcolor(RGB(255, 153, 0));
				floodfill(i, j, RGB(255, 153, 0));
			}
			++k;
		}
		++k;
	}
	for (i = 0;i < MAX_X; i++)
	{
		for (j = 0; j < MAX_Y; j++)
		{
			map[i][j] = 0;
		}
	}

}

void set_start(int *set_x, int *set_y)
{
	MOUSEMSG m;
	IMAGE temp_now;
	int temp_x = 0, temp_y = 0, i = 0, block_x, block_y;
	LOGFONT f;
	char buffer[20];

	setcolor(BLACK);
	getfont(&f);                          
	f.lfHeight = 16;                     
	strcpy(f.lfFaceName, "宋体");        
	f.lfQuality = ANTIALIASED_QUALITY;    
	setfont(&f);                          // 设置字体样式
	outtextxy(200, 5, "马踏棋盘");
	outtextxy(170, 20, "左击:DFS 右击:贪心");
	while (1)
	{
		++i;
		m = GetMouseMsg();
		if ((m.x != temp_x || m.y != temp_y) && i != 1)
		{
			block_x = m.x - (m.x - 40) % 50;
			block_y = m.y - (m.y - 40) % 50;
			putimage(temp_x - 25, temp_y - 25, &temp_now);
			getimage(&temp_block, block_x, block_y, 50, 50);
			temp_x = m.x;
			temp_y = m.y;
			getimage(&temp_now, m.x - 25, m.y - 25, 50, 50);
			putimage(m.x - 25, m.y - 25, &knight[0], SRCAND);
			putimage(m.x - 25, m.y - 25, &knight[1], SRCPAINT);
		}
		if (m.x > 40 && m.x < 440 && m.y > 40 && m.y < 440)
		{
			sprintf(buffer, "x = %-10d", (block_y - 40)/50);
			outtextxy(5, 5, buffer);
			sprintf(buffer, "y = %-10d", (block_x - 40)/50);
			outtextxy(5, 20, buffer);
			if (m.uMsg == WM_LBUTTONDOWN || m.uMsg == WM_RBUTTONDOWN)
			{
				outtextxy(340, 5, "Exploring...");
				if (m.uMsg == WM_LBUTTONDOWN)
				{
					flag_way = 0;
				}
				if (m.uMsg == WM_RBUTTONDOWN)
				{
					flag_way = 1;
				}
				*set_x = (block_y - 40)/50;
				*set_y = (block_x - 40)/50;
				putimage(temp_x - 25, temp_y - 25, &temp_now);
				putimage(block_x, block_y, &temp_block);
				putimage(block_x, block_y, &knight[0], SRCAND);
				putimage(block_x, block_y, &knight[1], SRCPAINT);
				break;
			}
		}
	}
}

void init_stack(horse_stack *stack, int set_x, int set_y)
{
	stack[0].x = -1;
	stack[0].y = -1;
	stack[0].flag_check = -1;	//栈底
}

unsigned int explore_path_dfs(horse_stack *stack, int set_x, int set_y)
{
	int i, k, j, top = 0, path_switch = 0;		//累加器     栈顶   路径切换
	int now_x = set_x, now_y = set_y, next_x, next_y;	//当前坐标    下一个路径点坐标
	int zero_flag;		//找到下一个路径点的标志
	int check_begin = 0;	
	int find_flag = 0, end_flag = 0;	//找到一条符合条件的路径的标志    搜索了所有情况的标志
	unsigned int num_path = 0;
	char buffer[20];
	UINT64 num_step = 0;

	while (!find_flag && !end_flag)
	{
		zero_flag = 0;	//有可达路径点标志
		for (i = check_begin; i < 8 && !zero_flag; )
		{
			next_x = now_x + path_x[i];	//下一个路径点x坐标
			next_y = now_y + path_y[i];	//			  y
			if (next_x >= 0 && next_y >= 0 && next_x < MAX_X && next_y < MAX_Y && map[next_x][next_y] == 0)		//下个路径点坐标是否合法
			{
				++top;
				check_begin = 0;
				stack[top].x = now_x;//保存当前坐标
				stack[top].y = now_y;
				map[now_x][now_y] = top;		//标记已探索路径点
				now_x = next_x;
				now_y = next_y;	
				stack[top].flag_check = i;		//标记已搜索下一个路径点
				zero_flag = 1;
				num_step += 1;
			}
			i++;
		}
		if(i == 8 && !zero_flag)	//无下一个可达点
		{
			if(top == MAX_X * MAX_Y - 1)	//判断是否已全部遍历
			{
				sprintf(buffer, "Time = %lfs", (double)(clock() - runtime)/CLOCKS_PER_SEC);
				outtextxy(5, 445, buffer);
				outtextxy(340, 5, "Found!!!Running..");
				sprintf(buffer, "Point = %I64u", num_step);
				outtextxy(240, 445, buffer);
				stack[top + 1].x = now_x;
				stack[top + 1].y = now_y;
				map[now_x][now_y] = MAX_X * MAX_Y;
				find_flag = 1;			//找到一条路径的标志
 				num_path += 1;
// 				printf("\r%3u", num_path);
				draw_map();
				horse_run(stack, set_x, set_y);
			}
			if(!find_flag)
			{
				if (stack[top].flag_check == -1)	//是否已到栈底
				{	
					end_flag = 1;			//结束标志置1
				}
				else
				{
					map[now_x][now_y] = 0;	//取消标记
					now_x = stack[top].x;	
					now_y = stack[top].y;
					check_begin = stack[top].flag_check + 1;	//非栈底的话 出栈 改变当前路径点 重新探索
					--top;
				}
			}
		}
// 		for (k = 0; k < MAX_X; k++)
// 		{
// 			for (j = 0; j < MAX_Y; j++)
// 			{
// 				fprintf(path_data, "%3d", map[k][j]);
// 			}
// 			fprintf(path_data, "\n");
// 		}
// 		fprintf(path_data, "\n");
	}
	return num_path;
}

unsigned int explore_path_hungry(horse_stack *stack, int set_x, int set_y)
{
	int i, j, k;
	int now_x = set_x, now_y = set_y;
	int next_x, next_y;
	int temp_x, temp_y;
	int num, min;
	int temp_table[8], sort_table[8];
	int find_flag = 0;
	int check_begin = 0, top = 0;
	int end_flag = 0;
	char buffer[20];
	UINT64 num_step = 0;

	map[set_x][set_y] = 1;
	while(top >= 0)
	{
		memset(temp_table, 9, sizeof(temp_table));
		for (i = 0; i < 9; i++)
		{
			next_x = now_x + path_x[i];
			next_y = now_y + path_y[i];
			num = 0;
			if (next_x >= 0 && next_y >=0 && next_x < MAX_X && next_y < MAX_Y && map[next_x][next_y] == 0)
			{
				for (j = 0; j < 8; j++)
				{
					temp_x = next_x + path_x[j];
					temp_y = next_y + path_y[j];
					if (temp_x >= 0 && temp_y >=0 && temp_x < MAX_X && temp_y < MAX_Y && map[temp_x][temp_y] == 0)
					{
						num += 1;
					}	
				}
				temp_table[i] = num;
			}
		}
		for (i = 0; i < 8; i++)
		{
			min = 9;
			for (j = 0; j < 8; j++)
			{
				if (temp_table[j] < min)
				{
					min = temp_table[j];
					sort_table[i] = j;
					k = j;
				}
			}
			temp_table[k] = 9;
		}
		find_flag = 0;
		for (i = check_begin; i < 8; i++)
		{
			next_x = now_x + path_x[sort_table[i]];
			next_y = now_y + path_y[sort_table[i]];
			if (next_x >= 0 && next_y >=0 && next_x < MAX_X && next_y < MAX_Y && map[next_x][next_y] == 0)
			{
				num_step += 1;
				find_flag = 1;
				break;
			}
		}
		if (find_flag)
		{
			map[now_x][now_y] = top+1;
			++top;
			stack[top].x = now_x;
			stack[top].y = now_y;
			now_x = next_x;
			now_y = next_y;
			stack[top].flag_sort = i;
			stack[top].flag_check = sort_table[i];
			check_begin = 0;
			if (top >= MAX_X * MAX_Y - 1)
			{
				sprintf(buffer, "Time = %lfs", (double)(clock() - runtime)/CLOCKS_PER_SEC);
				outtextxy(5, 445, buffer);
				outtextxy(340, 5, "Found!!!Running..");
				sprintf(buffer, "Point = %I64u", num_step);
				outtextxy(240, 445, buffer);
				stack[top + 1].x = now_x;
				stack[top + 1].y = now_y;
				map[now_x][now_y] = MAX_X * MAX_Y;
				draw_map();
				horse_run(stack, set_x, set_y);
				return 1;
			}
		}
		else
		{
			map[now_x][now_y] = 0;
			check_begin = stack[top].flag_sort + 1;
			now_x = stack[top].x;
			now_y = stack[top].y;
			top--;
		}
	}
	return 0;
}

int draw_map()
{
	int i, j;

	for (i = 0; i < MAX_X; i++)
	{
		for (j = 0; j < MAX_Y; j++)
		{
			fprintf(path_data, "%3d", map[i][j]);
		}
		fprintf(path_data, "\n");
	}
	fprintf(path_data, "\n");
	fflush(path_data);
	return 0;
}

void horse_run(horse_stack *stack, int set_x, int set_y)
{
	int i, j;
	int next_x = set_x * 50 + 40 + 25, next_y = set_y * 50 + 40 + 25;
	int temp_x = next_y, temp_y = next_x;
	IMAGE temp_now = temp_block;
	char buffer[20];

	putimage(temp_x - 25, temp_y - 25, &temp_now);
	for (i = 2; i <= MAX_X * MAX_Y ; i++)
	{
		sprintf(buffer, "Step = %d", i == MAX_X * MAX_Y ? MAX_X*MAX_Y : i - 1);
		outtextxy(5, 460, buffer);
		for (j = 0; j < 50; j++)
		{
			getimage(&temp_now, temp_x - 25, temp_y - 25, 50, 50);
			putimage(temp_x - 25, temp_y - 25, &knight[0], SRCAND);
			putimage(temp_x - 25, temp_y - 25, &knight[1], SRCPAINT);
			Sleep(5);
			putimage(temp_x - 25, temp_y - 25, &temp_now);
			setcolor(BLACK);
			fillcircle(temp_x, temp_y, 2);
			fillcircle(temp_x, temp_y, 1);
			temp_y = next_x + j * path_x[stack[i - 1].flag_check];
			temp_x = next_y + j * path_y[stack[i - 1].flag_check];
		}
		next_y = stack[i].y * 50 + 40 + 25;
		next_x = stack[i].x * 50 + 40 + 25;
	}
	putimage(temp_x - 25, temp_y - 25, &knight[0], SRCAND);
	putimage(temp_x - 25, temp_y - 25, &knight[1], SRCPAINT);
	outtextxy(340, 5, "Finshed!-----     ");
}