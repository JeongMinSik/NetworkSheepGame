#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////

#define		MAX_PACKET_SIZE			1024
#define		SERVER_PORT				9000
#define		SERVERADDR				"127.0.0.1"
#define		MAX_ID_CNT				10
#define		MAX_PLAYER_CNT			1

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

// ������ �и�������, FPS 62.5
#define FIXED_FRAME_TIME	16  // ���������ӽð�
#define MILISEC_PER_SYNC	100	// �� ��ũ�� �ð�


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