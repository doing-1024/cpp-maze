#include <iostream>
#include <graphics.h>
#include <queue>
#include <ctime>
#include <memory>
#include <chrono>
#include <windows.h>
#include <vector>
#include "Lencode.h"

#define timePoint std::chrono::system_clock::time_point 

const int INF = 0x7f7f7f7f;
using namespace std;

// 基础配置
int map[110][110], mapX = 13, mapY = 23, maxhp;
int Not[30] = {obc::grass, obc::soil, obc::stone, obc::dia, obc::iron};
bool game_, MAP[110][110];
PIMAGE img[50];
int guaiShu;

// 动态缩放变量
int tileW, tileH;

// 距离计算函数 (平方距离)
int getDistSq(int x1, int y1, int x2, int y2) {
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

class Skill {
public:
	virtual ~Skill() = default;
	virtual void use() {}
};

class Node {
protected:
	vector<unique_ptr<Skill>> skillBar;
public:
	int x, y; Tag t; int hp, hurt, p, q, id, k;
	Node(int x = 0, int y = 0, Tag t = obc::none, int hp = 0, int hurt = 0, int p = 0, int q = 0, int k = 0)
	: x(x), y(y), t(t), hp(hp), hurt(hurt), p(p), q(q), id(0), k(k) {}
	virtual void useSkill() { for (auto &i : skillBar) i->use(); }
};

class comzom : public Node {
private:
	class Skill1 : public ::Skill {
	private:
		int state = 0;
		timePoint last = chrono::system_clock::now();
		timePoint now = chrono::system_clock::now();
		int v; comzom *thie;
	public:
		Skill1(comzom *thisTmp) : thie(thisTmp) {}
		void use() override;
	};
public:
	comzom(int x, int y, Tag t = obc::comzom, int hp = 10, int hurt = 2, int p = 300, int q = 1000, int k = 0)
	: Node(x, y, t, hp, hurt, p, q, k) { skillBar.push_back(make_unique<Skill1>(this)); }
};

class drown : public Node {
public:
	drown(int x, int y, Tag t = obc::drown, int hp = 10, int hurt = 5, int p = 400, int q = 3000, int k = 1)
	: Node(x, y, t, hp, hurt, p, q, k) {}
};

class chickJock : public Node {
public:
	chickJock(int x, int y, Tag t = obc::chickJock, int hp = 10, int hurt = 2, int p = 200, int q = 700, int k = 0)
	: Node(x, y, t, hp, hurt, p, q, k) {}
};

void comzom::Skill1::use() {
	if (maxhp < 15) return;
	if (state == 0) { last = chrono::system_clock::now(); state = 1; thie->p = INF; }
	if (state == 1) {
		now = chrono::system_clock::now();
		auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
		if (tmp.count() >= 3000) { state = 2; last = chrono::system_clock::now(); thie->hurt = 4; thie->p = 200; thie->q = 800; }
	}
	if (state == 2) {
		now = chrono::system_clock::now();
		auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
		if (tmp.count() >= 7000) { last = chrono::system_clock::now(); thie->hurt = 2; thie->p = 300; thie->q = 1000; state = 3; v = rand() % 10000; }
	}
	if (state == 3) {
		now = chrono::system_clock::now();
		auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
		if (tmp.count() >= v) state = 0;
	}
}

vector<unique_ptr<Node>> mons;
Node player; // 玩家全局对象
vector<deque<pair<int, int>>> qu;
const int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

bool ok(int x) { for (int i = 0; i <= 4; i++) if (x == Not[i]) return false; return true; }

bool quok;
void dfsGetQu(int t, int x1, int y1, int x, int y) {
	if (x1 == x && y1 == y) { quok = 1; return; }
	for (int i = 0; i < 4; i++) {
		int X = x1 + D[i][0], Y = y1 + D[i][1];
		if (ok(map[X][Y]) && !MAP[X][Y] && X >= 0 && Y >= 0 && X <= mapX + 1 && Y <= mapY + 1) {
			MAP[X][Y] = 1; qu[t].push_back(make_pair(D[i][0], D[i][1]));
			dfsGetQu(t, X, Y, x, y);
			if (quok) break;
			qu[t].pop_back(); MAP[X][Y] = 0;
		}
	}
}

void getQu(int t, int x, int y) { quok = 0; memset(MAP, 0, sizeof(MAP)); dfsGetQu(t, mons[t]->x, mons[t]->y, x, y); }

void paint(int x, int y, int t) {
	putimage(y * tileW, x * tileH, tileW, tileH, img[t], 0, 0, getwidth(img[t]), getheight(img[t]));
}

void paintFull(int t) {
	putimage(0, 0, getwidth(), getheight(), img[t], 0, 0, getwidth(img[t]), getheight(img[t]));
}

void drawItemBar() {
	int barTileW = getwidth() / 10;
	for(int i = 0; i < 10; i++) {
		putimage(i * barTileW, (mapX + 6) * tileH, barTileW, tileH, img[23], 0, 0, getwidth(img[23]), getheight(img[23]));
	}
}

void painthart() {
	int maxPerRow = mapY + 4;
	for (int i = 0; i < min(maxPerRow, player.hp); i++) paint(mapX + 4, i, 14);
	for (int i = min(maxPerRow, player.hp); i < min(maxPerRow, maxhp); i++) paint(mapX + 4, i, 15);
	if (maxhp > maxPerRow) {
		for (int i = 0; i < max(0, player.hp - maxPerRow); i++) paint(mapX + 5, i, 14);
		for (int i = max(0, player.hp - maxPerRow); i < maxhp - maxPerRow; i++) paint(mapX + 5, i, 15);
	}
}

void pinatmap() {
	for (int i = 0; i <= mapX + 1; i++) {
		paint(i + 1, 0, 11);
		for (int j = 0; j <= mapY + 1; j++) paint(i + 1, j + 1, map[i][j]);
		paint(i + 1, mapY + 3, 11);
	}
	for (int i = 0; i <= mapY + 3; i++) {
		paint(0, i, 11); paint(mapX + 3, i, 11);
	}
	painthart();
	drawItemBar();
}

void makemap(int x, int y) {
	int c[4][2] = {0, 1, 1, 0, 0, -1, -1, 0}, j, i;
	for (i = 0; i < 4; i++) { j = rand() % 4; swap(c[i][0], c[j][0]); swap(c[i][1], c[j][1]); }
	map[x][y] = 1;
	for (i = 0; i < 4; i++)
		if (!ok(map[x + 2 * c[i][0]][y + 2 * c[i][1]])) {
			map[x + c[i][0]][y + c[i][1]] = 1; makemap(x + 2 * c[i][0], y + 2 * c[i][1]);
		}
}

int Get(int x, int y) { if (x == -1 && y == 0) return 1; if (x == 1 && y == 0) return 2; if (x == 0 && y == -1) return 3; return 4; }

bool getok(int x1, int y1, int x2, int y2) {
	int x = Get(x1, y1), y = Get(x2, y2);
	return (y - x == 1 && y % 2 == 0) || (y - x == -1 && x % 2 == 0);
}

void goit(int x, int y, Node& t) {
	int newx = t.x + x, newy = t.y + y;
	if (newx <= mapX + 1 && newx >= 0 && newy <= mapY + 1 && newy >= 0 && ok(map[newx][newy])) {
		t.x = newx; t.y = newy;
		if (t.t == 5) {
			for (int i = 0; i < (int)mons.size(); i++) {
				qu[i].push_back(make_pair(x, y));
				while (qu[i].size() >= 2){
					pair<int,int> a = qu[i].back(); qu[i].pop_back();
					pair<int,int> b = qu[i].back();
					if (getok(a.first, a.second, b.first, b.second)) qu[i].pop_back();
					else { qu[i].push_back(a); break; }
				}
			}
		}
	}
}

int Rand(vector<int> x) {
	int n = 0; for (int i : x) n += i;
	for (int i = 1; i < (int)x.size(); i++) x[i] += x[i - 1];
	int y = rand() % n;
	for (int i = 0; i < (int)x.size(); i++) if (y < x[i]) return i;
	return 0;
}

pair<int, int> getxy() {
	pair<int, int> xy; xy.first = rand() % (mapX + 1); xy.second = rand() % (mapY + 1);
	while (!ok(map[xy.first][xy.second])) { xy.first = rand() % (mapX + 1); xy.second = rand() % (mapY + 1); }
	return xy;
}

int STATE = 0, maxType = 0, resultImgTag = 0;
vector<timePoint> lastTime, lasthurt;

void initGame() {
	cout << "游戏开始\n";
	if (maxhp > (mapY + 4) * 2) maxhp = (mapY + 4) * 2;
	srand(time(0)); mons.clear(); qu.clear();
	
	// PR 逻辑初始化玩家: hp=maxhp, hurt=1, p=0, q=0, k=2 (给玩家2格攻击距离)
	player = {0, 0, obc::player, maxhp, 1, 0, 0, 2}; 
	
	for (int i = 0; i <= mapX + 1; i++)
		for (int j = 0; j <= mapY + 1; j++)
			map[i][j] = Not[Rand({40, 30, 15, 1, 4})];
	makemap(2 * (rand() % (mapX / 2) + 1), 2 * (rand() % (mapY / 2) + 1));
	cout << "地图创建完成\n";
	
	maxType = 0;
	for (int i = 0; i < guaiShu; i++) {
		int type = Rand({20, 50, 30});
		pair<int, int> xy = getxy();
		if (type == 0) mons.push_back(make_unique<comzom>(xy.first, xy.second));
		else if (type == 1) mons.push_back(make_unique<drown>(xy.first, xy.second));
		else mons.push_back(make_unique<chickJock>(xy.first, xy.second));
		
		mons.back()->id = i; 
		qu.push_back(deque<pair<int, int>>()); 
		getQu(i, 0, 0); 
		maxType = max(maxType, type);
	}
	cout << "怪物生成完成. 本局怪数: " << guaiShu << endl;
	cout << "怪物血量监控: \n";
	
	lastTime.assign(guaiShu, chrono::system_clock::now());
	lasthurt.assign(guaiShu, chrono::system_clock::now());
}

void do_menu() {
	paintFull(obc::zhudating);
	putimage(300, 300, img[obc::start]);
	while (mousemsg()) {
		mouse_msg msg = getmouse();
		if (msg.msg == mouse_msg_down && msg.is_left())
			if (msg.x >= 300 && msg.y >= 300 && msg.x <= 618 && msg.y <= 462) {
				guaiShu = Rand({0, 60, 20, 15, 5}); initGame(); STATE = 1;
			}
	}
}

void do_game() {
	// 1. 处理鼠标点击攻击 (PR 逻辑)
	while(mousemsg()){
		mouse_msg x = getmouse();
		if(x.msg == mouse_msg_down && x.is_left()){
			// 将鼠标坐标转换为网格坐标 (考虑 tileW/tileH 缩放和偏移)
			int gridY = x.x / tileW - 1;
			int gridX = x.y / tileH - 1;
			for(int i = 0; i < (int)mons.size(); i++){
				if(mons[i]->hp > 0 && mons[i]->x == gridX && mons[i]->y == gridY){
					if(getDistSq(player.x, player.y, mons[i]->x, mons[i]->y) <= player.k * player.k){
						mons[i]->hp -= player.hurt;
						cout << i << " : " << mons[i]->hp << endl;
						if(mons[i]->hp <= 0) cout << i << " is dead\n";
					}
				}
			}
		}
	}

	// 2. 怪物技能使用
	for (auto& i : mons) if(i->hp > 0) i->useSkill();

	// 3. 玩家键盘移动
	if (GetAsyncKeyState(0x57) & 0x0001) goit(-1, 0, player);
	if (GetAsyncKeyState(0x53) & 0x0001) goit(1, 0, player);
	if (GetAsyncKeyState(0x41) & 0x0001) goit(0, -1, player);
	if (GetAsyncKeyState(0x44) & 0x0001) goit(0, 1, player);

	// 4. 怪物 AI 和 攻击判定 (PR 逻辑：使用平方距离判定攻击)
	auto now = chrono::system_clock::now();
	for (int i = 0; i < (int)mons.size(); i++) {
		if(mons[i]->hp <= 0) continue;

		// 移动逻辑
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastTime[i]);
		if (elapsed.count() >= mons[i]->p && !qu[i].empty()) {
			pair<int,int> a = qu[i].front(); qu[i].pop_front();
			goit(a.first, a.second, *mons[i]); lastTime[i] = now;
		}

		// 攻击判定逻辑
		auto Elapsed = chrono::duration_cast<chrono::milliseconds>(now - lasthurt[i]);
		if (Elapsed.count() >= mons[i]->q && getDistSq(player.x, player.y, mons[i]->x, mons[i]->y) <= mons[i]->k * mons[i]->k) {
			player.hp -= mons[i]->hurt; 
			lasthurt[i] = now;
		}
	}
    
	// 5. 渲染
	paintFull(1); 
	pinatmap();
	for (auto& m : mons) if(m->hp > 0) paint(m->x + 1, m->y + 1, m->t);
	paint(player.x + 1, player.y + 1, player.t);
	paint(mapX + 2, mapY + 2, obc::port);

	// 6. 胜利/失败判定
	if (player.x == mapX + 1 && player.y == mapY + 1) { 
		maxhp += guaiShu * maxType; resultImgTag = obc::win; STATE = 2; 
	} else if (player.hp <= 0) { 
		maxhp--; resultImgTag = obc::loss; STATE = 2; 
	}
}

void do_result() {
	paintFull(resultImgTag);
	if (kbhit()) {
		int x = getch();
		if (x == ' ') STATE = 0;
	}
}

int main() {
	ShowWindow(GetForegroundWindow(), SW_MAXIMIZE);
	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	initgraph(sw, sh, INIT_NOFORCEEXIT);
	setrendermode(RENDER_MANUAL);

	tileW = sw / (mapY + 4);
	tileH = sh / (mapX + 7);

	string lujing[100] {
		"beijing.jpg", "caofangkuai.jpg",  "nitu.jpg", "chuansongmenfangkuai.jpg",
		"wanjia.jpg", "shenglitupian.jpg", "yuanshi.jpg", "zuanshikuang.jpg",
		"tiekuang.jpg", "jiangshi.jpg",    "jiyan.jpg", "kongbai.jpg",
		"shibai.jpg", "xin.jpg", "kongxin.jpg", "gong.jpg", "jian.jpg",
		"jiqishi.jpg", "nishi.jpg", "yujie.jpg", "zhudating.jpg",
		"maoxianmoshi.jpg", "wupinlan.jpg"
	};

	for (int i = 0; i < 23; i++) {
		img[i + 1] = newimage();
		string _ = "./caizhibao/" + lujing[i];
		getimage(img[i + 1], _.c_str());
	}

	maxhp = 60;
	while (is_run()) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;
		
		cleardevice();
		if (STATE == 0) do_menu();
		else if (STATE == 1) do_game();
		else if (STATE == 2) do_result();
		
		delay_fps(50);
	}

	for (int i = 1; i <= 23; i++) delimage(img[i]);
	closegraph();
	cout << "资源释放完毕";
	return 0;
}
