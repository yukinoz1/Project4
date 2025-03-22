#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<easyx.h>
#include<time.h>
#include<graphics.h>
#include <conio.h>
#include<Windows.h>
#include <windows.h>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
//划分5*4个区域生成敌人
#define AREA_WIDTH (WIDTH / 5)  
#define AREA_HEIGHT ((HEIGHT - 100) / 4)  
#define AREA_COLS 5  
#define AREA_ROWS 4

ExMessage msg = { 0 };
//游戏状态
enum GameStatus
{
	GameReady,
	GameRunning,
	GameOver,
	GameWin,
	GamePause
};
GameStatus status = GameReady;
bool keyStates[256];
//定义常量
enum My {
	WIDTH = 1600, HEIGHT = 900,//主界面宽高
	BULLET_NUM = 30,//1型子弹数量
	BULLET2_NUM = 30,
	SHIP_SPEED = 10, BULLET_SPEED = 20,//飞机与子弹速度
	BIG, SMALL, ENEMY_NUM = 5, ENEMY_SPEED = 3, ENEMY_BulletNUM = 5, ENEMYbullet_SPEED = 6,//敌机数量与速度
	invisble_time = 2000//玩家受击后的无敌时间
};
double gravity = 1;//定义重力



//图片结构体
struct image {
	IMAGE img_background;
	IMAGE img_myplane[2];
	IMAGE img_enemy[2][2];
	IMAGE img_bullet[2];
	IMAGE img_bullet2[2];
	IMAGE img_enemyBullet[2][2];
	IMAGE img_laser[2];
}picture;
IMAGE airplane[4];


//飞机参数
struct plane {
	int x;
	int y;//坐标
	int vx;
	int vy;
	bool live;//判断是否存活
	int hp;
	int type;
	int width;
	int height;//飞机宽高
	int bullettype;//子弹类型
};
int score = 0;
int index = 0;
int flag_laser[ENEMY_NUM] = { 0 };
DWORD t_laser[ENEMY_NUM];//记录激光时间间隔
DWORD t_bullet1 = 0;//记录子弹1间隔
DWORD t_bullet2 = 0;//记录子弹2间隔
DWORD t_change = 0;//记录转换武器间隔
DWORD t_collision = 0;//碰撞时间间隔
DWORD bullet_stop = 0;//切换子弹后再发射子弹的间隔
int t_playeracg = 0;//主角动画播放时间
DWORD t_boom = 0;//敌人死亡音效间隔






struct plane myplane;
struct plane bullet[BULLET_NUM];
struct plane bullet2[BULLET2_NUM];
struct plane enemy[ENEMY_NUM];
struct plane enemyBullet[ENEMY_NUM][ENEMY_BulletNUM];
struct plane enemy_laser[ENEMY_NUM];

void Load();//加载图片
void init_game();//初始化游戏
void draw_game();//绘制游戏
void playeracg();//主角动画部分
void plane_move();
void create_bullet();//创建子弹
void create_bullet1();
void create_bullet2();
void create_enemy();//创建敌人
void create_enemyBullet();//创建敌人子弹
void create_laser();//创建激光
void draw_laser();//绘制激光
bool time(int ms, int id);//计时器
void shootplane();//射击
void collision();//碰撞判定

void  drawAlpha(IMAGE* picture, int  picture_x, int picture_y);
DWORD WINAPI MusicThread(LPVOID lpParam);//建立多线程
void music(char c[100]);//播放音效
void openmusic();//预先打开音乐


int main()
{
	openmusic();
	initgraph(WIDTH, HEIGHT);
	setbkmode(TRANSPARENT);
	cleardevice();
	init_game();
	music("voice\\bullet2.mp3");
	
	for (int i = 0; i < 256; i++)
	{
		keyStates[i] = false;
	}

	while (1)
	{
		

		peekmessage(&msg);
		if (msg.message == WM_KEYDOWN)
		{
			keyStates[msg.vkcode] = true;
		}
		else if (msg.message == WM_KEYUP)
		{
			keyStates[msg.vkcode] = false;
		}

		if (status == GameRunning)
		{
			plane_move();
			if (time(500, 0))
				create_enemy();
			shootplane();
			create_bullet();
			collision();
			if (time(800, 2))
				create_enemyBullet();
			if (time(3000, 3))
				create_laser();
		}

		BeginBatchDraw();
		cleardevice();

		draw_game();
		draw_laser();

		EndBatchDraw();

		msg.message = 0;
		Sleep(5);
	}
	system("pause");
}

//加载图片
void Load()
{
	loadimage(&picture.img_background, "image\\Background_1.png", 1600, 900);
	loadimage(&picture.img_bullet[0], "image\\bullet_pink1.jpg", 40, 20);
	loadimage(&picture.img_bullet[1], "image\\bullet_pink2.jpg", 40, 20);
	loadimage(&picture.img_bullet2[0], "image\\1.jpg", 50, 20);
	loadimage(&picture.img_bullet2[1], "image\\2.jpg", 50, 20);
	loadimage(&picture.img_enemy[0][0], "image\\smallenemy1.jpg", 80, 50);
	loadimage(&picture.img_enemy[0][1], "image\\smallenemy2.jpg", 80, 50);
	loadimage(&picture.img_enemy[1][0], "image\\bigenemy1.jpg", 160, 100);
	loadimage(&picture.img_enemy[1][1], "image\\bigenemy2.jpg", 160, 100);
	loadimage(&picture.img_enemyBullet[0][0], "image/smallenemy_bullet1.jpg", 30, 15);
	loadimage(&picture.img_enemyBullet[0][1], "image/smallenemy_bullet2.jpg", 30, 15);
	loadimage(&picture.img_enemyBullet[1][0], "image/bigenemy_bullet1.jpg", 40, 20);
	loadimage(&picture.img_enemyBullet[1][1], "image/bigenemy_bullet2.jpg", 40, 20);
	loadimage(&picture.img_laser[0], "image/laser1.jpg");
	loadimage(&picture.img_laser[1], "image/laser2.jpg");
	//主角动画
	char airplaneimage[64];	//定义数组，将图片的地址写入到数组
	loadimage(&airplane[0], "image\\3.png");
	loadimage(&airplane[1], "image\\4.png");
	loadimage(&airplane[2], "image\\5.png");
	loadimage(&airplane[3], "image\\6.png");



}

//初始化游戏
void init_game()
{
	

	Load();
	score = 0;
	//设置飞机相关参数
	myplane.width = 158;
	myplane.height = 111;
	myplane.hp = 10;
	myplane.vx = 0;
	myplane.vy = 0;
	myplane.x = 0;
	myplane.y = (HEIGHT - myplane.width) / 2;
	myplane.live = true;
	myplane.bullettype = 1;

	//初始化子弹1
	for (int i = 0; i < BULLET_NUM; i++)
	{
		bullet[i].width = 30;
		bullet[i].height = 15;
		bullet[i].vx = 0;
		bullet[i].vy = 0;
		bullet[i].x = 0;
		bullet[i].y = 0;
		bullet[i].live = false;
	}

	//初始化子弹2
	for (int i = 0; i < BULLET2_NUM; i++)
	{
		bullet2[i].width = 30;
		bullet2[i].height = 30;
		bullet2[i].vx = 0;
		bullet2[i].vy = 0;
		bullet2[i].x = 0;
		bullet2[i].y = 0;
		bullet2[i].live = false;
	}

	//初始化敌人
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		enemy[i].x = 0;
		enemy[i].y = 0;
		enemy[i].live = false;
	}

	//初始化敌人子弹
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		for (int j = 0; j < ENEMY_BulletNUM; j++)
		{
			enemyBullet[i][j].width = 30;
			enemyBullet[i][j].height = 15;
			enemyBullet[i][j].live = false;
		}
	}

	//激光初始化
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		enemy_laser[i].width = 512;
		enemy_laser[i].height = 256;
		enemy_laser[i].live = false;
	}
}

//判断坐标是否在矩形中
bool inArea(int mx, int my, int x, int y, int w, int h)
{
	if (mx > x && mx<x + w && my>y && my < y + h)
		return true;
	return false;
}

//按钮（可以变色）
bool button(int x, int y, int w, int h, const char* text)
{
	if (inArea(msg.x, msg.y, x, y, w, h))
	{
		setfillcolor(RGB(93, 107, 153));
	}
	else
	{
		setfillcolor(RGB(57, 214, 255));
	}

	setlinecolor(BLACK);
	fillroundrect(x, y, x + w, y + h, 5, 5);
	settextcolor(BLACK);
	settextstyle(30, 0, "微软雅黑");
	int wSpace = (w - textwidth(text)) / 2;
	int hSpace = (h - textheight(text)) / 2;
	outtextxy(x + wSpace, y + hSpace, text);

	if (msg.message == WM_LBUTTONDOWN && inArea(msg.x, msg.y, x, y, w, h))
	{
		return true;
	}
	return false;
}

//按钮（纯文本）
void button2(int x, int y, int w, int h, const char* text)
{
	setfillcolor(RGB(57, 214, 255));
	setlinecolor(BLACK);
	fillroundrect(x, y, x + w, y + h, 5, 5);
	settextcolor(BLACK);
	settextstyle(30, 0, "微软雅黑");
	int wSpace = (w - textwidth(text)) / 2;
	int hSpace = (h - textheight(text)) / 2;
	outtextxy(x + wSpace, y + hSpace, text);
}

//绘制游戏
void draw_game()
{

	putimage(0, 0, &picture.img_background);
	if (status != GameReady)
	{
		char SCORE[50];
		char HP[10];
		sprintf(SCORE, "score:%d", score);
		sprintf(HP, "hp:%d", myplane.hp);
		button2(0, 0, 100, 40, SCORE);
		button2(0, 50, 100, 40, HP);
	}

	if (status == GameReady)
	{
		int bx = (getwidth() - 250) / 2;
		if (button(bx, 400, 250, 50, "Start Game!"))
		{
			status = GameRunning;
		}
		if (button(bx, 500, 250, 50, "End Game"))
		{
			exit(0);
		}
	}
	else if (status == GameRunning) {


		playeracg();

		for (int i = 0; i < BULLET_NUM; i++)
		{
			if (bullet[i].live)
			{
				putimage(bullet[i].x, bullet[i].y, &picture.img_bullet[0], SRCAND);
				putimage(bullet[i].x, bullet[i].y, &picture.img_bullet[1], SRCPAINT);
			}
		}

		for (int i = 0; i < BULLET2_NUM; i++)
		{
			if (bullet2[i].live)
			{
				putimage(bullet2[i].x, bullet2[i].y, &picture.img_bullet2[0], SRCAND);
				putimage(bullet2[i].x, bullet2[i].y, &picture.img_bullet2[1], SRCPAINT);
			}
		}

		for (int i = 0; i < ENEMY_NUM; i++) {
			if (enemy[i].live) {
				if (enemy[i].type == BIG) {
					putimage(enemy[i].x, enemy[i].y, &picture.img_enemy[1][0], SRCAND);
					putimage(enemy[i].x, enemy[i].y, &picture.img_enemy[1][1], SRCPAINT);
				}
				else {
					putimage(enemy[i].x, enemy[i].y, &picture.img_enemy[0][0], SRCAND);
					putimage(enemy[i].x, enemy[i].y, &picture.img_enemy[0][1], SRCPAINT);
				}
			}
		}

		for (int i = 0; i < ENEMY_NUM; i++)
		{
			for (int j = 0; j < ENEMY_BulletNUM; j++)
			{
				if (enemyBullet[i][j].live)
				{
					putimage(enemyBullet[i][j].x, enemyBullet[i][j].y, &picture.img_enemyBullet[0][0], SRCAND);
					putimage(enemyBullet[i][j].x, enemyBullet[i][j].y, &picture.img_enemyBullet[0][1], SRCPAINT);
				}
			}
		}
	}
	else if (status == GameWin)
	{
	}
	else if (status == GamePause)
	{
		int bx = (getwidth() - 250) / 2;
		if (button(bx, 400, 250, 50, "重新开始"))
		{
			init_game();
			status = GameRunning;
		}
		if (button(bx, 500, 250, 50, "返回主菜单"))
		{
			init_game();
			status = GameReady;
		}
		if (button(bx, 600, 250, 50, "结束游戏"))
			exit(0);
	}
	else
	{
		int bx = (getwidth() - 250) / 2;
		{
			setfillcolor(RGB(57, 214, 255));
			setlinecolor(BLACK);
			fillroundrect((getwidth() - 800) / 2, 100, (getwidth() - 800) / 2 + 800, 100 + 200, 5, 5);
			settextcolor(LIGHTRED);
			settextstyle(100, 0, "微软雅黑");
			int wSpace = (800 - textwidth("YOU DEAD!")) / 2;
			int hSpace = (200 - textheight("YOU DEAD!")) / 2;
			outtextxy((getwidth() - 800) / 2 + wSpace, 100 + hSpace, "YOU DEAD!");
		}

		if (button(bx, 400, 250, 50, "重新开始"))
		{
			init_game();
			status = GameRunning;
		}
		if (button(bx, 500, 250, 50, "返回主菜单"))
		{
			init_game();
			status = GameReady;
		}
		if (button(bx, 600, 250, 50, "结束游戏"))
			exit(0);
	}
}

//绘制激光
void draw_laser()
{
	if (status == GameRunning)
		for (int i = 0; i < ENEMY_NUM; i++)
		{
			if (enemy_laser[i].live)
			{
				if (flag_laser[i] == 0)
				{
					putimage(enemy_laser[i].x, enemy_laser[i].y, enemy_laser[i].width, enemy_laser[i].height, &picture.img_laser[0], index * enemy_laser[i].width, 0, SRCAND);
					putimage(enemy_laser[i].x, enemy_laser[i].y, enemy_laser[i].width, enemy_laser[i].height, &picture.img_laser[1], index * enemy_laser[i].width, 0, SRCPAINT);
					index = (clock() / 200) % 7;
					flag_laser[i] = 1;
					t_laser[i] = clock();
				}
				else
				{
					putimage(enemy_laser[i].x, enemy_laser[i].y, enemy_laser[i].width, enemy_laser[i].height, &picture.img_laser[0], index * enemy_laser[i].width, 0, SRCAND);
					putimage(enemy_laser[i].x, enemy_laser[i].y, enemy_laser[i].width, enemy_laser[i].height, &picture.img_laser[1], index * enemy_laser[i].width, 0, SRCPAINT);
					index = (clock() / 200) % 7;
					if (clock() - t_laser[i] > 5000)
					{
						t_laser[i] = 0;
						flag_laser[i] = 0;
						enemy_laser[i].live = false;
					}
				}
			}
		}
}

//1 玩家移动：2 子弹位置更新： 3敌人位置更新
void plane_move()
{
	if (status != GameRunning)
		return;

	//玩家位置更新
	myplane.x += SHIP_SPEED * myplane.vx;
	myplane.y += SHIP_SPEED * myplane.vy;
	if (myplane.x >= WIDTH - myplane.width)
		myplane.x = WIDTH - myplane.width;
	if (myplane.x <= 0)
		myplane.x = 0;
	if (myplane.y <= 0)
		myplane.y = 0;
	if (myplane.y >= HEIGHT - myplane.height)
		myplane.y = HEIGHT - myplane.height;

	if (keyStates[VK_UP] || keyStates['W'])
	{
		myplane.vy = -1;
	}
	else if (keyStates[VK_DOWN] || keyStates['S'])
	{
		myplane.vy = 1;
	}
	else
	{
		myplane.vy = 0;
	}

	if (keyStates[VK_LEFT] || keyStates['A'])
	{
		myplane.vx = -1;
	}
	else if (keyStates[VK_RIGHT] || keyStates['D'])
	{
		myplane.vx = 1;
	}
	else
	{
		myplane.vx = 0;
	}

	

	if (keyStates[VK_ESCAPE])
	{
		status = GamePause;
	}

	if (keyStates['F'])
	{
		create_bullet2();
	}

	//子弹1位置更新
	for (int i = 0; i < BULLET_NUM; i++)
	{
		if (bullet[i].live)
		{
			bullet[i].x += BULLET_SPEED * bullet[i].vx;
		}
		if (bullet[i].x > WIDTH)
			bullet[i].live = false;
	}
	//子弹2位置更新
	for (int i = 0; i < BULLET2_NUM; i++)
	{
		if (bullet2[i].live)
		{
			bullet2[i].x += (double)BULLET_SPEED * bullet2[i].vx;
			bullet2[i].y += (double)BULLET_SPEED / 15 * bullet2[i].vy;
			bullet2[i].vy += gravity;
		}
		if (bullet2[i].x > WIDTH || bullet2[i].y > HEIGHT)
			bullet2[i].live = false;
	}

	//敌机位置更新
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		if (enemy[i].live) {
			if (enemy[i].type == BIG && enemy_laser[i].live)
				enemy[i].x = enemy[i].x;
			else
				enemy[i].x -= ENEMY_SPEED;
		}
		if (enemy[i].x < 0) {
			enemy[i].live = false;
		}
	}

	//敌机子弹更新
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		for (int j = 0; j < ENEMY_BulletNUM; j++)
		{
			if (enemyBullet[i][j].live)
			{
				enemyBullet[i][j].x -= ENEMYbullet_SPEED;
			}
			if (enemyBullet[i][j].x < 0)
			{
				enemyBullet[i][j].live = false;
			}
		}
	}
}

//创建子弹
void create_bullet1()
{
	DWORD currentTime = GetTickCount(); // 获取当前时间（以毫秒为单位）
	if (currentTime - t_bullet1 > 100)
	{
		for (int i = 0; i < BULLET_NUM; i++)
		{
			if (!bullet[i].live)
			{
				bullet[i].x = myplane.x + myplane.width;
				bullet[i].y = myplane.y + (myplane.height - bullet[i].height) / 2;
				bullet[i].live = true;
				bullet[i].vx = 1;
				break;
			}
		}
		t_bullet1 = currentTime;
	}
}
void create_bullet2()
{
	DWORD currentTime = GetTickCount(); // 获取当前时间（以毫秒为单位）


	if (currentTime - t_bullet2 > 1000)
	{
		PlaySound("voice\\1.wav", NULL, SND_ASYNC);
		{
			for (int i = 0; i < BULLET_NUM; i++)
			{
				if (!bullet2[i].live)
				{
					bullet2[i].x = myplane.x + myplane.width;
					bullet2[i].y = myplane.y + (myplane.height - bullet2[i].height) / 2;
					bullet2[i].live = true;
					bullet2[i].vx = 1;
					bullet2[i].vy = -10;
					break;
				}
			}t_bullet2 = currentTime;
		}

	}
}



void create_bullet()
{
	DWORD current_time = GetTickCount();
	if (current_time - bullet_stop > 600)
	{
		if (myplane.bullettype == 1)
			create_bullet1();
		else
			create_bullet2();
	}

	
	
}


//创建敌人
//创建敌人
void create_enemy()
{
	if (status != GameRunning)
		return;
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		if (!enemy[i].live)
		{
			int flag = rand() % 10;
			if (flag >= 0 && flag < 2) {
				enemy[i].type = BIG;
				enemy[i].hp = 4;
				enemy[i].width = 160;
				enemy[i].height = 100;
			}
			else {
				enemy[i].type = SMALL;
				enemy[i].hp = 2;
				enemy[i].width = 80;
				enemy[i].height = 50;
			}
			int area_x = rand() % AREA_COLS;  // 随机选择横向区域
			int area_y = rand() % AREA_ROWS;  // 随机选择纵向区域
			enemy[i].x = area_x * AREA_WIDTH + rand() % AREA_WIDTH;  // 在选定区域内生成x坐标
			enemy[i].y = area_y * AREA_HEIGHT + rand() % AREA_HEIGHT;  // 在选定区域内生成y坐标
			if (enemy[i].x > WIDTH / 3 * 1)//保证敌机生成在右边
				enemy[i].live = true;
			break;
		}
	}
}

//创建敌人子弹
void create_enemyBullet()
{
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		for (int j = 0; j < ENEMY_BulletNUM; j++)
		{
			if (!enemyBullet[i][j].live && enemy[i].live)
			{
				if (enemy[i].type == SMALL)
				{
					enemyBullet[i][j].x = enemy[i].x - enemyBullet[i][j].width;
					enemyBullet[i][j].y = enemy[i].y + (enemy[i].height - enemyBullet[i][j].height) / 2;
					enemyBullet[i][j].live = true;
					break;
				}
			}
		}
	}
}

//创建敌人激光
void create_laser()
{
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		if (!enemy_laser[i].live && enemy[i].live && enemy[i].type == BIG)
		{
			enemy_laser[i].x = enemy[i].x - enemy_laser[i].width;
			enemy_laser[i].y = enemy[i].y + (enemy[i].height - enemy_laser[i].height) / 2;
			enemy_laser[i].live = true;
			break;
		}
	}
}

//计时器
bool time(int ms, int id)
{
	static DWORD t[10];
	if (clock() - t[id] > ms)
	{
		t[id] = clock();
		return true;
	}
	return false;
}

//碰撞检测
bool check_crash(struct plane a, struct plane b)
{
	if (a.x + a.width > b.x && a.x < b.x + b.width && a.y<b.y + b.height && a.y + a.height>b.y)
		return true;
	return false;
}

void shootplane()
{
	for (int i = 0; i < ENEMY_NUM; i++)
	{
		if (!enemy[i].live)
		{
			continue;
		}
		for (int j = 0; j < BULLET_NUM; j++)
		{
			if (!bullet[j].live)
				continue;
			if (check_crash(bullet[j], enemy[i]))
			{
				bullet[j].live = false;
				enemy[i].hp--;
				DWORD current_time = GetTickCount();
				if (current_time - t_boom >= 2000)
				{
					PlaySound("voice\\enemy_death.wav", NULL, SND_ASYNC);
					t_boom = current_time;
				}
			}
			if (enemy[i].hp <= 0)
			{
				enemy[i].live = false;
				t_laser[i] = 0;
				flag_laser[i] = 0;
				enemy_laser[i].live = false;
				if (enemy[i].type == BIG)
					score += 5;
				else
					score += 1;
				break;
			}
		}
		if (!enemy[i].live)
		{
			continue;
		}
		for (int j = 0; j < BULLET2_NUM; j++)
		{
			if (!bullet2[j].live)
				continue;
			if (check_crash(bullet2[j], enemy[i]))
			{
				bullet2[j].live = false;
				enemy[i].hp -= 2;
			}
			if (enemy[i].hp <= 0)
			{
				enemy[i].live = false;
				t_laser[i] = 0;
				flag_laser[i] = 0;
				enemy_laser[i].live = false;
				if (enemy[i].type == BIG)
					score += 5;
				else
					score += 1;
				break;
			}
		}
	}
}

void collision()
{
	DWORD current_time = GetTickCount();

	for (int i = 0; i < ENEMY_NUM; i++)
	{
		if (current_time - t_collision >= invisble_time)
			if (enemy[i].live && check_crash(myplane, enemy[i]))
			{
				myplane.hp--;
				
				
				t_collision = current_time;
				enemy[i].live = false;

				if (enemy[i].hp == 0)
				{
					
					enemy[i].live = false;
					t_laser[i] = 0;
					flag_laser[i] = 0;
					enemy_laser[i].live = false;
					
					if (enemy[i].type == BIG)
						score += 5;
					else
						score += 1;
				}

				if (myplane.hp == 0)
					status = GameOver;


			}
	}

	for (int i = 0; i < ENEMY_NUM; i++)
	{
		for (int j = 0; j < ENEMY_BulletNUM; j++)
		{

			if (current_time - t_collision >= invisble_time)
				if (enemyBullet[i][j].live && check_crash(myplane, enemyBullet[i][j]))
				{
					enemyBullet[i][j].live = false;
					myplane.hp--;
					t_collision = current_time;
				}
			if (myplane.hp == 0)
				status = GameOver;
		}
	}

	for (int i = 0; i < ENEMY_NUM; i++)
	{

		if (current_time - t_collision >= invisble_time)
			if (enemy_laser[i].live && check_crash(myplane, enemy_laser[i]))
			{
				myplane.hp--;
				t_collision = current_time;
			}
		if (myplane.hp == 0)
			status = GameOver;
	}
}
//// 载入PNG图并去透明部分
void drawAlpha(IMAGE* picture, int  picture_x, int picture_y) //x为载入图片的X坐标，y为Y坐标
{

	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16); //获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP0x00007FF6033B63ED 处(位于 Project4.exe 中)引发的异常: 0xC0000005: 读取位置 0x000001941DF0F008 时发生访问冲突。+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //αp=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //αp=sa/255 , FP=sb , BP=db
			}
		}
	}
}
void playeracg()
{
	DWORD current_time = GetTickCount();
	DWORD a = current_time - t_playeracg;
	int i = a / 250;
	if (i > 3) {
		i = 0;
		t_playeracg = current_time;
	}

	drawAlpha(&airplane[i], myplane.x, myplane.y);//绘制透明图片



}

DWORD WINAPI MusicThread(LPVOID lpParam)
{
	char* c = (char*)lpParam;
	char command[200];
	// 停止当前音频
	sprintf(command, "stop %s", c);
	mciSendString(command, NULL, 0, NULL);


	// 关闭音频文件
	sprintf(command, "close %s", c);
	mciSendString(command, NULL, 0, NULL);


	// 播放音频文件
	sprintf(command, "play %s", c);
	mciSendString(command, NULL, 0, NULL);


	return 0;
}


void music(char c[100])
{
	HANDLE hThread = CreateThread(NULL, 0, MusicThread, c, 0, NULL);
	if (hThread != NULL)
	{
		// 关闭线程句柄，让线程在后台运行
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
}

void openmusic()
{
	mciSendString("open voice\\bullet2.mp3", NULL, 0, NULL);
	mciSendString("open voice\\1.mp3", NULL, 0, NULL);
	mciSendString("open voice\\enemy_death.mp3", NULL, 0, NULL);
}