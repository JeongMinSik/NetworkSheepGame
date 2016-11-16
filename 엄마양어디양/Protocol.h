#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////

#define		MAX_PACKET_SIZE			2048
#define		SERVER_PORT				9000
#define		MAX_ID_CNT				100
#define		MAX_PLAYER_CNT			2
#define		MOVING_OB_CNT			127

#define		PAK_LOGIN				0
#define		PAK_REG					1
#define		PAK_RMV					2
#define		PAK_KEY_UP				3
#define		PAK_KEY_DOWN			4
#define		PAK_READY				5
#define		PAK_START				6	
#define		PAK_ENDING				7
#define		PAK_SYNC				8
#define		PAK_HURT				9

//////////////////////////////////////////////////////////////////////////////////

// 단위는 밀리세컨드, FPS 60
#define FIXED_FRAME_TIME			17		// 최저프레임시간
#define MILISEC_PER_SYNC			33		

#pragma pack(push, 1)																				 

struct POINT3D {
	FLOAT	x, y, z;
	POINT3D() = default;
	POINT3D(float x, float y, float z) : x(x), y(y), z(z) {};
	POINT3D& operator=(POINT3D& rPoint) {
		this->x = rPoint.x;
		this->y = rPoint.y;
		this->z = rPoint.z;
		return *this;
	}
};

struct HEADER
{
	SHORT			packetSize;
	BYTE			packetID;
};

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
	HEADER	header;
	BYTE	sheep_ID[MAX_PLAYER_CNT];
	POINT3D sheep_pos[MAX_PLAYER_CNT];
	POINT3D object_pos[MOVING_OB_CNT];
	BOOL	state[MAX_PLAYER_CNT][8];
};

struct SC_EVENT {
	HEADER header;
	BYTE ID;
};

//////////////////////////////////////////////////////////////////////////////////

struct CS_KEY {
	HEADER header;
	BYTE key;
};

//////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)