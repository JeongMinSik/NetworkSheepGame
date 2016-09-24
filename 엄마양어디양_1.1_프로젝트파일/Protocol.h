#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////

#define		MAX_PACKET_SIZE			1024
#define		SERVER_PORT				9000
#define		SERVERADDR				"127.0.0.1"
#define		MAX_ID_CNT				10
#define		MAX_READY_CNT			1

#define		PAK_LOGIN				0
#define		PAK_REG					1
#define		PAK_RMV					2
#define		PAK_KEY_UP				3
#define		PAK_KEY_DOWN			4
#define		PAK_READY				5
#define		PAK_START				6		

//////////////////////////////////////////////////////////////////////////////////

// ������ �и�������, FPS 62.5
#define FIXED_FRAME_TIME	16   // ���������ӽð�
#define MAX_FRAME_TIME		250  // ���� �� ���� ���� �ִ������ӽð�
#define DELTA_TIME			10.0	 // �ɰ��� ����� ����



#pragma pack(push, 1)																				 
//////////////////////////////////////////////////////////////////////////////////

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


struct SC_KEY {
	HEADER header;
	BYTE key;
	BYTE ID;
};

//////////////////////////////////////////////////////////////////////////////////

struct CS_KEY {
	HEADER header;
	BYTE key;
};

//////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)