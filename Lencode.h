#pragma once
#define Tag ObTag
#include<stdint.h>
//coder name:liuruizhou，AI：doubao 
/*
这个可以让我们不用记hyj的编码，直接输入名称就行 
*/
//把每个对象的编码转化成字符 
typedef uint8_t ObTag;//统一的t的运用
namespace obc{//object encode list 对象编码列表
	enum encode{
		backGround = 1,  // BG的别名
		BG = 1,          // 背景
		grass = 2,       // 草方块
		soil = 3,        // 泥土
		portal = 4,      // 传送门方块
		port = 4,        // portal的别名
		player = 5,      // 玩家
		win = 6,         // 胜利图片
		stone = 7,       // 原石
		diamond = 8,     // 钻石矿
		dia = 8,         // diamond的别名
		iron = 9,        // 铁矿
		zombie = 10,     // 僵尸
		comzom = 10,	 // 普通僵尸别名
		bedrock = 11,    // 基岩
		none = 12,       // 空白
		blank = 12,      // 真实的空白 
		loss = 13,       // 失败 
		heart = 14,      // 心
		vheart = 15,     // 空心
		bow = 16,        // 弓
		arrow = 17,      // 箭 
		arr = 17,        // arrow的别名
		chickenJockey=18,// 鸡骑士
		chickJock=18,    // 简称
		CJ = 18,         // 简称*2
		drowned = 19,    // 溺尸
		drown = 19,      // 溺尸别名
		yujie = 20,      // 雨姐
		zhudating = 21,  // 主大厅
		start = 22       // 开始按钮 
	};
}
/*
1:背景.jpg
2:草方块.jpg
3:泥土.jpg
4:传送门方块.jpg
5:玩家.jpg
6:胜利图片.jpg
7:原石.jpg
8:钻石矿.jpg
9:铁矿.jpg
10:僵尸.jpg
11:基岩.jpg
12:空白.jpg
13:失败.jpg
14:心.jpg
15:空心.jpg
16:弓.jpg
17:箭.jpg
18:鸡骑士.jpg
19:溺尸.jpg
20:雨姐.jpg
*/
