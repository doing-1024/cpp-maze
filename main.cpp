#include <iostream>
#include <graphics.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <queue>
#include <ctime>
#include <memory>
#include <chrono>
#include <windows.h>
#include <vector>
#pragma comment(lib, "winmm.lib")
#define timePoint std::chrono::system_clock::time_point

const int INF = 0x7f7f7f7f;
using namespace std;

int map[110][110], mapX = 15, mapY = 23, maxhp;
int Not[30] = {2, 3, 7, 8, 9};
int foot[4];
bool Q, game_, MAP[110][110];
PIMAGE img[50];
string p;
int guaiShu;

class Node {
public:
    int x, y, t, hp, hurt, p, q, id;
    Node(int x=0, int y=0, int t=0, int hp=0, int hurt=0, int p=0, int q=0)
        : x(x), y(y), t(t), hp(hp), hurt(hurt), p(p), q(q), id(0) {}
    virtual void useSkill() {}
};

class comzom : public Node {
public:
    comzom(int x, int y, int t=10, int hp=10, int hurt=2, int p=300, int q=1000)
        : Node(x, y, t, hp, hurt, p, q) {}
    void useSkill() override;
private:
    int state = 0;
    timePoint last = chrono::system_clock::now();
    timePoint now = chrono::system_clock::now();
    int v;
};

class drown : public Node {
public:
    drown(int x, int y, int t=19, int hp=10, int hurt=5, int p=400, int q=3000)
        : Node(x, y, t, hp, hurt, p, q) {}
};

class chickJock : public Node {
public:
    chickJock(int x, int y, int t=18, int hp=10, int hurt=2, int p=200, int q=700)
        : Node(x, y, t, hp, hurt, p, q) {}
};

void comzom::useSkill() {
    if (maxhp < 15) return;
    if (state == 0) {
        last = chrono::system_clock::now();
        state = 1;
        this->p = INF;
    }
    if (state == 1) {
        now = chrono::system_clock::now();
        auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
        if (tmp.count() >= 3000) {
            state = 2;
            last = chrono::system_clock::now();
            this->hurt = 4; this->p = 200; this->q = 800;
        }
    }
    if (state == 2) {
        now = chrono::system_clock::now();
        auto tmp = chrono::duration_cast<chrono::milliseconds>(now - last);
        if (tmp.count() >= 7000) {
            last = chrono::system_clock::now();
            this->hurt = 2; this->p = 300; this->q = 1000;
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

vector<unique_ptr<Node>> mons;
Node player(0, 0, 5, 6, 0, 0, 0);
pair<int, int> qu[101][1001];
int qutop[101], qutop2[101];
const int D[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

bool ok(int x) {
    for (int i = 0; i <= 4; i++)
        if (x == Not[i]) return false;
    return true;
}

bool quok;
void dfsGetQu(int t, int x1, int y1, int x, int y) {
    if (x1 == x && y1 == y) {
        quok = 1;
        return;
    }
    for (int i = 0; i < 4; i++) {
        int X = x1 + D[i][0], Y = y1 + D[i][1];
        if (ok(map[X][Y]) && !MAP[X][Y] && X >= 0 && Y >= 0 && X <= mapX + 1 && Y <= mapY + 1) {
            MAP[X][Y] = 1;
            qu[t][++qutop[t]] = make_pair(D[i][0], D[i][1]);
            dfsGetQu(t, X, Y, x, y);
            if (quok) break;
            qutop[t]--;
            MAP[X][Y] = 0;
        }
    }
}

void getQu(int t, int x, int y) {
    qutop[t] = qutop2[t] = quok = 0;
    memset(MAP, 0, sizeof(MAP));
    dfsGetQu(t, mons[t]->x, mons[t]->y, x, y);
}

void paint(int x, int y, int t) {
    putimage(y * 70, x * 55, img[t]);
}

void painthart() {
    for (int i = 0; i < min(27, player.hp); i++)
        paint(mapX + 4, i, 14);
    for (int i = min(27, player.hp); i < min(27, maxhp); i++)
        paint(mapX + 4, i, 15);
    for (int i = 0; i < player.hp - 27; i++)
        paint(mapX + 5, i, 14);
    for (int i = player.hp - 27; i < maxhp - 27; i++)
        paint(mapX + 5, i, 15);
}

void pinatmap() {
    paint(0, 0, 1);
    for (int i = 0; i <= mapX + 1; i++) {
        paint(i + 1, 0, 11);
        for (int j = 0; j <= mapY + 1; j++)
            paint(i + 1, j + 1, map[i][j]);
        paint(i + 1, mapY + 3, 11);
    }
    for (int i = 0; i <= mapY + 3; i++) {
        paint(0, i, 11);
        paint(mapX + 3, i, 11);
    }
    painthart();
}

void makemap(int x, int y) {
    int c[4][2] = {0, 1, 1, 0, 0, -1, -1, 0}, j, i;
    for (i = 0; i < 4; i++) {
        j = rand() % 4;
        swap(c[i][0], c[j][0]);
        swap(c[i][1], c[j][1]);
    }
    map[x][y] = 1;
    for (i = 0; i < 4; i++)
        if (!ok(map[x + 2 * c[i][0]][y + 2 * c[i][1]])) {
          map[x + c[i][0]][y + c[i][1]] = 1;
          makemap(x + 2 * c[i][0], y + 2 * c[i][1]);
        }
}

int Get(int x, int y) {
    if (x == -1 && y == 0) return 1;
    if (x == 1 && y == 0) return 2;
    if (x == 0 && y == -1) return 3;
    return 4;
}

bool getok(int x1, int y1, int x2, int y2) {
    int x = Get(x1, y1), y = Get(x2, y2);
    if (y - x == 1 && y % 2 == 0) return true;
    if (y - x == -1 && x % 2 == 0) return true;
    return false;
}

void goit(int x, int y, Node& t) {
    int newx = t.x + x, newy = t.y + y;
    if (newx <= mapX + 1 && newx >= 0 && newy <= mapY + 1 && newy >= 0 && ok(map[newx][newy])) {
        paint(t.x + 1, t.y + 1, 1);
        paint(newx + 1, newy + 1, t.t);
        t.x = newx; t.y = newy;
        if (t.t == 5) {
            for (int i = 0; i < mons.size(); i++) {
                qu[i][++qutop[i]] = make_pair(x, y);
                while (qutop[i] - 1 > qutop2[i] && qutop[i] >= 2 && qutop2[i] >= 0)
                    if (qutop[i] >= 2 && getok(qu[i][qutop[i]].first, qu[i][qutop[i]].second, 
                        qu[i][qutop[i] - 1].first, qu[i][qutop[i] - 1].second))
                        qutop[i] -= 2;
                    else break;
            }
        }
    }
}

int Rand(vector<int> x) {
    int n = 0;
    for (int i : x) n += i;
    for (int i = 1; i < (int)x.size(); i++)
        x[i] += x[i - 1];
    int y = rand() % n;
    for (int i = 0; i < (int)x.size(); i++)
        if (y < x[i]) return i;
    return 0;
}

pair<int, int> getxy() {
    pair<int, int> xy;
    xy.first = rand() % (mapX + 1);
    xy.second = rand() % (mapY + 1);
    while (!ok(map[xy.first][xy.second])) {
        xy.first = rand() % (mapX + 1);
        xy.second = rand() % (mapY + 1);
    }
    return xy;
}

void stratgame() {
    srand(time(0));
    mons.clear();
    player = {0, 0, 5, maxhp, 0, 0, 0};
    
    for (int i = 0; i <= mapX + 1; i++)
        for (int j = 0; j <= mapY + 1; j++)
            map[i][j] = Not[Rand({40, 30, 15, 1, 4})];
    makemap(2 * (rand() % (mapX / 2) + 1), 2 * (rand() % (mapY / 2) + 1));
    map[mapX + 1][mapY + 1] = 4;
    
    int maxType = 0;
    for (int i = 0; i < guaiShu; i++) {
        int type = Rand({20, 50, 30});
        pair<int, int> xy = getxy();
        if (type == 0)
            mons.push_back(make_unique<comzom>(xy.first, xy.second));
        else if (type == 1)
            mons.push_back(make_unique<drown>(xy.first, xy.second));
        else
            mons.push_back(make_unique<chickJock>(xy.first, xy.second));
        mons.back()->id = mons.size() - 1;
        getQu(mons.size() - 1, 0, 0);
        maxType = max(maxType, type);
    }
    
    for (auto& p : mons) {
        qutop[p->id] = qutop2[p->id] = 0;
    }
    
    pinatmap();
    paint(1, 1, 5);
    
    vector<timePoint> lastTime(guaiShu), lasthurt(guaiShu);
    for (int i = 0; i < guaiShu; i++) {
        lastTime[i] = chrono::system_clock::now();
        lasthurt[i] = chrono::system_clock::now();
    }
    
    while (1) { 
      for (auto& i : mons) i->useSkill();
        
      if (GetAsyncKeyState(0x57) & 0x0001) goit(-1, 0, player);
      if (GetAsyncKeyState(0x53) & 0x0001) goit(1, 0, player);
      if (GetAsyncKeyState(0x41) & 0x0001) goit(0, -1, player);
      if (GetAsyncKeyState(0x44) & 0x0001) goit(0, 1, player);
      if (GetAsyncKeyState(0x43) & 0x0001) Q = !Q;
      if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) {
        game_ = 1;
        break;
      }
        
      if (player.x == mapX + 1 && player.y == mapY + 1) {
        maxhp += guaiShu * maxType;
        int footIdx = rand() % 4;
        if (guaiShu >= 3 && foot[footIdx] == 1) {
          paint(0, 0, 20);
          foot[footIdx] = 2;
        } else
          paint(0, 0, 6);
        break;
      }
        
      auto now = chrono::system_clock::now();
      for (int i = 0; i < mons.size(); i++) {
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastTime[i]);
        if (elapsed.count() >= mons[i]->p && qutop2[i] < qutop[i]) {
          goit(qu[i][++qutop2[i]].first, qu[i][qutop2[i]].second, *mons[i]);
          lastTime[i] = now;
        }
            
        auto Elapsed = chrono::duration_cast<chrono::milliseconds>(now - lasthurt[i]);
        if (Elapsed.count() >= mons[i]->q && player.x == mons[i]->x && player.y == mons[i]->y) {
          player.hp -= mons[i]->hurt;
          lasthurt[i] = now;
          painthart();
        }
      }
        
      if (player.hp <= 0) {
        paint(0, 0, 13);
        maxhp--;
        break;
      }
      delay_ms(16);
    }
}

void getStrat() {
    paint(0, 0, 21);
    putimage(300, 300, img[22]);
    mouse_msg msg;
    while (1) {
        while (mousemsg()) {
            msg = getmouse();
            if (msg.msg == mouse_msg_down && msg.is_left())
                if (msg.x >= 300 && msg.y >= 300 && msg.x <= 618 && msg.y <= 462) {
                    guaiShu = Rand({0, 60, 20, 15, 5});
                    return;
                }
        }
        delay_ms(16);
    }
}

int main() {
    ShowWindow(GetForegroundWindow(), SW_MAXIMIZE);
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    initgraph((int)screen_width, (int)screen_height, INIT_NOFORCEEXIT);
    
    string lujing[100] {
		"beijing.jpg", "caofangkuai.jpg",  "nitu.jpg", "chuansongmenfangkuai.jpg",
		"wanjia.jpg", "shenglitupian.jpg", "yuanshi.jpg", "zuanshikuang.jpg",
		"tiekuang.jpg", "jiangshi.jpg",    "jiyan.jpg", "kongbai.jpg",
		"shibai.jpg", "xin.jpg", "kongxin.jpg", "gong.jpg", "jian.jpg",
		"jiqishi.jpg","nishi.jpg","yujie.jpg","zhudating.jpg",
		"maoxianmoshi.jpg",
    };
    
    for (int i = 0; i < 22; i++) {
        img[i + 1] = newimage();
        string _ = "./caizhibao/" + lujing[i];
        getimage(img[i + 1], _.c_str());
    }
    
    //mciSendString(TEXT("open C:\\迷宫游戏\\音频\\音乐.mp3 alias s1"), NULL, 0, NULL);
    //mciSendString(TEXT("play s1"), NULL, 0, NULL);
    
    maxhp = 6;
    for (int i = 0; i < 4; i++) foot[i] = 1;
    
    while (1) {
        paint(0, 0, 12);
        getStrat();
        if (maxhp > 54) maxhp = 54;
        stratgame();
        
        while (1) {
          int x = getch();
          if (x == ' ') break;
          if (x == 27) {
            game_ = 1;
            break;
          }
        }
        if (game_) break;
    } 
    
    mciSendString(TEXT("close s1"), NULL, 0, NULL);
    for (int i = 1; i <= 22; i++) delimage(img[i]);
    closegraph();
}
