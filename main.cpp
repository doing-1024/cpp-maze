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
const int NOT_COUNT = 6;
int Not[30] = {obc::grass, obc::soil, obc::yuanmu, obc::stone, obc::dia, obc::iron};
bool game_, MAP[110][110];
PIMAGE img[50];
bool imgHasAlpha[50] = {false};
int guaiShu;
pair<int, int> Map2[110][110], Not2[30] = {{3, 0}, {3, 0}, {4, 0}, {5, 1}, {10, 3}, {5, 2}};
int tileW, tileH;

int wupinlan[10], wupinlanCnt[10], At;
const int MAX_STACK = 99;
const int ITEM_MUBAN = 131;
const int ITEM_MUGUN = 132;

struct ItemStack {
	int id = 0;
	int cnt = 0;
};

struct RectI {
	int x = 0, y = 0, w = 0, h = 0;
};

bool pointInRect(int px, int py, const RectI& r) {
	return px >= r.x && py >= r.y && px < r.x + r.w && py < r.y + r.h;
}

bool imageHasAlpha(int tag) {
	return tag >= 0 && tag < 50 && imgHasAlpha[tag];
}

void drawImageTag(int xDest, int yDest, int widthDest, int heightDest, int tag, bool smooth = false) {
	if (tag <= 0 || tag >= 50) return;
	if (!img[tag]) return;
	int srcW = getwidth(img[tag]);
	int srcH = getheight(img[tag]);
	if (imageHasAlpha(tag)) {
		putimage_withalpha(NULL, img[tag], xDest, yDest, widthDest, heightDest, 0, 0, srcW, srcH, smooth);
	} else {
		putimage(xDest, yDest, widthDest, heightDest, img[tag], 0, 0, srcW, srcH);
	}
}

void drawImageTagAt(int xDest, int yDest, int tag) {
	if (tag <= 0 || tag >= 50) return;
	if (!img[tag]) return;
	if (imageHasAlpha(tag)) {
		putimage_withalpha(NULL, img[tag], xDest, yDest);
	} else {
		putimage(xDest, yDest, img[tag]);
	}
}

RectI getHotbarSlotRect(int i) {
	RectI r;
	int row = viewH + 3;
	int itemH = tileH * 1.2;
	int itemW = itemH * 1.2;
	int totalW = itemW * 10;
	int startX = (getwidth() - totalW) / 2;
	int startY = row * tileH;
	r.x = startX + i * itemW;
	r.y = startY;
	r.w = itemW;
	r.h = itemH;
	return r;
}

ItemStack getHotbarStack(int i) {
	return {wupinlan[i], wupinlanCnt[i]};
}

void setHotbarStack(int i, ItemStack s) {
	wupinlan[i] = s.id;
	wupinlanCnt[i] = s.cnt;
	if (wupinlanCnt[i] <= 0 || wupinlan[i] == 0) {
		wupinlan[i] = 0;
		wupinlanCnt[i] = 0;
	}
}

bool canFitInStack(int dstId, int dstCnt, int srcId, int srcCnt) {
	if (srcId == 0 || srcCnt <= 0) return true;
	if (dstId == 0) return srcCnt <= MAX_STACK;
	if (dstId != srcId) return false;
	return dstCnt + srcCnt <= MAX_STACK;
}

bool addToInventory(int itemId);

bool canAddToInventoryCount(int itemId, int cnt) {
	if (itemId == 0 || cnt <= 0) return false;
	int remaining = cnt;
	for (int i = 0; i < 10; i++) {
		if (wupinlan[i] == itemId) {
			int space = MAX_STACK - wupinlanCnt[i];
			if (space > 0) {
				int add = min(space, remaining);
				remaining -= add;
				if (remaining <= 0) return true;
			}
		}
	}
	for (int i = 0; i < 10; i++) {
		if (wupinlan[i] == 0) {
			int add = min(MAX_STACK, remaining);
			remaining -= add;
			if (remaining <= 0) return true;
		}
	}
	return false;
}

bool addToInventoryCount(int itemId, int cnt) {
	if (itemId == 0 || cnt <= 0) return false;
	if (!canAddToInventoryCount(itemId, cnt)) return false;
	for (int k = 0; k < cnt; k++) {
		addToInventory(itemId);
	}
	return true;
}

// -------- Crafting (E) --------
bool craftingOpen = false;
int craftGridId[3][3] = {{0}};
int craftGridCnt[3][3] = {{0}};
bool craftDirty = true;

struct CraftRecipe {
	int inId[3][3] = {{0}};
	int outId = 0;
	int outCnt = 1;
};
vector<CraftRecipe> craftRecipes;

const CraftRecipe* craftPreviewRecipe = nullptr;
int craftPreviewOutId = 0;
int craftPreviewOutCnt = 0;
int craftPreviewOffsetR = 0;
int craftPreviewOffsetC = 0;

int lastMouseX = 0, lastMouseY = 0;

enum class DragSrc {
	None,
	Hotbar,
	Craft,
	Output
};

struct DragState {
	bool active = false;
	DragSrc src = DragSrc::None;
	ItemStack stack;
	int hotbarIdx = -1;
	int craftR = -1, craftC = -1;
	const CraftRecipe* outRecipe = nullptr;
	int outOffsetR = 0, outOffsetC = 0;
	bool suppressOutput = false;
	bool splitOne = false; // 从堆叠中只取出1个
};

DragState drag;

void initCraftingRecipes() {
	craftRecipes.clear();
	int logItem = 200 + obc::yuanmu;
	int stoneItem = 200 + obc::stone;
	int plankItem = ITEM_MUBAN;
	int stickItem = ITEM_MUGUN;

	// 原木 -> 4木板
	{
		CraftRecipe r;
		r.inId[0][0] = logItem;
		r.outId = plankItem;
		r.outCnt = 4;
		craftRecipes.push_back(r);
	}

	// 2木板(竖排) -> 4木棍
	{
		CraftRecipe r;
		r.inId[0][0] = plankItem;
		r.inId[1][0] = plankItem;
		r.outId = stickItem;
		r.outCnt = 4;
		craftRecipes.push_back(r);
	}

	// 木镐：上排3木板，中间竖2木棍
	{
		CraftRecipe r;
		r.inId[0][0] = plankItem; r.inId[0][1] = plankItem; r.inId[0][2] = plankItem;
		r.inId[1][1] = stickItem;
		r.inId[2][1] = stickItem;
		r.outId = 121;
		r.outCnt = 1;
		craftRecipes.push_back(r);
	}
	// 木剑：2木板 + 1木棍
	{
		CraftRecipe r;
		r.inId[0][0] = plankItem;
		r.inId[1][0] = plankItem;
		r.inId[2][0] = stickItem;
		r.outId = 111;
		r.outCnt = 1;
		craftRecipes.push_back(r);
	}
	// 石剑：2石头 + 1木棍
	{
		CraftRecipe r;
		r.inId[0][0] = stoneItem;
		r.inId[1][0] = stoneItem;
		r.inId[2][0] = stickItem;
		r.outId = 112;
		r.outCnt = 1;
		craftRecipes.push_back(r);
	}
}

void clearCraftingGrid() {
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) {
			craftGridId[r][c] = 0;
			craftGridCnt[r][c] = 0;
		}
	craftDirty = true;
}

void returnCraftingGridToInventoryOrDrop();

struct CraftMatch {
	const CraftRecipe* recipe = nullptr;
	int offsetR = 0, offsetC = 0;
};

CraftMatch matchCrafting() {
	int minR = 3, minC = 3, maxR = -1, maxC = -1;
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++)
			if (craftGridId[r][c] != 0 && craftGridCnt[r][c] > 0) {
				minR = min(minR, r);
				minC = min(minC, c);
				maxR = max(maxR, r);
				maxC = max(maxC, c);
			}
	if (maxR == -1) return {};

	int norm[3][3] = {{0}};
	for (int r = minR; r <= maxR; r++)
		for (int c = minC; c <= maxC; c++)
			norm[r - minR][c - minC] = craftGridId[r][c];

	for (auto& rec : craftRecipes) {
		bool ok = true;
		for (int r = 0; r < 3 && ok; r++)
			for (int c = 0; c < 3; c++) {
				int req = rec.inId[r][c];
				int cur = norm[r][c];
				if (req == 0) {
					if (cur != 0) { ok = false; break; }
				} else {
					if (cur != req) { ok = false; break; }
					int ar = minR + r, ac = minC + c;
					if (ar < 0 || ar >= 3 || ac < 0 || ac >= 3) { ok = false; break; }
					if (craftGridCnt[ar][ac] < 1) { ok = false; break; }
				}
			}
		if (ok) return {&rec, minR, minC};
	}
	return {};
}

void refreshCraftPreview() {
	craftPreviewRecipe = nullptr;
	craftPreviewOutId = 0;
	craftPreviewOutCnt = 0;
	craftPreviewOffsetR = 0;
	craftPreviewOffsetC = 0;

	auto m = matchCrafting();
	if (!m.recipe) return;
	craftPreviewRecipe = m.recipe;
	craftPreviewOutId = m.recipe->outId;
	craftPreviewOutCnt = m.recipe->outCnt;
	craftPreviewOffsetR = m.offsetR;
	craftPreviewOffsetC = m.offsetC;
}

void consumeCraftIngredients(const CraftRecipe* rec, int offR, int offC) {
	if (!rec) return;
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) {
			if (rec->inId[r][c] == 0) continue;
			int ar = offR + r, ac = offC + c;
			if (ar < 0 || ar >= 3 || ac < 0 || ac >= 3) continue;
			craftGridCnt[ar][ac]--;
			if (craftGridCnt[ar][ac] <= 0) {
				craftGridId[ar][ac] = 0;
				craftGridCnt[ar][ac] = 0;
			}
		}
	craftDirty = true;
}

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

void returnCraftingGridToInventoryOrDrop() {
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) {
			int id = craftGridId[r][c];
			int cnt = craftGridCnt[r][c];
			if (id == 0 || cnt <= 0) continue;
			if (!addToInventoryCount(id, cnt)) {
				// 背包满了就掉地上（当前玩家格子）
				for (int k = 0; k < cnt; k++) diaoluo[player.x][player.y].push(id);
			}
			craftGridId[r][c] = 0;
			craftGridCnt[r][c] = 0;
		}
	craftDirty = true;
}

float dropItemAngle[110][110] = {0};

bool ok(int x) {
  if (x == obc::bedrock) return false;
	for (int i = 0; i < NOT_COUNT; i++) if (x == Not[i]) return false;
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
		case obc::yuanmu: return EGERGB(130, 90, 50);
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
		drawImageTag(drawY * tileW, drawX * tileH + offsetY, tileW, tileH, t);
	}
}

void paintFull(int t) {
	drawImageTag(0, 0, getwidth(), getheight(), t, true);
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
		drawImageTag(i * tileW, row * tileH + offsetY, tileW, tileH, obc::heart);
	}
	for (int i = min(maxPerRow, player.hp); i < min(maxPerRow, maxhp); i++) {
		int offsetY = 0;
		if (heartWaveActive) {
			auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastHurtTime);
			double phase = (elapsed.count() * 0.001 * HEART_WAVE_FREQUENCY) - (i * 0.3);
			offsetY = (int)(sin(phase * 3.14159 * 2) * HEART_WAVE_AMPLITUDE);
		}
		drawImageTag(i * tileW, row * tileH + offsetY, tileW, tileH, obc::vheart);
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
		if (i == At) drawImageTag(startX + i * itemW, startY, itemW, itemH, obc::wplat);
		else drawImageTag(startX + i * itemW, startY, itemW, itemH, obc::wupinlan);
		
		if (wupinlan[i]) {
      int iconSize = (int)(itemH * 0.7);
      int paddingX = (itemW - iconSize) / 2;
      int paddingY = (itemH - iconSize) / 2;
			drawImageTag(startX + i * itemW + paddingX, startY + paddingY, iconSize, iconSize, wupin[wupinlan[i]].t);
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

ItemStack getCraftStack(int r, int c) {
	return {craftGridId[r][c], craftGridCnt[r][c]};
}

void setCraftStack(int r, int c, ItemStack s) {
	craftGridId[r][c] = s.id;
	craftGridCnt[r][c] = s.cnt;
	if (craftGridId[r][c] == 0 || craftGridCnt[r][c] <= 0) {
		craftGridId[r][c] = 0;
		craftGridCnt[r][c] = 0;
	}
	craftDirty = true;
}

struct CraftUILayout {
	bool valid = false;
	RectI window;
	RectI grid[3][3];
	RectI out;
	int slot = 0;
	int gap = 0;
	int headerH = 28;
};

CraftUILayout calcCraftUILayout() {
	CraftUILayout l;
	int sw = getwidth(), sh = getheight();
	int slot = min(tileW, tileH);
	if (slot < 36) slot = 36;
	if (slot > 72) slot = 72;
	int gap = max(6, slot / 8);
	int pad = max(10, slot / 4);

	int gridW = slot * 3 + gap * 2;
	int gridH = slot * 3 + gap * 2;

	int wndW = pad * 2 + gridW + pad * 2 + slot + pad * 2;
	int wndH = l.headerH + pad * 2 + gridH;

	int wndX = (sw - wndW) / 2;
	int wndY = (sh - wndH) / 2 - tileH; // 往上挪一点，别挡住底部物品栏
	if (wndY < 10) wndY = 10;

	l.window = {wndX, wndY, wndW, wndH};
	l.slot = slot;
	l.gap = gap;

	int gridX = wndX + pad;
	int gridY = wndY + l.headerH + pad;
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) {
			l.grid[r][c] = {gridX + c * (slot + gap), gridY + r * (slot + gap), slot, slot};
		}

	int outX = gridX + gridW + pad * 2;
	int outY = gridY + (gridH - slot) / 2;
	l.out = {outX, outY, slot, slot};
	l.valid = true;
	return l;
}

void drawSlotBase(const RectI& r, bool highlight) {
	drawImageTag(r.x, r.y, r.w, r.h, highlight ? obc::wplat : obc::wupinlan);
}

void drawStackInSlot(const RectI& r, int itemId, int cnt) {
	if (itemId == 0 || cnt <= 0) return;
	int iconSize = (int)(r.h * 0.72);
	int paddingX = (r.w - iconSize) / 2;
	int paddingY = (r.h - iconSize) / 2;
	Tag tt = wupin[itemId].t;
	drawImageTag(r.x + paddingX, r.y + paddingY, iconSize, iconSize, tt, true);
	if (cnt > 1) {
		int fontH = max(10, (int)(r.h * 0.28));
		setfont(fontH, 0, "Consolas");
		setbkmode(TRANSPARENT);
		settextcolor(WHITE);
		string s = to_string(cnt);
		int tx = r.x + r.w - textwidth(s.c_str()) - 4;
		int ty = r.y + r.h - textheight(s.c_str()) - 2;
		outtextxy(tx, ty, s.c_str());
	}
}

enum class UISlotKind {
	None,
	Hotbar,
	Craft,
	Output
};

struct UISlotRef {
	UISlotKind kind = UISlotKind::None;
	int idx = -1;
	int r = -1, c = -1;
};

UISlotRef hitTestCraftingUI(int mx, int my, const CraftUILayout& l) {
	if (l.valid && pointInRect(mx, my, l.out)) return {UISlotKind::Output, -1, -1, -1};
	if (l.valid) {
		for (int r = 0; r < 3; r++)
			for (int c = 0; c < 3; c++)
				if (pointInRect(mx, my, l.grid[r][c])) return {UISlotKind::Craft, -1, r, c};
	}
	for (int i = 0; i < 10; i++) {
		RectI rr = getHotbarSlotRect(i);
		if (pointInRect(mx, my, rr)) return {UISlotKind::Hotbar, i, -1, -1};
	}
	return {};
}

void drawCraftingUI() {
	if (!craftingOpen) return;
	CraftUILayout l = calcCraftUILayout();

	// 背景遮罩
	setfillcolor(EGERGBA(0, 0, 0, 130));
	ege_fillrect(0, 0, getwidth(), getheight());

	// 窗口
	setfillcolor(EGERGBA(25, 25, 25, 220));
	ege_fillrect(l.window.x, l.window.y, l.window.w, l.window.h);
	setlinecolor(EGEGRAY(220));
	ege_rectangle(l.window.x, l.window.y, l.window.w, l.window.h);

	// 标题
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	setfont(18, 0, "Consolas");
	outtextxy(l.window.x + 12, l.window.y + 6, "Crafting (E)");
	setfont(14, 0, "Consolas");
	outtextxy(l.window.x + 200, l.window.y + 8, "Drag items to 3x3");

	// 左侧3x3
	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 3; c++) {
			drawSlotBase(l.grid[r][c], false);
			drawStackInSlot(l.grid[r][c], craftGridId[r][c], craftGridCnt[r][c]);
		}

	// 箭头
	settextcolor(EGEGRAY(240));
	setfont(26, 0, "Consolas");
	int arrowX = l.grid[1][2].x + l.slot + 18;
	int arrowY = l.grid[1][2].y + l.slot / 2 - 10;
	outtextxy(arrowX, arrowY, "=>");

	// 结果格
	drawSlotBase(l.out, false);
	if (!(drag.active && drag.suppressOutput)) {
		drawStackInSlot(l.out, craftPreviewOutId, craftPreviewOutCnt);
	}

	// 拖拽中物品
		if (drag.active && drag.stack.id != 0 && drag.stack.cnt > 0) {
			int iconSize = (int)(l.slot * 0.78);
			int px = lastMouseX - iconSize / 2;
			int py = lastMouseY - iconSize / 2;
			Tag tt = wupin[drag.stack.id].t;
			drawImageTag(px, py, iconSize, iconSize, tt, true);
			if (drag.stack.cnt > 1) {
				int fontH = max(10, (int)(iconSize * 0.28));
				setfont(fontH, 0, "Consolas");
				setbkmode(TRANSPARENT);
			settextcolor(WHITE);
			string s = to_string(drag.stack.cnt);
			outtextxy(px + iconSize - textwidth(s.c_str()) - 2, py + iconSize - textheight(s.c_str()) - 2, s.c_str());
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
					drawImageTag(drawY * tileW + 4 + offsetX, drawX * tileH + 5, dstW, dropBaseH, t);
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
	craftingOpen = false;
	drag = DragState();
	clearCraftingGrid();
	craftPreviewRecipe = nullptr;
	craftPreviewOutId = 0;
	craftPreviewOutCnt = 0;

  // 先初始化全部为杂乱方块
	for (int i = 0; i <= mapX + 1; i++)
		for (int j = 0; j <= mapY + 1; j++) {
			int p = Rand({35, 25, 20, 15, 1, 4});
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
	drawImageTagAt(100, 185, obc::start);
	drawImageTagAt(550, 185, obc::cz);
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
	if (GetAsyncKeyState('E') & 0x0001) {
		craftingOpen = !craftingOpen;
		if (craftingOpen) {
			downok = 0;
			leftok = 0;
			drag = DragState();
			craftDirty = true;
			} else {
				// 关闭时把拖拽中物品放回去
				if (drag.active) {
					if (drag.src == DragSrc::Hotbar && drag.hotbarIdx >= 0 && drag.hotbarIdx < 10) {
						if (drag.splitOne) {
							ItemStack cur = getHotbarStack(drag.hotbarIdx);
							if (cur.id == 0) setHotbarStack(drag.hotbarIdx, drag.stack);
							else setHotbarStack(drag.hotbarIdx, {cur.id, cur.cnt + drag.stack.cnt});
						} else {
							setHotbarStack(drag.hotbarIdx, drag.stack);
						}
					} else if (drag.src == DragSrc::Craft && drag.craftR >= 0 && drag.craftR < 3 && drag.craftC >= 0 && drag.craftC < 3) {
						if (drag.splitOne) {
							ItemStack cur = getCraftStack(drag.craftR, drag.craftC);
							if (cur.id == 0) setCraftStack(drag.craftR, drag.craftC, drag.stack);
							else setCraftStack(drag.craftR, drag.craftC, {cur.id, cur.cnt + drag.stack.cnt});
						} else {
							setCraftStack(drag.craftR, drag.craftC, drag.stack);
						}
					}
					drag = DragState();
				}
			returnCraftingGridToInventoryOrDrop();
			craftPreviewRecipe = nullptr;
			craftPreviewOutId = 0;
			craftPreviewOutCnt = 0;
		}
	}
	minimapLayout = calcMiniMapLayout();
	if (craftingOpen) {
		CraftUILayout layout = calcCraftUILayout();
		if (craftDirty) {
			refreshCraftPreview();
			craftDirty = false;
		}
		mouse_msg x;
		while (mousemsg()) {
			x = getmouse();
			lastMouseX = x.x;
			lastMouseY = x.y;
			if (pointInMiniMap(x.x, x.y)) continue;
				if (!x.is_left()) continue;

				if (x.is_down() && !drag.active) {
					bool takeOne = x.is_doubleclick();
					UISlotRef hit = hitTestCraftingUI(x.x, x.y, layout);
					if (hit.kind == UISlotKind::Hotbar) {
						ItemStack s = getHotbarStack(hit.idx);
						if (s.id != 0 && s.cnt > 0) {
							drag.active = true;
							drag.src = DragSrc::Hotbar;
							drag.hotbarIdx = hit.idx;
							if (takeOne) {
								drag.splitOne = true;
								drag.stack = {s.id, 1};
								s.cnt -= 1;
								setHotbarStack(hit.idx, s);
							} else {
								drag.stack = s;
								setHotbarStack(hit.idx, {0, 0});
							}
						}
					} else if (hit.kind == UISlotKind::Craft) {
						ItemStack s = getCraftStack(hit.r, hit.c);
						if (s.id != 0 && s.cnt > 0) {
							drag.active = true;
							drag.src = DragSrc::Craft;
							drag.craftR = hit.r;
							drag.craftC = hit.c;
							if (takeOne) {
								drag.splitOne = true;
								drag.stack = {s.id, 1};
								s.cnt -= 1;
								setCraftStack(hit.r, hit.c, s);
							} else {
								drag.stack = s;
								setCraftStack(hit.r, hit.c, {0, 0});
							}
						}
					} else if (hit.kind == UISlotKind::Output) {
						if (craftPreviewRecipe && craftPreviewOutId != 0 && craftPreviewOutCnt > 0) {
							drag.active = true;
							drag.src = DragSrc::Output;
						drag.stack = {craftPreviewOutId, craftPreviewOutCnt};
						drag.outRecipe = craftPreviewRecipe;
						drag.outOffsetR = craftPreviewOffsetR;
						drag.outOffsetC = craftPreviewOffsetC;
						drag.suppressOutput = true;
					}
				}
			}

				if (x.is_up() && drag.active) {
					UISlotRef target = hitTestCraftingUI(x.x, x.y, layout);
					if (target.kind == UISlotKind::Output) target.kind = UISlotKind::None;

					auto restoreToSource = [&]() {
						if (drag.src == DragSrc::Hotbar) setHotbarStack(drag.hotbarIdx, drag.stack);
						else if (drag.src == DragSrc::Craft) setCraftStack(drag.craftR, drag.craftC, drag.stack);
					};
					auto restoreSplitToSource = [&]() {
						if (drag.src == DragSrc::Hotbar && drag.hotbarIdx >= 0 && drag.hotbarIdx < 10) {
							ItemStack cur = getHotbarStack(drag.hotbarIdx);
							if (cur.id == 0) setHotbarStack(drag.hotbarIdx, drag.stack);
							else setHotbarStack(drag.hotbarIdx, {cur.id, cur.cnt + drag.stack.cnt});
						} else if (drag.src == DragSrc::Craft && drag.craftR >= 0 && drag.craftR < 3 && drag.craftC >= 0 && drag.craftC < 3) {
							ItemStack cur = getCraftStack(drag.craftR, drag.craftC);
							if (cur.id == 0) setCraftStack(drag.craftR, drag.craftC, drag.stack);
							else setCraftStack(drag.craftR, drag.craftC, {cur.id, cur.cnt + drag.stack.cnt});
						}
					};

					if (drag.src == DragSrc::Output) {
						bool crafted = false;
						if (target.kind == UISlotKind::Hotbar) {
							CraftMatch cur = matchCrafting();
						bool recipeOk = cur.recipe && cur.recipe == drag.outRecipe &&
						                cur.offsetR == drag.outOffsetR && cur.offsetC == drag.outOffsetC;
						if (recipeOk) {
							ItemStack dst = getHotbarStack(target.idx);
							if (canFitInStack(dst.id, dst.cnt, drag.stack.id, drag.stack.cnt)) {
								if (dst.id == 0) setHotbarStack(target.idx, drag.stack);
								else setHotbarStack(target.idx, {dst.id, dst.cnt + drag.stack.cnt});
								consumeCraftIngredients(cur.recipe, cur.offsetR, cur.offsetC);
								crafted = true;
							}
						}
						}
						drag = DragState();
						if (crafted) {
							refreshCraftPreview();
							craftDirty = false;
						}
						continue;
					}
					if (drag.splitOne) {
						bool placed = false;
						if (target.kind == UISlotKind::Hotbar) {
							ItemStack dst = getHotbarStack(target.idx);
							if (canFitInStack(dst.id, dst.cnt, drag.stack.id, drag.stack.cnt)) {
								if (dst.id == 0) setHotbarStack(target.idx, drag.stack);
								else setHotbarStack(target.idx, {dst.id, dst.cnt + drag.stack.cnt});
								placed = true;
							}
						} else if (target.kind == UISlotKind::Craft) {
							ItemStack dst = getCraftStack(target.r, target.c);
							if (canFitInStack(dst.id, dst.cnt, drag.stack.id, drag.stack.cnt)) {
								if (dst.id == 0) setCraftStack(target.r, target.c, drag.stack);
								else setCraftStack(target.r, target.c, {dst.id, dst.cnt + drag.stack.cnt});
								placed = true;
							}
						}
						if (!placed) restoreSplitToSource();
						drag = DragState();
						continue;
					}

					bool dropped = false;
					if (target.kind == UISlotKind::None) {
						restoreToSource();
						dropped = true;
				} else if (drag.src == DragSrc::Hotbar && target.kind == UISlotKind::Hotbar && target.idx == drag.hotbarIdx) {
					restoreToSource();
					dropped = true;
				} else if (drag.src == DragSrc::Craft && target.kind == UISlotKind::Craft && target.r == drag.craftR && target.c == drag.craftC) {
					restoreToSource();
					dropped = true;
				} else if (target.kind == UISlotKind::Hotbar) {
					ItemStack dst = getHotbarStack(target.idx);
					if (dst.id == 0) {
						setHotbarStack(target.idx, drag.stack);
						dropped = true;
					} else if (dst.id == drag.stack.id && dst.cnt + drag.stack.cnt <= MAX_STACK) {
						setHotbarStack(target.idx, {dst.id, dst.cnt + drag.stack.cnt});
						dropped = true;
					} else {
						// swap
						setHotbarStack(target.idx, drag.stack);
						if (drag.src == DragSrc::Hotbar) setHotbarStack(drag.hotbarIdx, dst);
						else setCraftStack(drag.craftR, drag.craftC, dst);
						dropped = true;
					}
				} else if (target.kind == UISlotKind::Craft) {
					ItemStack dst = getCraftStack(target.r, target.c);
					if (dst.id == 0) {
						setCraftStack(target.r, target.c, drag.stack);
						dropped = true;
					} else if (dst.id == drag.stack.id && dst.cnt + drag.stack.cnt <= MAX_STACK) {
						setCraftStack(target.r, target.c, {dst.id, dst.cnt + drag.stack.cnt});
						dropped = true;
					} else {
						// swap
						setCraftStack(target.r, target.c, drag.stack);
						if (drag.src == DragSrc::Hotbar) setHotbarStack(drag.hotbarIdx, dst);
						else setCraftStack(drag.craftR, drag.craftC, dst);
						dropped = true;
					}
				}

				if (!dropped) restoreToSource();
				drag = DragState();
			}
		}
		if (craftDirty) {
			refreshCraftPreview();
			craftDirty = false;
		}
	} else {
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
	}

	paintFull(1);
	pinatmap();
	for (auto& m : mons) if (m->hp > 0) paint(m->x, m->y, m->t);
	for (auto& m : mons) if (m->hp > 0) drawHealthBar(*m);
	paint(player.x, player.y, player.t);
	
  // 渲染传送门，只有当它在视野内时
  paint(mapX, mapY, obc::port); 
	drawMiniMap();
	drawCraftingUI();

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

	wupin[ITEM_MUBAN] = nodeWP(obc::muban, 0, 0, 0);
	wupindiaoluo[ITEM_MUBAN] = nodeWP(obc::muban, 0, 0, 0);
	wupin[ITEM_MUGUN] = nodeWP(obc::mugun, 0, 0, 0);
	wupindiaoluo[ITEM_MUGUN] = nodeWP(obc::mugun, 0, 0, 0);
	
	for (int idx = 0; idx < NOT_COUNT; idx++) {
		int i = Not[idx];
		wupin[200 + i] = nodeWP((unsigned char)i, 0, 0, 0);
		wupindiaoluo[200 + i] = nodeWP((unsigned char)i, 0, 0, 0);
	}
	wupindiaoluo[200 + obc::dia] = nodeWP((unsigned char)obc::dia3, 0, 0, 0);
	wupin[200 + obc::dia] = nodeWP((unsigned char)obc::dia2, 0, 0, 0);

	initCraftingRecipes();
	
	string lujing[100] {
		"beijing.jpg", "caofangkuai.jpg",  "nitu.jpg", "chuansongmenfangkuai.jpg",
		"wanjia.jpg", "shenglitupian.jpg", "yuanshi.jpg", "zuanshikuang.jpg",
		"tiekuang.jpg", "jiangshi.jpg",    "jiyan.jpg", "kongbai.jpg",
		"shibai.jpg", "xin.jpg", "kongxin.jpg", "gong.jpg", "jian.jpg",
		"jiqishi.jpg", "nishi.jpg", "zhudating.jpg", "maoxianmoshi.jpg",
		"wupinlan.jpg", "mujian.jpg", "mujianblack.jpg", "wupinlanat.jpg",
		"shijian.jpg", "shijianblack.jpg", "zhizhu.jpg", "chongzhi.jpg","mugao.jpg",
		"mugaob.jpg","zuanshib.jpg","zuanshi.jpg",
		"yuanmu.jpg", "muban.jpg", "mugun.png"
	};

	for (int i = 0; i < 36; i++) {
		img[i + 1] = newimage();
		string _ = "./caizhibao/" + lujing[i];
		if (_.size() >= 4 && _.substr(_.size() - 4) == ".png") {
			getimage_pngfile(img[i + 1], _.c_str());
			imgHasAlpha[i + 1] = true;
		} else {
			getimage(img[i + 1], _.c_str());
			imgHasAlpha[i + 1] = false;
		}
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
	for (int i = 1; i <= 36; i++) delimage(img[i]);
	if (minimapTerrainCache) delimage(minimapTerrainCache);
	closegraph();
	cout << "资源释放完毕";
	return 0;
}
