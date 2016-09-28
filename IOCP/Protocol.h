#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////

#define		MAX_PACKET_SIZE			128
#define		SERVER_PORT				9000
#define		SERVERADDR				"192.168.183.192"
#define		MAX_ID_CNT				10
#define		MAX_PLAYER_CNT			2

#define		PAK_LOGIN				0
#define		PAK_REG					1
#define		PAK_RMV					2
#define		PAK_KEY_UP				3
#define		PAK_KEY_DOWN			4
#define		PAK_READY				5
#define		PAK_START				6	
#define		PAK_ENDING				7
#define		PAK_SYNC				8

//////////////////////////////////////////////////////////////////////////////////

// 단위는 밀리세컨드, FPS 62.5
#define FIXED_FRAME_TIME	16   // 최저프레임시간
#define MILISEC_PER_SYNC	35	// 한 싱크당 시간 

#pragma pack(push, 1)																				 
//////////////////////////////////////////////////////////////////////////////////

struct POINT3D {
	BYTE	ID;
	FLOAT	x, y, z;
};

struct HEADER
{
	BYTE			ucSize;
	BYTE			byPacketID;
};

//////////////////////////////////////////////////////////////////////////////////

struct SC_LOG_INOUT {
	HEADER header;
	BYTE ID;
	BYTE clientNum;
	BYTE readyCount;
};

struct SC_START {
	HEADER header;
	BYTE ID_LIST[MAX_PLAYER_CNT];
};

struct SC_KEY {
	HEADER header;
	BYTE key;
	BYTE ID;
};

struct SC_SYNC {
	HEADER header;
	POINT3D positions[MAX_PLAYER_CNT];
};

//////////////////////////////////////////////////////////////////////////////////

struct CS_KEY {
	HEADER header;
	BYTE key;
};

//////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)