#include <windows.h>
#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <vector>
using namespace std;

// 每颗烟花的粒子数量
#define PARTICLE_NUMBER 45
// 同时存在的最大烟花数
#define MAX_FIREWORK_NUMBER 6
// 每颗烟花运动时长
#define FIREWORK_DURATION 150

/*
* 速度结构体
* x 表示横向运动速度，y 表示纵向运动速度
*/
typedef struct 
{
	float x;
	float y;
} Speed;

/*
* 烟花粒子结构体
* x 表示横坐标，y 表示纵坐标
* speed 表示粒子当前运动速度
* enable 表示粒子是否存在
*/
typedef struct 
{
	float x = 0;
	float y = 0;
	Speed speed;
	bool enable = true;
} Particle;

/*
* 烟花结构体
* x、y 表示烟花爆炸点坐标
* particles 储存了该烟花的粒子群
* R、G、B 表示该烟花的颜色
* radius 表示烟花伪半径，该值越大，烟花半径越大
* gap 记录了烟花被绘制的次数
* number 记录了当前存在的烟花数量
*/
typedef struct 
{
	float x;
	float y;
	vector<Particle> particles;
	BYTE R = 255;
	BYTE G = 255;
	BYTE B = 255;
	float radius = 2.0;
	int gap = 0;
	static int number;
} Firework;

int Firework::number = 0;

// 全局变量定义
// 烟花群
vector<Firework> fws;
// 重力值
const float gravity = 0.006f;

// 全屏模糊函数
void Blur()
{
	static DWORD* pMem = GetImageBuffer(); // 获取显存地址
	for (int i = 800; i < 800 * 599; i++)
	{
		pMem[i] = RGB(
			(GetRValue(pMem[i]) + GetRValue(pMem[i - 800]) + GetRValue(pMem[i - 1]) + GetRValue(pMem[i + 1]) + GetRValue(pMem[i + 800])) / 5,
			(GetGValue(pMem[i]) + GetGValue(pMem[i - 800]) + GetGValue(pMem[i - 1]) + GetGValue(pMem[i + 1]) + GetGValue(pMem[i + 800])) / 5,
			(GetBValue(pMem[i]) + GetBValue(pMem[i - 800]) + GetBValue(pMem[i - 1]) + GetBValue(pMem[i + 1]) + GetBValue(pMem[i + 800])) / 5);
	}
}

// 精准延时函数
void HpSleep(int ms)
{
	static clock_t oldclock = clock();		// 静态变量，记录上一次 tick

	oldclock += ms * CLOCKS_PER_SEC / 1000;	// 更新 tick

	if (clock() > oldclock)					// 如果已经超时，无需延时
		oldclock = clock();
	else
		while (clock() < oldclock)			// 延时
			Sleep(1);						// 释放 CPU 控制权，降低 CPU 占用率
}

// 在 （x，y） 坐标处添加烟花
bool AddFirework(short x, short y)
{
	// 若当前烟花数量达到最大值，返回 false
	if (Firework::number == MAX_FIREWORK_NUMBER) return false;
	// 创建烟花
	Firework fw;
	// 设置烟花爆炸点
	fw.x = x;
	fw.y = y;
	// 设置烟花颜色
	fw.R = rand() % 146 + 110;
	fw.G = rand() % 146 + 110;
	fw.B = rand() % 146 + 110;
	// 随机烟花粒子位置
	for (int i = 0; i < PARTICLE_NUMBER; i++)
	{
		// 创建粒子
		Particle p;
		// 在圆形范围内随机粒子速度
		do {
			p.speed.x = (rand() % 201) / (200 / fw.radius / fw.radius) - fw.radius;
			p.speed.y = (rand() % 201) / (200 / fw.radius / fw.radius) - fw.radius;
		} while ((p.speed.x * p.speed.x + p.speed.y * p.speed.y) >= fw.radius * fw.radius);
		// 将粒子添加进粒子群
		fw.particles.push_back(p);
	}
	// 烟花数量+1
	Firework::number++;
	// 将该烟花添加进烟花群
	fws.push_back(fw);
	return true;
}

// 移动全部烟花粒子
void MoveFireworkParticle()
{
	// 临时变量
	int x, y;
	for (int i = 0; i < Firework::number; i++)
	{
		if (fws[i].gap++ < FIREWORK_DURATION) {
			for (int j = 0; j < PARTICLE_NUMBER; j++)
			{
				// 若粒子不存在，跳过该粒子
				if (!fws[i].particles[j].enable)
					continue;
				x = int(fws[i].particles[j].x + fws[i].x);
				y = int(fws[i].particles[j].y + fws[i].y);
				// 若粒子出界，将粒子存在标志置为 false
				if (x < 5 || x > (800 - 5) || y < 5 || y > (600 - 5))
				{
					fws[i].particles[j].enable = false;
					continue;
				}
				// 移动粒子位置
				fws[i].particles[j].x -= fws[i].particles[j].speed.x;
				fws[i].particles[j].y -= fws[i].particles[j].speed.y;
				// 受重力影响的速度变化
				fws[i].particles[j].speed.y -= gravity;
				// 绘制粒子
				setfillcolor(RGB(fws[i].R, fws[i].G, fws[i].B));
				solidcircle(int(fws[i].particles[j].x + fws[i].x), int(fws[i].particles[j].y + fws[i].y), 1);
			}
		}
	}
}

// 清除已经消失的烟花
void ClearDisappearFirework()
{
	// 创建迭代器
	vector<Firework>::iterator it;
	// 找出所有已消失的烟花
	for (it = fws.begin(); it != fws.end();)
	{
		// 若烟花绘制次数已达到最大生命周期，从烟花群中删除该烟花
		if ((*it).gap == FIREWORK_DURATION)
		{
			it = fws.erase(it);
			Firework::number--;
		}
		else
			++it;
	}
}

int main()
{
	initgraph(800, 600);
	// 开启批量绘图
	BeginBatchDraw();
	// 设置随即种子
	srand((unsigned)time(NULL));
	// 自动生成烟花的缓冲变量
	int autoCreateFirework = 0;
	while (true) {
		// 按下鼠标新增烟花
		while (MouseHit()) {
			MOUSEMSG m = GetMouseMsg();
			if (m.mkLButton) {
				AddFirework(m.x, m.y);
			}
		}
		// 按下q键退出程序
		if (_kbhit()) {
			if (_getch() == 'q') break;
		}
		// 每隔一段时间自动生成一个烟花
		if (autoCreateFirework-- == 0) {
			AddFirework(rand() % 500 + 100, rand() % 400 + 100);
			autoCreateFirework = 180;
		}
		// 移动烟花
		MoveFireworkParticle();
		// 删除消失的烟花
		ClearDisappearFirework();
		// 全屏模糊处理
		Blur();
		// 刷新视图
		FlushBatchDraw();
		HpSleep(20);
	}
	// 停止批量绘图
	EndBatchDraw();
	// 关闭绘图环境
	closegraph();
	return 0;
}
