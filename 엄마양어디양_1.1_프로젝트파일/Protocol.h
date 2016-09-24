#pragma once
#include "stdafx.h"

#define		MAX_PACKET_SIZE			1024
#define		SERVER_PORT				9000
#define		SERVERADDR				"127.0.0.1"
#define		MAX_ID_CNT				10

#define		PAK_LOGIN				0
#define		PAK_REG					1
#define		PAK_RMV					2
#define		PAK_KEY_UP				3
#define		PAK_KEY_DOWN			4


#pragma pack(push, 1)

struct HEADER
{
	BYTE			ucSize;
	BYTE			byPacketID;
};

//////////////////////////////////////////////////////////////////////////////////

struct SC_LOG_INOUT {
	HEADER header;
	BYTE ID;
};
struct SC_SYNC {
	HEADER header;
	BYTE ID;
	CHAR KEY;
};

//////////////////////////////////////////////////////////////////////////////////

struct CS_SYNC {
	HEADER header;
	CHAR KEY;
};

//////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)