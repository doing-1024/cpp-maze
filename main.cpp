#include <iostream>
#include <graphics.h>
#include <queue>
#include <ctime>
#include <memory>
#include <chrono>
#include <windows.h>
#include <vector>
#include <fstream>
#include <map>
#include <cmath>
#include "Lencode.h"
#define timePoint std::chrono::system_clock::time_point

const int INF = 0x7f7f7f7f;
const bool guaiok = 1;
using namespace std;

int Map[110][110];
const int mapTotalX = 50;
const int mapTotalY = 50;
int mapX = mapTotalX, mapY = mapTotalY;
const int viewH = 13; 
const int viewW = 23;
int camX = 0, camY = 0;

int maxhp;
int Not[30] = {obc::grass, obc::soil, obc::stone, obc::dia, obc::iron};
bool game_, MAP[110][110];
PIMAGE img[50];
int guaiShu;
pair<int, int> Map2[110][110], Not2[30] = {{3, 0}, {3, 0}, {5, 1}, {10, 3}, {5, 2}};
int tileW, tileH;

int wupinlan[10], wupinlanCnt[10], At;
const int MAX_STACK = 99;

queue<int> diaoluo[110][110];

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
	int x, y;
	Tag t;
	int hp, maxhp, hurt, p, q, id, k, wa;
	Node(int x = 0, int y = 0, Tag t = obc::none, int hp = 0, int hurt = 0, int p = 0, int q = 0, int k = 0, int wa = 0)
	: x(x), y(y), t(t), hp(hp), maxhp(hp), hurt(hurt), p(p), q(q), id(0), k(k), wa(wa) {}
	virtual void useSkill() {
		for (auto &i : skillBar) i->use();
	}
};

class comzom : public Node {
private:
	class Skill1 : public ::Skill {
	private:
		int state = 0;
		timePoint last = chrono::system_clock::now();
		timePoint now = chrono::system_clock::now();
		int v;
		comzom *thie;
	public:
		Skill1(comzom *thisTmp) : thie(thisTmp) {}
		void use() override;
	};
public:
	comzom(int x, int y, Tag t = obc::comzom, int hp = 10, int hurt = 2, int p = 300, int q = 1000, int k = 0)
	: Node(x, y, t, hp, hurt, p, q, k) {
		skillBar.push_back(make_unique<Skill1>(this));
	}
};

class drown : public Node {
public:
	drown(int x, int y, Tag t = obc::drown, int hp = 7, int hurt = 5, int p = 400, int q = 3000, int k = 1)
	: Node(x, y, t, hp, hurt, p, q, k) {}
};

class chickJock : public Node {
public:
	chickJock(int x, int y, Tag t = obc::chickJock, int hp = 5, int hurt = 2, int p = 200, int q = 700, int k = 0)
	: Node(x, y, t, hp, hurt, p, q, k) {}
};

class zhizhu : public Node {
public:
	zhizhu(int x, int y, Tag t = obc::zhizhu, int hp = 8, int hurt = 4, int p = 300, int q = 1000, int k = 1)
	: Node(x, y, t, hp, hurt, p, q, k) {}
};

void comzom::Skill1::use() {
	if (::maxhp < 15) return;
	if (state == 0) {
		last = chrono::system_clock::now();
		state = 1;
		thie->p = INF;
	}
	if (state == 1) {
		now = chrono::system_clock::now();
		auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
		if (tmp.count() >= 3000) {
			state = 2;
			last = chrono::system_clock::now();
			thie->hurt = 4;
			thie->p = 200;
			thie->q = 800;
		}
	}
	if (state == 2) {
		now = chrono::system_clock::now();
		auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
		if (tmp.count() >= 7000) {
			last = chrono::system_clock::now();
			thie->hurt = 2;
			thie->p = 300;
			thie->q = 1000;
			state = 3;
			v = rand() % 10000;
		}
	}
	if (state == 3) {
		now = chrono::system_clock::now();
		auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
		if (tmp.count() >= v) state = 0;
	}
}

class nodeWP {
public:
	Tag t;
	int hp, hurt, wa;
	nodeWP(Tag t = obc::none, int hp = 0, int hurt = 0, int wa = 0)
	: t(t), hp(hp), hurt(hurt), wa(wa) {}
};
map<int, nodeWP> wupin, wupindiaoluo;
vector<unique_ptr<Node>> mons;
Node player;
vector<deque<pair<int, int>>> qu;
const int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

float dropItemAngle[110][110] = {0};

bool ok(int x) {
  if (x == obc::bedrock) return false;
	for (int i = 0; i <= 4; i++) if (x == Not[i]) return false;
	return true;
}

bool addToInventory(int itemId) {
	if (itemId == 0) return false;
	for (int i = 0; i < 10; i++) {
		if (wupinlan[i] == itemId && wupinlanCnt[i] < MAX_STACK) {
			wupinlanCnt[i]++;
			return true;
		}
	}
	for (int i = 0; i < 10; i++) {
		if (wupinlan[i] == 0) {
			wupinlan[i] = itemId;
			wupinlanCnt[i] = 1;
			return true;
		}
	}
	return false;
}

bool bfsCanReach(int sx, int sy, int tx, int ty) {
  if (!ok(Map[tx][ty])) return false;
  bool visited[110][110] = {0};
  queue<pair<int, int>> q;
  q.push({sx, sy});
  visited[sx][sy] = true;
  while (!q.empty()) {
    auto [x, y] = q.front();
    q.pop();
    if (x == tx && y == ty) return true;
    for (int i = 0; i < 4; i++) {
      int nx = x + D[i][0], ny = y + D[i][1];
      if (nx >= 0 && ny >= 0 && nx <= mapX + 1 && ny <= mapY + 1 && 
          !visited[nx][ny] && ok(Map[nx][ny])) {
        visited[nx][ny] = true;
        q.push({nx, ny});
      }
    }
  }
  return false;
}

deque<pair<int, int>> bfsFindPath(int sx, int sy, int tx, int ty) {
  deque<pair<int, int>> path;
  if (!bfsCanReach(sx, sy, tx, ty)) return path;
  bool visited[110][110] = {0};
  pair<int, int> parent[110][110];
  queue<pair<int, int>> q;
  q.push({sx, sy});
  visited[sx][sy] = true;
  while (!q.empty()) {
    auto [x, y] = q.front();
    q.pop();
    if (x == tx && y == ty) break;
    for (int i = 0; i < 4; i++) {
      int nx = x + D[i][0], ny = y + D[i][1];
      if (nx >= 0 && ny >= 0 && nx <= mapX + 1 && ny <= mapY + 1 && 
          !visited[nx][ny] && ok(Map[nx][ny])) {
        visited[nx][ny] = true;
        parent[nx][ny] = {x, y};
        q.push({nx, ny});
      }
    }
  }
  int cx = tx, cy = ty;
  while (cx != sx || cy != sy) {
    auto [px, py] = parent[cx][cy];
    path.push_front({cx - px, cy - py});
    cx = px;
    cy = py;
  }
  return path;
}

int navDist[110][110];
bool navDirty = true;

void rebuildNavDist() {
	for (int i = 0; i <= mapX + 1; i++)
		for (int j = 0; j <= mapY + 1; j++)
			navDist[i][j] = INF;
	if (player.x < 0 || player.y < 0 || player.x > mapX + 1 || player.y > mapY + 1) return;
	if (!ok(Map[player.x][player.y])) return;

	queue<pair<int, int>> q;
	navDist[player.x][player.y] = 0;
	q.push({player.x, player.y});
	while (!q.empty()) {
		auto [x, y] = q.front();
		q.pop();
		for (int i = 0; i < 4; i++) {
			int nx = x + D[i][0], ny = y + D[i][1];
			if (nx < 0 || ny < 0 || nx > mapX + 1 || ny > mapY + 1) continue;
			if (!ok(Map[nx][ny])) continue;
			if (navDist[nx][ny] != INF) continue;
			navDist[nx][ny] = navDist[x][y] + 1;
			q.push({nx, ny});
		}
	}
}

bool heartWaveActive = false;
timePoint lastHurtTime;
const int HEART_WAVE_DURATION = 800;
const double HEART_WAVE_AMPLITUDE = 3.0;
const double HEART_WAVE_FREQUENCY = 2.0;

struct MiniMapLayout {
	int bgX = 0, bgY = 0, bgW = 0, bgH = 0;
	int mapX = 0, mapY = 0, mapW = 0, mapH = 0;
	int rows = 0, cols = 0;
	int cell = 0;
};
bool minimapEnabled = true;
MiniMapLayout minimapLayout;
PIMAGE minimapTerrainCache = NULL;
int minimapTerrainCacheW = 0;
int minimapTerrainCacheH = 0;
bool minimapTerrainCacheDirty = true;

color_t minimapTileColor(int t);

MiniMapLayout calcMiniMapLayout() {
	MiniMapLayout l;
	if (!minimapEnabled) return l;
	l.rows = mapX + 2;
	l.cols = mapY + 2;
	int maxDim = l.rows > l.cols ? l.rows : l.cols;

	int target = getwidth() / 4;
	int t2 = getheight() / 4;
	if (t2 < target) target = t2;
	if (target > 320) target = 320;

	l.cell = target / maxDim;
	if (l.cell < 2) l.cell = 2;
	if (l.cell > 10) l.cell = 10;

	l.mapW = l.cols * l.cell;
	l.mapH = l.rows * l.cell;

	int pad = l.cell;
	if (pad < 4) pad = 4;
	const int headerH = 18;
	const int margin = 10;

	l.bgW = l.mapW + pad * 2;
	l.bgH = l.mapH + headerH + pad * 2;
	l.bgX = getwidth() - l.bgW - margin;
	l.bgY = margin;
	if (l.bgX < margin) l.bgX = margin;
	if (l.bgY < margin) l.bgY = margin;

	l.mapX = l.bgX + pad;
	l.mapY = l.bgY + pad + headerH;
	return l;
}

void ensureMiniMapTerrainCache() {
	if (!minimapEnabled) return;
	if (minimapLayout.cell <= 0 || minimapLayout.mapW <= 0 || minimapLayout.mapH <= 0) return;
	if (!minimapTerrainCache || minimapTerrainCacheW != minimapLayout.mapW || minimapTerrainCacheH != minimapLayout.mapH) {
		if (minimapTerrainCache) delimage(minimapTerrainCache);
		minimapTerrainCache = newimage(minimapLayout.mapW, minimapLayout.mapH);
		minimapTerrainCacheW = minimapLayout.mapW;
		minimapTerrainCacheH = minimapLayout.mapH;
		minimapTerrainCacheDirty = true;
	}
	if (!minimapTerrainCacheDirty) return;

	PIMAGE old = gettarget();
	settarget(minimapTerrainCache);
	cleardevice();
	for (int i = 0; i < minimapLayout.rows; i++) {
		for (int j = 0; j < minimapLayout.cols; j++) {
			int t = abs(Map[i][j]);
			setfillcolor(minimapTileColor(t));
			ege_fillrect(j * minimapLayout.cell, i * minimapLayout.cell, minimapLayout.cell, minimapLayout.cell);
		}
	}
	settarget(old);
	minimapTerrainCacheDirty = false;
}

bool pointInMiniMap(int px, int py) {
	if (!minimapEnabled) return false;
	if (minimapLayout.bgW <= 0 || minimapLayout.bgH <= 0) return false;
	return px >= minimapLayout.bgX && px < minimapLayout.bgX + minimapLayout.bgW &&
	       py >= minimapLayout.bgY && py < minimapLayout.bgY + minimapLayout.bgH;
}

color_t minimapTileColor(int t) {
	switch (t) {
		case obc::bedrock: return EGEGRAY(20);
		case obc::grass: return EGERGB(60, 160, 60);
		case obc::soil: return EGERGB(140, 90, 40);
		case obc::stone: return EGEGRAY(110);
		case obc::dia: return EGERGB(60, 220, 220);
		case obc::iron: return EGERGB(200, 140, 70);
		default:
			return ok(t) ? EGEGRAY(210) : EGEGRAY(70);
	}
}

void drawMiniMapMarker(int x, int y, color_t col) {
	if (!minimapEnabled || minimapLayout.cell <= 0) return;
	if (x < 0 || y < 0 || x >= minimapLayout.rows || y >= minimapLayout.cols) return;
	int px = minimapLayout.mapX + y * minimapLayout.cell;
	int py = minimapLayout.mapY + x * minimapLayout.cell;
	setfillcolor(col);
	ege_fillrect(px, py, minimapLayout.cell, minimapLayout.cell);
}

void drawMiniMap() {
	if (!minimapEnabled) return;
	if (minimapLayout.cell <= 0) return;
	ensureMiniMapTerrainCache();

	setfillcolor(EGERGBA(0, 0, 0, 180));
	ege_fillrect(minimapLayout.bgX, minimapLayout.bgY, minimapLayout.bgW, minimapLayout.bgH);
	setlinecolor(EGEGRAY(230));
	ege_rectangle(minimapLayout.bgX, minimapLayout.bgY, minimapLayout.bgW, minimapLayout.bgH);

	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	setfont(16, 0, "Consolas");
	outtextxy(minimapLayout.bgX + 6, minimapLayout.bgY + 2, "MiniMap (M)");

	if (minimapTerrainCache) putimage(minimapLayout.mapX, minimapLayout.mapY, minimapTerrainCache);

	// 视野范围框
	int viewRows = viewH + 3;
	int viewCols = viewW + 3;
	int vx = camY, vy = camX;
	if (vx < 0) vx = 0;
	if (vy < 0) vy = 0;
	if (vx > minimapLayout.cols) vx = minimapLayout.cols;
	if (vy > minimapLayout.rows) vy = minimapLayout.rows;
	if (vx + viewCols > minimapLayout.cols) viewCols = minimapLayout.cols - vx;
	if (vy + viewRows > minimapLayout.rows) viewRows = minimapLayout.rows - vy;
	if (viewCols > 0 && viewRows > 0) {
		setlinecolor(EGERGB(255, 220, 40));
		ege_rectangle(minimapLayout.mapX + vx * minimapLayout.cell,
		              minimapLayout.mapY + vy * minimapLayout.cell,
		              viewCols * minimapLayout.cell, viewRows * minimapLayout.cell);
	}

	// 出口/怪物/玩家
	drawMiniMapMarker(mapX, mapY, EGERGB(200, 60, 255)); // exit
	for (auto& m : mons) if (m->hp > 0) drawMiniMapMarker(m->x, m->y, EGERGB(255, 70, 70));
	drawMiniMapMarker(player.x, player.y, EGERGB(60, 150, 255));
}

bool quok;
void dfsGetQu(int t, int x1, int y1, int x, int y) {
	if (x1 == x && y1 == y) {
		quok = 1;
		return;
	}
	for (int i = 0; i < 4; i++) {
		int X = x1 + D[i][0], Y = y1 + D[i][1];
		if (ok(Map[X][Y]) && !MAP[X][Y] && X >= 0 && Y >= 0 && X <= mapX + 1 && Y <= mapY + 1) {
			MAP[X][Y] = 1;
			qu[t].push_back(make_pair(D[i][0], D[i][1]));
			dfsGetQu(t, X, Y, x, y);
			if (quok) break;
			qu[t].pop_back();
			MAP[X][Y] = 0;
		}
	}
}

void getQu(int t, int x, int y) {
	quok = 0;
	memset(MAP, 0, sizeof(MAP));
	dfsGetQu(t, mons[t]->x, mons[t]->y, x, y);
}

void updateCamera() {
  camX = player.x - viewH / 2;
  camY = player.y - viewW / 2;
  if (camX < 0) camX = 0;
  if (camY < 0) camY = 0;
  if (camX > mapX + 1 - viewH) camX = mapX + 1 - viewH;
  if (camY > mapY + 1 - viewW) camY = mapY + 1 - viewW;
}

void paint(int x, int y, int t, int offsetY = 0) {
	int drawX = x - camX;
	int drawY = y - camY;
	
	if (drawX >= 0 && drawX <= viewH + 2 && drawY >= -1 && drawY <= viewW + 2) {
		putimage(drawY * tileW, drawX * tileH + offsetY, tileW, tileH, img[t], 0, 0, getwidth(img[t]), getheight(img[t]));
	}
}

void paintFull(int t) {
	putimage(0, 0, getwidth(), getheight(), img[t], 0, 0, getwidth(img[t]), getheight(img[t]));
}

void drawHealthBar(const Node& m) {
	if (m.hp <= 0 || m.maxhp <= 0) return;
	int drawX = m.x - camX;
	int drawY = m.y - camY;
	if (drawX < 0 || drawX > viewH + 2 || drawY < 0 || drawY > viewW + 2) return;

	int barW = (int)(tileW * 0.8);
	int barH = max(4, (int)(tileH * 0.12));
	int screenX = drawY * tileW + (tileW - barW) / 2;
	int screenY = drawX * tileH - barH - 2;
	if (screenY < 0) screenY = drawX * tileH + 2;

	float pct = (float)m.hp / (float)m.maxhp;
	if (pct < 0.0f) pct = 0.0f;
	if (pct > 1.0f) pct = 1.0f;

	setfillcolor(DARKRED);
	ege_fillrect(screenX, screenY, barW, barH);
	setfillcolor(GREEN);
	ege_fillrect(screenX, screenY, (int)(barW * pct), barH);
	setlinecolor(BLACK);
	ege_rectangle(screenX, screenY, barW, barH);
}

void painthart() {
	int row = viewH + 2;
	int maxPerRow = viewW + 1;
	
	auto now = chrono::system_clock::now();
	if (heartWaveActive) {
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastHurtTime);
		if (elapsed.count() >= HEART_WAVE_DURATION) {
			heartWaveActive = false;
		}
	}
	
	for (int i = 0; i < min(maxPerRow, player.hp); i++) {
		int offsetY = 0;
		if (heartWaveActive) {
			auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastHurtTime);
			double phase = (elapsed.count() * 0.001 * HEART_WAVE_FREQUENCY) - (i * 0.3);
			offsetY = (int)(sin(phase * 3.14159 * 2) * HEART_WAVE_AMPLITUDE);
		}
    putimage(i * tileW, row * tileH + offsetY, tileW, tileH, img[obc::heart], 0, 0, getwidth(img[obc::heart]), getheight(img[obc::heart]));
	}
	for (int i = min(maxPerRow, player.hp); i < min(maxPerRow, maxhp); i++) {
		int offsetY = 0;
		if (heartWaveActive) {
			auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastHurtTime);
			double phase = (elapsed.count() * 0.001 * HEART_WAVE_FREQUENCY) - (i * 0.3);
			offsetY = (int)(sin(phase * 3.14159 * 2) * HEART_WAVE_AMPLITUDE);
		}
		putimage(i * tileW, row * tileH + offsetY, tileW, tileH, img[obc::vheart], 0, 0, getwidth(img[obc::vheart]), getheight(img[obc::vheart]));
	}
}

void drawItemBar() {
	int row = viewH + 3;
	int itemH = tileH * 1.2;
	int itemW = itemH * 1.2;
	int totalW = itemW * 10;
	int startX = (getwidth() - totalW) / 2;
	int startY = row * tileH;

	for (int i = 0; i < 10; i++) {
		if (i == At)
			putimage(startX + i * itemW, startY, itemW, itemH, img[obc::wplat], 0, 0, getwidth(img[obc::wplat]), getheight(img[obc::wplat]));
		else
			putimage(startX + i * itemW, startY, itemW, itemH, img[obc::wupinlan], 0, 0, getwidth(img[obc::wupinlan]), getheight(img[obc::wupinlan]));
		
		if (wupinlan[i]) {
      int iconSize = (int)(itemH * 0.7);
      int paddingX = (itemW - iconSize) / 2;
      int paddingY = (itemH - iconSize) / 2;
			putimage(startX + i * itemW + paddingX, startY + paddingY, iconSize, iconSize, 
               img[wupin[wupinlan[i]].t], 0, 0, getwidth(img[wupin[wupinlan[i]].t]), getheight(img[wupin[wupinlan[i]].t]));
			if (wupinlanCnt[i] > 1) {
				int fontH = max(10, (int)(itemH * 0.28));
				setfont(fontH, 0, "Consolas");
				setbkmode(TRANSPARENT);
				settextcolor(WHITE);
				string cnt = to_string(wupinlanCnt[i]);
				int tx = startX + i * itemW + itemW - textwidth(cnt.c_str()) - 4;
				int ty = startY + itemH - textheight(cnt.c_str()) - 2;
				outtextxy(tx, ty, cnt.c_str());
			}
    }
	}
}

void pinatmap() {
  int startI = camX;
  int endI = camX + viewH + 2; 
  int startJ = camY;
  int endJ = camY + viewW + 2;

  for (int i = startI; i <= endI; i++) {
    for (int j = startJ; j <= endJ; j++) {
      if (i < 0 || i > mapX + 1 || j < 0 || j > mapY + 1) continue;
      paint(i, j, abs(Map[i][j]));
    }
  }

	const int dropBaseW = (int)(tileW * 0.7f);
	const int dropBaseH = (int)(tileH * 0.7f);
	for (int i = startI; i <= endI; i++)
		for (int j = startJ; j <= endJ; j++) {
			if (i >= 0 && i <= mapX && j >= 0 && j <= mapY && !diaoluo[i][j].empty()) {
				int x = i, y = j, t = wupindiaoluo[diaoluo[i][j].front()].t;
				dropItemAngle[i][j] += 0.08f;
				if (dropItemAngle[i][j] > 6.28318f) dropItemAngle[i][j] -= 6.28318f;
				
				float angle = dropItemAngle[i][j];
				float scale = fabsf(cosf(angle));
				int dstW = (int)(dropBaseW * scale);
				if (dstW < 1) dstW = 1;
				int offsetX = (dropBaseW - dstW) / 2;
				
        int drawX = x - camX;
        int drawY = y - camY;
        if(drawX >= 0 && drawX <= viewH + 2 && drawY >= 0 && drawY <= viewW + 2) {
					putimage(drawY * tileW + 4 + offsetX, drawX * tileH + 5,
					         dstW, dropBaseH, img[t], 0, 0, getwidth(img[t]), getheight(img[t]));
				}
			}
		}
	painthart();
	drawItemBar();
}

void makemap(int x, int y) {
	int c[4][2] = {0, 1, 1, 0, 0, -1, -1, 0}, j, i;
	for (i = 0; i < 4; i++) {
		j = rand() % 4;
		swap(c[i][0], c[j][0]);
		swap(c[i][1], c[j][1]);
	}
	Map[x][y] = 1;
	for (i = 0; i < 4; i++)
		if (x + 2 * c[i][0] >= 1 && x + 2 * c[i][0] <= mapX && 
		    y + 2 * c[i][1] >= 1 && y + 2 * c[i][1] <= mapY &&
		    !ok(Map[x + 2 * c[i][0]][y + 2 * c[i][1]])) {
			Map[x + c[i][0]][y + c[i][1]] = 1;
			makemap(x + 2 * c[i][0], y + 2 * c[i][1]);
		}
}

int GetDirID(int x, int y) {
	if (x == -1 && y == 0) return 1;
	if (x == 1 && y == 0) return 2;
	if (x == 0 && y == -1) return 3;
	return 4;
}

bool getok(int x1, int y1, int x2, int y2) {
	int x = GetDirID(x1, y1), y = GetDirID(x2, y2);
	return (y - x == 1 && y % 2 == 0) || (y - x == -1 && x % 2 == 0);
}

bool goit(int x, int y, Node& t) {
	int newx = t.x + x, newy = t.y + y;
	if (newx <= mapX + 1 && newx >= 0 && newy <= mapY + 1 && newy >= 0 && ok(Map[newx][newy])) {
		t.x = newx;
		t.y = newy;
		if (t.t == obc::player) {
			for (int i = 0; i < (int)mons.size(); i++) {
				qu[i].push_back(make_pair(x, y));
				while (qu[i].size() >= 2) {
					pair<int, int> a = qu[i].back();
					qu[i].pop_back();
					pair<int, int> b = qu[i].back();
					if (getok(a.first, a.second, b.first, b.second)) qu[i].pop_back();
					else {
						qu[i].push_back(a);
						break;
					}
				}
			}
			while (!diaoluo[t.x][t.y].empty()) {
				int itemId = diaoluo[t.x][t.y].front();
				if (addToInventory(itemId)) {
					diaoluo[t.x][t.y].pop();
				} else {
					break;
				}
			}
		}
		return 1;
	}
	return 0;
}

void randomWalk(Node& m) {
  vector<pair<int, int>> validMoves;
  for (int i = 0; i < 4; i++) {
    int nx = m.x + D[i][0], ny = m.y + D[i][1];
    if (nx >= 0 && ny >= 0 && nx <= mapX + 1 && ny <= mapY + 1 && ok(Map[nx][ny])) {
      validMoves.push_back({D[i][0], D[i][1]});
    }
  }
  if (!validMoves.empty()) {
    int idx = rand() % validMoves.size();
    goit(validMoves[idx].first, validMoves[idx].second, m);
  }
}

int Rand(vector<int> x) {
	int n = 0;
	for (int i : x) n += i;
	for (int i = 1; i < (int)x.size(); i++) x[i] += x[i - 1];
	int y = rand() % n;
	for (int i = 0; i < (int)x.size(); i++) if (y < x[i]) return i;
	return 0;
}

pair<int, int> getxy() {
	pair<int, int> xy;
	xy.first = rand() % (mapX + 1);
	xy.second = rand() % (mapY + 1);
	while (!ok(Map[xy.first][xy.second])) {
		xy.first = rand() % (mapX + 1);
		xy.second = rand() % (mapY + 1);
	}
	return xy;
}

int STATE = 0, resultImgTag = 0, wax, way;
vector<timePoint> lastTime, lasthurt;
timePoint lasthp, watime;
bool downok, leftok;
int gridX, gridY;

void initGame() {
	srand(time(0));
	mons.clear();
	qu.clear();
	wax = way = downok = leftok = 0;
	player = {1, 1, obc::player, maxhp, 1, 0, 0, 1, 0}; 

  // 先初始化全部为杂乱方块
	for (int i = 0; i <= mapX + 1; i++)
		for (int j = 0; j <= mapY + 1; j++) {
			int p = Rand({40, 30, 15, 1, 4});
			Map[i][j] = Not[p];
			Map2[i][j] = Not2[p];
			while (!diaoluo[i][j].empty())   diaoluo[i][j].pop();
			dropItemAngle[i][j] = 0;
		}

  // 设置基岩边界，防止makemap越界，同时作为物理墙壁
  for(int i=0; i<=mapX+1; i++) { Map[i][0] = obc::bedrock; Map[i][mapY+1] = obc::bedrock; }
  for(int j=0; j<=mapY+1; j++) { Map[0][j] = obc::bedrock; Map[mapX+1][j] = obc::bedrock; }

  // 修复1：强制从(1,1)开始生成迷宫，确保玩家出生点为空且连通
	makemap(1, 1);
  Map[1][1] = 1; // 双重保险，确保出生点是路

  // 修复2：传送门放在墙内 (mapX, mapY)
  Map[mapX][mapY] = 1; // 终点必须是路
  // 确保终点连通，防止迷宫算法因奇偶性问题产生死角
  Map[mapX-1][mapY] = 1; 
  Map[mapX][mapY-1] = 1;

	for (int i = 0; i < guaiShu; i++) {
		int type = Rand({20, 50, 30, 30});
		pair<int, int> xy = getxy();
		if (type == 0) mons.push_back(make_unique<comzom>(xy.first, xy.second));
		else if (type == 1) mons.push_back(make_unique<drown>(xy.first, xy.second));
		else if (type == 2)  mons.push_back(make_unique<chickJock>(xy.first, xy.second));
		else  mons.push_back(make_unique<zhizhu>(xy.first, xy.second));
		mons.back()->id = i;
		qu.push_back(deque<pair<int, int>>());
		getQu(i, 0, 0);
	}
	lastTime.assign(guaiShu, chrono::system_clock::now());
	lasthurt.assign(guaiShu, chrono::system_clock::now());
	watime = lasthp = lastHurtTime = chrono::system_clock::now();
	navDirty = true;
	minimapTerrainCacheDirty = true;
}

void do_menu() {
	paintFull(obc::zhudating);
	putimage(100, 185, img[obc::start]);
	putimage(550, 185, img[obc::cz]);
	while (mousemsg()) {
		mouse_msg msg = getmouse();
		if (msg.msg == mouse_msg_down && msg.is_left())
			if (msg.x >= 100 && msg.y >= 185 && msg.x <= 418 && msg.y <= 347) {
				if (guaiok)
					guaiShu = Rand({0, 30, 30, 40, 10, 10});
				initGame();
				STATE = 1;
			} else if (msg.x >= 550 && msg.y >= 185 && msg.x <= 868 && msg.y <= 347) {
				STATE = 0;
				ofstream Cout("C:\\cundang\\1.txt");
				Cout << 6 << endl;
				Cout << 111 << ' ' << 112 << ' ' << 121 << ' ' << 200 + obc::dia << ' ';
				for (int i = 1; i <= 6; i++) Cout << 0 << ' ';
				Cout << endl;
				Cout << 1 << ' ' << 1 << ' ' << 1 << ' ' << 1 << ' ';
				for (int i = 1; i <= 6; i++) Cout << 0 << ' ';
				Cout << endl;
				Cout.close();
				ifstream Cin("C:\\cundang\\1.txt");
				Cin >> maxhp;
				for (int i = 0; i <= 9; i++) Cin >> wupinlan[i];
				for (int i = 0; i <= 9; i++) {
					if (!(Cin >> wupinlanCnt[i])) wupinlanCnt[i] = wupinlan[i] ? 1 : 0;
				}
				Cin.close();
				return;
			}
	}
}

void do_game() {
  updateCamera();
	if (GetAsyncKeyState('M') & 0x0001) minimapEnabled = !minimapEnabled;
	minimapLayout = calcMiniMapLayout();
	for (auto& i : mons) if (i->hp > 0) i->useSkill();

	if (GetAsyncKeyState(0x57) & 0x0001) if (goit(-1, 0, player)) navDirty = true;
	if (GetAsyncKeyState(0x53) & 0x0001) if (goit(1, 0, player)) navDirty = true;
	if (GetAsyncKeyState(0x41) & 0x0001) if (goit(0, -1, player)) navDirty = true;
	if (GetAsyncKeyState(0x44) & 0x0001) if (goit(0, 1, player)) navDirty = true;
	if (GetAsyncKeyState('X') & 0x0001) {
		int bestIdx = -1;
		int bestDist = INF;
		int rangeSq = player.k * player.k;
		for (int i = 0; i < (int)mons.size(); i++) {
			if (mons[i]->hp <= 0) continue;
			int d = getDistSq(player.x, player.y, mons[i]->x, mons[i]->y);
			if (d <= rangeSq && d < bestDist) {
				bestDist = d;
				bestIdx = i;
			}
		}
		if (bestIdx != -1) {
			mons[bestIdx]->hp -= player.hurt + wupin[wupinlan[At]].hurt;
		}
	}
	if ((GetAsyncKeyState('Q') & 0x0001) && wupinlan[At]) {
		diaoluo[player.x][player.y].push(wupinlan[At]);
		wupinlanCnt[At]--;
		if (wupinlanCnt[At] <= 0) {
			wupinlan[At] = 0;
			wupinlanCnt[At] = 0;
		}
	}
	int nextat = 255;
	for (char x = '1'; x <= '9'; x++)
		if (GetAsyncKeyState(x) & 0x0001) nextat = x - '1';
	if (GetAsyncKeyState('0') & 0x0001) nextat = 9;
	if (nextat != 255 && nextat != At) {
		At = nextat;
		watime = chrono::system_clock::now();
	}
	mouse_msg x;
	while (mousemsg()) {
		x = getmouse();
		if (pointInMiniMap(x.x, x.y)) continue;
		if (x.is_left()) {
			gridY = x.x / tileW + camY;
			gridX = x.y / tileH + camX;
			if (x.is_down()) {
				downok = 1;
				leftok = 0;
				wax = gridX, way = gridY;
				watime = chrono::system_clock::now();
			}
			if (x.is_up()) {
				downok = 0;
				watime = chrono::system_clock::now();
			}
			if (downok && !leftok) {
				if (gridX >= 0 && gridX <= mapX + 1 && gridY >= 0 && gridY <= mapY + 1 && Map[gridX][gridY] == 1)
					for (auto& m : mons)
						if (m->hp > 0 && gridX == m->x && gridY == m->y)
							if (getDistSq(player.x, player.y, m->x, m->y) <= player.k * player.k)
								m->hp -= player.hurt + wupin[wupinlan[At]].hurt;
				if (wax != gridX || way != gridY) {
					wax = gridX, way = gridY;
					watime = chrono::system_clock::now();
				}
			}
		} else if (x.is_right()) {
			if (x.is_down()) {
        int tx = x.y / tileH + camX;
        int ty = x.x / tileW + camY;
				if (tx >= 0 && tx <= mapX + 1 && ty >= 0 && ty <= mapY + 1 &&
            wupinlan[At] / 100 == 2 && getDistSq(tx, ty, player.x, player.y) <= player.k * player.k
				    && Map[tx][ty] == 1) {
					Map[tx][ty] = wupinlan[At] % 200;
					navDirty = true;
					minimapTerrainCacheDirty = true;
					wupinlanCnt[At]--;
					if (wupinlanCnt[At] <= 0) {
						wupinlan[At] = 0;
						wupinlanCnt[At] = 0;
					}
				}
			}
		}
	}
	if (downok && !leftok) {
		auto P = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - watime);
		if (wax != gridX || way != gridY) {
			wax = gridX;
			way = gridY;
			watime = chrono::system_clock::now();
		} else if (wax >= 0 && wax <= mapX && way >= 0 && way <= mapY && P.count() >= Map2[wax][way].first * 1000) {
			watime = chrono::system_clock::now();
			if (Map[wax][way] > 1 && player.wa + wupin[wupinlan[At]].wa >= Map2[wax][way].second
			    && getDistSq(wax, way, player.x, player.y) <= player.k * player.k) {
				diaoluo[wax][way].push(200 + Map[wax][way]);
				Map[wax][way] = 1;
				navDirty = true;
				minimapTerrainCacheDirty = true;
				leftok = 1;
			}
		}
	}

	if (navDirty) {
		rebuildNavDist();
		navDirty = false;
	}

	auto now = chrono::system_clock::now();
	for (int i = 0; i < (int)mons.size(); i++) {
		if (mons[i]->hp <= 0) continue;
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastTime[i]);
		if (elapsed.count() >= mons[i]->p) {
			lastTime[i] = now;
			int mx = mons[i]->x, my = mons[i]->y;
			int cur = navDist[mx][my];
			if (cur == 0) {
				// already on player
				} else if (cur == INF) {
					randomWalk(*mons[i]);
				} else {
					pair<int, int> moves[4];
					int movesCnt = 0;
					for (int d = 0; d < 4; d++) {
						int nx = mx + D[d][0], ny = my + D[d][1];
						if (nx < 0 || ny < 0 || nx > mapX + 1 || ny > mapY + 1) continue;
						if (!ok(Map[nx][ny])) continue;
						if (navDist[nx][ny] == cur - 1) moves[movesCnt++] = {D[d][0], D[d][1]};
					}
					if (movesCnt > 0) {
						auto mv = moves[rand() % movesCnt];
						goit(mv.first, mv.second, *mons[i]);
					} else {
						randomWalk(*mons[i]);
					}
				}
		}
		auto Elapsed = chrono::duration_cast<chrono::milliseconds>(now - lasthurt[i]);
		if (Elapsed.count() >= mons[i]->q && getDistSq(player.x, player.y, mons[i]->x, mons[i]->y) <= mons[i]->k * mons[i]->k) {
			player.hp -= mons[i]->hurt;
			lasthurt[i] = now;
			heartWaveActive = true;
			lastHurtTime = now;
		}
	}

	auto Elapsed = chrono::duration_cast<chrono::milliseconds>(now - lasthp);
	if (Elapsed.count() >= 5000) {
		player.hp = min(player.hp + 1, maxhp);
		lasthp = now;
	}

	paintFull(1);
	pinatmap();
	for (auto& m : mons) if (m->hp > 0) paint(m->x, m->y, m->t);
	for (auto& m : mons) if (m->hp > 0) drawHealthBar(*m);
	paint(player.x, player.y, player.t);
	
  // 渲染传送门，只有当它在视野内时
  paint(mapX, mapY, obc::port); 
	drawMiniMap();

  // 判定胜利：玩家到达右下角终点 (mapX, mapY)
	if (player.x == mapX && player.y == mapY) {
		maxhp += guaiShu;
		resultImgTag = obc::win;
		STATE = 2;
	} else if (player.hp <= 0) {
		resultImgTag = obc::loss;
		STATE = 2;
		maxhp -= guaiShu;
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

	int realW = getwidth();
	int realH = getheight();

	if (CreateDirectory("C:\\cundang", NULL)) {
		ofstream Cout("C:\\cundang\\1.txt");
		Cout << 6 << endl;
		Cout << 111 << ' ' << 112 << ' ' << 121 << ' ';
		for (int i = 1; i <= 8; i++) Cout << 0 << ' ';
		Cout << endl;
		Cout << 1 << ' ' << 1 << ' ' << 1 << ' ';
		for (int i = 1; i <= 8; i++) Cout << 0 << ' ';
		Cout << endl;
		Cout.close();
	}
	ifstream Cin("C:\\cundang\\1.txt");
	Cin >> maxhp;
	for (int i = 0; i <= 9; i++) Cin >> wupinlan[i];
	for (int i = 0; i <= 9; i++) {
		if (!(Cin >> wupinlanCnt[i])) wupinlanCnt[i] = wupinlan[i] ? 1 : 0;
	}
	Cin.close();
	
	tileW = realW / (viewW + 4);
	tileH = realH / (viewH + 7);

	wupin[111] = nodeWP(obc::mjb, 10, 1, 0);
	wupindiaoluo[111] = nodeWP(obc::mj, 10, 1, 0);
	
	wupin[112] = nodeWP(obc::sjb, 20, 2, 0);
	wupindiaoluo[112] = nodeWP(obc::sj, 20, 2, 0);
	
	wupin[121] = nodeWP(obc::mgb, 10, 0, 1);
	wupindiaoluo[121] = nodeWP(obc::mg, 10, 0, 1);	
	
	for (auto i : Not) {
		wupin[200 + i] = nodeWP((unsigned char)i, 0, 0, 0);
		wupindiaoluo[200 + i] = nodeWP((unsigned char)i, 0, 0, 0);
	}
	wupindiaoluo[200 + obc::dia] = nodeWP((unsigned char)obc::dia3, 0, 0, 0);
	wupin[200 + obc::dia] = nodeWP((unsigned char)obc::dia2, 0, 0, 0);
	
	string lujing[100] {
		"beijing.jpg", "caofangkuai.jpg",  "nitu.jpg", "chuansongmenfangkuai.jpg",
		"wanjia.jpg", "shenglitupian.jpg", "yuanshi.jpg", "zuanshikuang.jpg",
		"tiekuang.jpg", "jiangshi.jpg",    "jiyan.jpg", "kongbai.jpg",
		"shibai.jpg", "xin.jpg", "kongxin.jpg", "gong.jpg", "jian.jpg",
		"jiqishi.jpg", "nishi.jpg", "zhudating.jpg", "maoxianmoshi.jpg",
		"wupinlan.jpg", "mujian.jpg", "mujianblack.jpg", "wupinlanat.jpg",
		"shijian.jpg", "shijianblack.jpg", "zhizhu.jpg", "chongzhi.jpg","mugao.jpg",
		"mugaob.jpg","zuanshib.jpg","zuanshi.jpg" 
	};

	for (int i = 0; i < 33; i++) {
		img[i + 1] = newimage();
		string _ = "./caizhibao/" + lujing[i];
		getimage(img[i + 1], _.c_str());
	}
	while (is_run()) {
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;
		if (STATE == 0) do_menu();
		else if (STATE == 1) do_game();
		else if (STATE == 2) do_result();
		delay_fps(60);
	}

	ofstream Cout("C:\\cundang\\1.txt");
	Cout << maxhp << endl;
	for (int i = 0; i < 10; i++)
		Cout << wupinlan[i] << ' ';
	Cout << endl;
	for (int i = 0; i < 10; i++)
		Cout << wupinlanCnt[i] << ' ';
	Cout.close();
	for (int i = 1; i <= 33; i++) delimage(img[i]);
	if (minimapTerrainCache) delimage(minimapTerrainCache);
	closegraph();
	cout << "资源释放完毕";
	return 0;
}
