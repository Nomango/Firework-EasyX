#include <windows.h>
#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <vector>
using namespace std;

// ÿ���̻�����������
#define PARTICLE_NUMBER 45
// ͬʱ���ڵ�����̻���
#define MAX_FIREWORK_NUMBER 6

/*
* �ٶȽṹ��
* x ��ʾ�����˶��ٶȣ�y ��ʾ�����˶��ٶ�
*/
typedef struct 
{
	float x;
	float y;
} Speed;

/*
* �̻����ӽṹ��
* x ��ʾ�����꣬y ��ʾ������
* speed ��ʾ���ӵ�ǰ�˶��ٶ�
* enable ��ʾ�����Ƿ����
*/
typedef struct 
{
	float x = 0;
	float y = 0;
	Speed speed;
	bool enable = true;
} Particle;

/*
* �̻��ṹ��
* x��y ��ʾ�̻���ը������
* particles �����˸��̻�������Ⱥ
* R��G��B ��ʾ���̻�����ɫ
* radius ��ʾ�̻�α�뾶����ֵԽ���̻��뾶Խ��
* colorChangeBuffer ��ʾ��ɫ�仯�Ļ������
* number ��¼�˵�ǰ���ڵ��̻�����
*/
typedef struct 
{
	float x;
	float y;
	vector<Particle> particles;
	BYTE R = 255;
	BYTE G = 255;
	BYTE B = 255;
	float radius = 3.0;
	int colorChangeBuffer = 0;
	static int number;
} Firework;

int Firework::number = 0;

// ȫ�ֱ�������
// �̻�Ⱥ
vector<Firework> fws;
// ����ֵ
const float gravity = 0.006f;

// ȫ��ģ������
void Blur()
{
	static DWORD* pMem = GetImageBuffer(); // ��ȡ�Դ��ַ
	for (int i = 800; i < 800 * 599; i++)
	{
		pMem[i] = RGB(
			(GetRValue(pMem[i]) + GetRValue(pMem[i - 800]) + GetRValue(pMem[i - 1]) + GetRValue(pMem[i + 1]) + GetRValue(pMem[i + 800])) / 5,
			(GetGValue(pMem[i]) + GetGValue(pMem[i - 800]) + GetGValue(pMem[i - 1]) + GetGValue(pMem[i + 1]) + GetGValue(pMem[i + 800])) / 5,
			(GetBValue(pMem[i]) + GetBValue(pMem[i - 800]) + GetBValue(pMem[i - 1]) + GetBValue(pMem[i + 1]) + GetBValue(pMem[i + 800])) / 5);
	}
}

// ��׼��ʱ����
void HpSleep(int ms)
{
	static clock_t oldclock = clock();		// ��̬��������¼��һ�� tick

	oldclock += ms * CLOCKS_PER_SEC / 1000;	// ���� tick

	if (clock() > oldclock)					// ����Ѿ���ʱ��������ʱ
		oldclock = clock();
	else
		while (clock() < oldclock)			// ��ʱ
			Sleep(1);						// �ͷ� CPU ����Ȩ������ CPU ռ����
}

// �� ��x��y�� ���괦����̻�
bool AddFirework(short x, short y)
{
	// ����ǰ�̻������ﵽ���ֵ������ false
	if (Firework::number == MAX_FIREWORK_NUMBER) return false;
	// �����̻�
	Firework fw;
	// �����̻���ը��
	fw.x = x;
	fw.y = y;
	// �����̻���ɫ
	fw.R = rand() % 146 + 110;
	fw.G = rand() % 146 + 110;
	fw.B = rand() % 146 + 110;
	// ����̻�����λ��
	for (int i = 0; i < PARTICLE_NUMBER; i++)
	{
		// ��������
		Particle p;
		// ��Բ�η�Χ����������ٶ�
		do {
			p.speed.x = (rand() % 201) / (200 / fw.radius / fw.radius) - fw.radius;
			p.speed.y = (rand() % 201) / (200 / fw.radius / fw.radius) - fw.radius;
		} while ((p.speed.x * p.speed.x + p.speed.y * p.speed.y) >= fw.radius * fw.radius);
		// ��������ӽ�����Ⱥ
		fw.particles.push_back(p);
	}
	// �̻�����+1
	Firework::number++;
	// �����̻���ӽ��̻�Ⱥ
	fws.push_back(fw);
	return true;
}

// �ƶ�ȫ���̻�����
void MoveFireworkParticle()
{
	// ��ʱ����
	int x, y;
	for (int i = 0; i < Firework::number; i++)
	{
		for (int j = 0; j < PARTICLE_NUMBER; j++)
		{
			// �����Ӳ����ڣ�����������
			if (!fws[i].particles[j].enable)
				continue;
			x = int(fws[i].particles[j].x + fws[i].x);
			y = int(fws[i].particles[j].y + fws[i].y);
			// �����ӳ��磬�����Ӵ��ڱ�־��Ϊ false
			if (x < 5 || x >(800 - 5) || y < 5 || y >(600 - 5))
			{
				fws[i].particles[j].enable = false;
				continue;
			}
			// �ƶ�����λ��
			fws[i].particles[j].x -= fws[i].particles[j].speed.x;
			fws[i].particles[j].y -= fws[i].particles[j].speed.y;
			// ������Ӱ����ٶȱ仯
			fws[i].particles[j].speed.y -= gravity;
			// ����������ɫ
			if (fws[i].colorChangeBuffer++ == 30)
			{
				fws[i].R = (fws[i].R - 1 < 0) ? 0 : fws[i].R - 1;
				fws[i].G = (fws[i].G - 1 < 0) ? 0 : fws[i].G - 1;
				fws[i].B = (fws[i].B - 1 < 0) ? 0 : fws[i].B - 1;
				fws[i].colorChangeBuffer = 0;
			}
			setfillcolor(RGB(fws[i].R, fws[i].G, fws[i].B));
			// ��������
			solidcircle(int(fws[i].particles[j].x + fws[i].x), int(fws[i].particles[j].y + fws[i].y), 2);
		}
	}
}

// ����Ѿ���ʧ���̻�
void ClearDisappearFirework()
{
	// ����������
	vector<Firework>::iterator it;
	// �ҳ���������ʧ���̻�
	for (it = fws.begin(); it != fws.end();)
	{
		// ���̻����ƴ����Ѵﵽ����������ڣ����̻�Ⱥ��ɾ�����̻�
		if ((*it).R == 0 && (*it).G == 0 && (*it).B == 0)
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
	// ����������ͼ
	BeginBatchDraw();
	// �����漴����
	srand((unsigned)time(NULL));
	// �Զ������̻��Ļ������
	int autoCreateFirework = 0;
	while (true) {
		// ������������̻�
		while (MouseHit()) {
			MOUSEMSG m = GetMouseMsg();
			if (m.mkLButton) {
				AddFirework(m.x, m.y);
			}
		}
		// ����q���˳�����
		if (_kbhit()) {
			if (_getch() == 'q') break;
		}
		// ÿ��һ��ʱ���Զ�����һ���̻�
		if (autoCreateFirework-- == 0) {
			AddFirework(rand() % 500 + 100, rand() % 400 + 100);
			autoCreateFirework = 180;
		}
		// �ƶ��̻�
		MoveFireworkParticle();
		// ɾ����ʧ���̻�
		ClearDisappearFirework();
		// ȫ��ģ������
		Blur();
		// ˢ����ͼ
		FlushBatchDraw();
		HpSleep(10);
	}
	// ֹͣ������ͼ
	EndBatchDraw();
	// �رջ�ͼ����
	closegraph();
	return 0;
}
