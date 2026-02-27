#pragma once
#define Tag ObTag
#include<stdint.h>
//coder name:liuruizhou£¬AI£ºdoubao 
/*
Õâ¸ö¿ÉÒÔÈÃÎÒÃÇ²»ÓÃ¼ÇhyjµÄ±àÂë£¬Ö±½ÓÊäÈëÃû³Æ¾ÍÐÐ 
*/
//°ÑÃ¿¸ö¶ÔÏóµÄ±àÂë×ª»¯³É×Ö·û 
typedef uint8_t ObTag;//Í³Ò»µÄtµÄÔËÓÃ
namespace obc{//object encode list ¶ÔÏó±àÂëÁÐ±í
	enum encode{
		backGround = 1,  // BGµÄ±ðÃû
		BG = 1,          // ±³¾°
		grass = 2,       // ²Ý·½¿é
		soil = 3,        // ÄàÍÁ
		portal = 4,      // ´«ËÍÃÅ·½¿é
		port = 4,        // portalµÄ±ðÃû
		player = 5,      // Íæ¼Ò
		win = 6,         // Ê¤ÀûÍ¼Æ¬
		stone = 7,       // Ô­Ê¯
		diamond = 8,     // ×êÊ¯¿ó
		dia = 8,         // diamondµÄ±ðÃû
		iron = 9,        // Ìú¿ó
		zombie = 10,     // ½©Ê¬
		comzom = 10,	 // ÆÕÍ¨½©Ê¬±ðÃû
		bedrock = 11,    // »ùÑÒ
		none = 12,       // ¿Õ°×
		blank = 12,      // ÕæÊµµÄ¿Õ°× 
		loss = 13,       // Ê§°Ü 
		heart = 14,      // ÐÄ
		vheart = 15,     // ¿ÕÐÄ
		bow = 16,        // ¹­
		arrow = 17,      // ¼ý 
		arr = 17,        // arrowµÄ±ðÃû
		chickenJockey=18,// ¼¦ÆïÊ¿
		chickJock=18,    // ¼ò³Æ
		CJ = 18,         // ¼ò³Æ*2
		drowned = 19,    // ÄçÊ¬
		drown = 19,      // ÄçÊ¬±ðÃû
		zhudating = 20,  // Ö÷´óÌü
		start = 21,      // ¿ªÊ¼°´Å¥ 
		wupinlan = 22,	 // ÎïÆ·À¸
		mujian = 23,	 // Ä¾½£ 
		mj = 23,	     // Ä¾½£
		mujianblack = 24,// Ä¾½£»ÒÉ«±³¾°
		mjb = 24,		 // Ä¾½£»ÒÉ«±³¾°
		wupinlanw = 25,  // °×¿òÎïÆ·À¸
		wplat = 25,      // °×¿òÎïÆ·À¸
		shijian = 26,    // Ê¯½£
		sj = 26,         // Ê¯½£ 
		shijianblack = 27,// Ê¯½£»ÒÉ«±³¾° 
		sjb = 27,         // Ê¯½£»ÒÉ«±³¾° 
		zhizhu = 28,     // Ö©Öë
		cz = 29,         // ÖØÖÃ 
		mg = 30,         // Ä¾¸å
		mgb = 31,		  // Ä¾¸å»ÒÉ«
		dia2 = 32,       // ×êÊ¯µÄµÚ¶þÖÖÐÎÌ¬
		dia3 = 33        // ×êÊ¯µôÂäÎïÐÎÌ¬ 
	};
}
/*
1:±³¾°.jpg
2:²Ý·½¿é.jpg
3:ÄàÍÁ.jpg
4:´«ËÍÃÅ·½¿é.jpg
5:Íæ¼Ò.jpg
6:Ê¤ÀûÍ¼Æ¬.jpg
7:Ô­Ê¯.jpg
8:×êÊ¯¿ó.jpg
9:Ìú¿ó.jpg
10:½©Ê¬.jpg
11:»ùÑÒ.jpg
12:¿Õ°×.jpg
13:Ê§°Ü.jpg
14:ÐÄ.jpg
15:¿ÕÐÄ.jpg
16:¹­.jpg
17:¼ý.jpg
18:¼¦ÆïÊ¿.jpg
19:ÄçÊ¬.jpg
*/
