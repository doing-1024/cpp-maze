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
#include "Lencode.h"
#define timePoint std::chrono::system_clock::time_point
// TODO(liurizhou)
#pragma comment(lib, "winmm.lib")
const int INF=0x7f7f7f7f;//TODO(#iuruizhou)无限会很有用 
using namespace std;

/* TODO (liuruizhou#1#): 新增的，不能删 */
int Lrand(){
	srand(time(0));
	return rand();//目的，让我用起来更舒服 （这是我的个人习惯） 
}

int map[110][110], mapX = 13, mapY = 23, maxhp; //地图  地图大小
int Not[30] = {2, 3, 7, 8, 9};             //不可穿过方块编号
int foot[4];   //大汗脚状态 
bool Q,game_,MAP[110][110];//是否开启大汗脚 是否esc map<x,y> 是否访问过 
PIMAGE img[50]; //图片数组
string p;
class Node {   //实体类
	public:
	int x, y;   //位置
	int t;      //类型
	int hp;     //血量
	int hurt;   //伤害
	int p;   	//移速
	int q;      //攻击冷却
	int id=0;   //编号 （玩家默认是0，怪物从[1,INF]区间里） 
	Node(int x,int y,int t,int hp,int hurt,int p,int q):x(x),y(y),t(t),hp(hp),hurt(hurt),p(p),q(q){}
	virtual void useSkill(){return;}//这个东西就是造一个接口，子类可以随意修改，调用时默认调用子类的 
	
};
class comzom:public Node{//普通僵尸类 
public:
	comzom(int x,int y,int t=10,int hp=10,int hurt=2,int p=300,int q=1000):Node(x,y,t,hp,hurt,p,q){}
	void useSkill()override;//普通僵尸技能
	private:
	int state;    //状态 
	timePoint last=std::chrono::system_clock::now();//前时间 
	timePoint now=std::chrono::system_clock::now();//后时间 
	int v;		//冷却时间 
};
class drown:public Node{
	public:
	drown(int x,int y,int t=19,int hp=10,int hurt=5,int p=400,int q=3000):Node(x,y,t,hp,hurt,p,q){}
};
class chickJock:public Node{
	public:
	chickJock(int x,int y,int t=18,int hp=10,int hurt=2,int p=200,int q=700):Node(x,y,t,hp,hurt,p,q){}
};
void comzom::useSkill(){     //浦江加强，获得技能TODO(#1#liuruizhou) 
	//override意思是覆盖原来的接口 
	if(maxhp<15)return;
	
	if(state==0){//0技能前谣 
		last=std::chrono::system_clock::now();
		state=1;
		this->p=INF;//先停止一段时间，再突然加速 
	}if(state==1){//释放技能 
		now=std::chrono::system_clock::now();
		auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
		if(tmp.count()>=3000){
			state=2;
			last=std::chrono::system_clock::now();
			/* TODO (#1#liuruizhou): 更改普降属性 */
			this->hurt=4;this->p=200;this->q=800;
		}
	}if(state==2){//持续时间到了 
		now=std::chrono::system_clock::now();
		auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
		if(tmp.count()>=7000){
			last=std::chrono::system_clock::now();
			this->hurt=2;//改回去 
			this->p=300;
			this->q=1000;
			state=3;
			v=Lrand()%10000;//随机抽取冷却 
		}
	}if(state==3){//
		now=std::chrono::system_clock::now();
		auto tmp=std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
		if(tmp.count()>=v){
			state=0;
		}
	}
}
vector< unique_ptr<Node> >mons;//全新实体队列 
/*                                        TODO(liuruizhou)*/

Node player(0,0,5,6,0,0,0); //初始化玩家
pair<int,int> qu[101][501];//实体移动路径 
int qutop[101],qutop2[101];
const int D[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

bool ok(int x) {//方块是否可穿过
	for (int i = 0; i <= 4; i++)
		if (x == Not[i])
			return false;
	return true;
}
bool quok;
void dfsGetQu(int t,int x1,int y1,int x,int y){ 
	if(x1 == x&&y1 == y){
		quok = 1;
		return;
	}
	for(int i = 0;i < 4;i++){
		int X = x1 + D[i][0],Y = y1 + D[i][1];
		if(ok(map[X][Y])&&!MAP[X][Y]&&X >= 0&&Y >= 0&&X <= mapX + 1&&Y <= mapY + 1){
			MAP[X][Y] = 1;
			qu[t][++qutop[t]] = make_pair(D[i][0],D[i][1]);
			dfsGetQu(t,X,Y,x,y);
			if(quok)
				break;
			qutop[t]--;
			MAP[X][Y] = 0;
		}
	}	
} 
//TODO(liuruizhou)
void getQu(int t,int x,int y){
	qutop[t] = qutop2[t] = quok = 0;
	memset(MAP,0,sizeof(MAP));
	dfsGetQu(t,mons[t]->x,mons[t]->y,x,y);
} //创建实体t移动到(x,y)的路径
void paint(int x, int y, int t) {
	putimage(y * 70, x * 55, img[t]);//在位置x,y防置 图片t  图片=70 * 55像素 最大生命值
}
void painthart() {  //绘制生命值
	for (int i = 0; i < min(27, player.hp); i++)
		paint(mapX + 4, i, 14);
	for (int i = min(27,player.hp); i < min(27,maxhp); i++)
		paint(mapX + 4, i, 15);
	for (int i = 0; i < player.hp - 27; i++)
		paint(mapX + 5, i, 14);
	for (int i = player.hp - 27; i < maxhp - 27; i++)
		paint(mapX + 5, i, 15);	
}
void pinatmap() { // 绘制初始地图
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
void makemap(int x, int y) { // dfs创建地图
	int c[4][2] = { 0, 1, 1, 0, 0, -1, -1, 0 }, j, i;
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
int Get(int x, int y) { // 获取移动类型
	if (x == -1 && y == 0)
		return 1;
	if (x == 1 && y == 0)
		return 2;
	if (x == 0 && y == -1)
		return 3;
	return 4;
}
bool getok(int x1, int y1, int x2, int y2) { // 是否进行括号序列删除
	int x = Get(x1, y1), y = Get(x2, y2);
	if (y - x == 1 && y % 2 == 0)
		return true;
	if (y - x == -1 && x % 2 == 0)
		return true;
	return false;
}
void goit(int x, int y, Node&t) { // 实体t从(X,Y)移动到(X+x,Y+y)
	int newx = t.x + x, newy = t.y + y;
	if (newx <= mapX + 1 && newx >= 0 && newy <= mapY + 1 && newy >= 0 && ok(map[newx][newy])) {
		paint(t.x + 1, t.y + 1, 1);
		paint(newx + 1, newy + 1, t.t);
		t.x = newx;t.y = newy;
		if(t.t == obc::player){
			qu[t.id][++qutop[t.id]] = make_pair(x,y);
			while (qutop[t.id] - 1 > qutop2[t.id] && qutop[t.id] >= 2 && qutop2[t.id] >= 0)
				if (qutop[t.id] >= 2 && getok(qu[t.id][qutop[t.id]].first, qu[t.id][qutop[t.id]].second, qu[t.id][qutop[t.id] - 1].first, qu[t.id][qutop[t.id] - 1].second))
					qutop[t.id] -= 2;
				else 
					break;
		}
	}
}//TODO（刘瑞洲）更改了	for(int i : x)
int Rand(vector<int> x){//随机事件抽取器 
	int n = 0; 
	for(int i : x) n += i;
	for(int i = 1;i < (int)x.size();i++) 
	x[i] += x[i-1];
	int y = rand() % n;
	for(int i = 0;i < (int)x.size();i++) 
		if(y < x[i]){
			return i; 
		}
}

void stratgame() { // 主循环
	srand(time(0));
	mons.clear();
	player={0,0,5,maxhp,0,0,0};
	int _ = Rand({60,30,10});
	if (_ == 0)
		mons.push_back(make_unique<comzom>(0,0)); //普僵
	if (_ == 1)
		mons.push_back(make_unique<drown>(0,0)); //溺尸 
	if (_ == 2)
		mons.push_back(make_unique<chickJock>(0,0)); //鸡骑士 
	mons.back()->id=mons.size();
	for(auto&p:mons){
		qutop[p->id] = qutop2[p->id] = 0;
	}
	for (int i = 0; i <= mapX + 1; i++)
		for (int j = 0; j <= mapY + 1; j++)
			map[i][j] = Not[Rand({40,30,15,1,4})];
	makemap(2 * (rand() % (mapX / 2) + 1), 2 * (rand() % (mapY / 2) + 1));
	map[mapX + 1][mapY + 1] = 4;//设置出口
	pinatmap();
	paint(1, 1, 5);//绘制玩家
	//初始化地图
	auto lastTime = std::chrono::system_clock::now();
	auto lasthurt = std::chrono::system_clock::now();
	while(1){ 
		for(auto&i:mons){
			i->useSkill();//TODO (liuruizhou#)
		}
		if(GetAsyncKeyState(0x57) & 0x0001) // 'W'
			goit(-1,0,player);
		if(GetAsyncKeyState(0x53) & 0x0001) // 'S'
			goit(1,0,player);
		if(GetAsyncKeyState(0x41) & 0x0001) // 'A'
			goit(0,-1,player);
		if(GetAsyncKeyState(0x44) & 0x0001) // 'D'
			goit(0,1,player);
		if(GetAsyncKeyState(0x43) & 0x0001) // 'C'
			Q = !Q;
		if(GetAsyncKeyState(VK_ESCAPE) & 0x0001) { // ESC
			break;
			game_ = 1;
		}
		if(player.x == mapX + 1&&player.y == mapY + 1){ // 通关事件 
			int __ = rand() % 4;
			if(_ == 2&&foot[__] == 1){ //是否掉落大汗脚 
				paint(0,0,20);
				foot[__] = 2;
			}else
				paint(0,0,6);
			break;
		}
		auto now = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime);//距离上一次事件时间 
		auto Elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lasthurt);
		for(int i=0;i<mons.size();i++){
			if(elapsed.count() >= mons[i]->p)//处理移动 
			if(qutop2[i] < qutop[i]){
				goit(qu[i][++qutop2[i]].first,qu[i][qutop2[i]].second,*mons[i]);
				lastTime = now;
			}
			if(Elapsed.count() >= mons[i]->q&&player.x == mons[i]->x&&player.y == mons[i]->y){//处理伤害 
				player.hp -= mons[i]->hurt;
				lasthurt = now;
				painthart();
			}//玩家受到伤害 
			if(player.hp <= 0){
				paint(0,0,13);
				return;//结束游戏 
			}//玩家死亡
		}
		delay_ms(10);
	}
	return;	
}
int main() {
	ShowWindow(GetForegroundWindow(),SW_MAXIMIZE);
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	initgraph((int)screen_width,(int)screen_height,INIT_NOFORCEEXIT);
	string lujing[100] {
		"背景.jpg", "草方块.jpg",  "泥土.jpg", "传送门方块.jpg",
		"玩家.jpg", "胜利图片.jpg", "原石.jpg", "钻石矿.jpg",
		"铁矿.jpg", "僵尸.jpg",    "基岩.jpg", "空白.jpg",
		"失败.jpg", "心.jpg", "空心.jpg", "弓.jpg", "箭.jpg",
		"鸡骑士.jpg","溺尸.jpg","雨姐.jpg"
	}; 
	for (int i = 0; i < 20; i++) {
		img[i + 1] = newimage();
		string _ = "C:\\走迷宫\\材质\\原版\\" + lujing[i];
		getimage(img[i + 1], _.c_str());
	}
	//mciSendString(TEXT("open C:\\走迷宫\\音频\\稻香.mp3 alias s1"), NULL, 0, NULL);
	//mciSendString(TEXT("play s1"), NULL, 0, NULL);
	maxhp = 6;
	for(int i = 0;i < 4;i++)
		foot[i] = 1;
	while (1) {
		paint(0, 0, 12);
		if(maxhp > 54)
			maxhp = 54;
		stratgame();
		while(1){
			int	x = getch();
			if(x == ' ')
				break;
			if(x == 27){
				game_ = 1;
				break;
			}
		}
		if(game_)
			break;
	}
	mciSendString(TEXT("close s1"), NULL, 0, NULL);
	for (int i = 1; i <= 20; i++)
		delimage(img[i]);
	end:
	closegraph();
}
