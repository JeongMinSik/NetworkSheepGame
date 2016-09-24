#pragma once
#include "stdafx.h"

#define		MAX_PACKET_SIZE			256
#define		SERVER_PORT				9000
#define		MAX_ID_CNT				10

#define		PAK_SYNC				0
#define		PAK_ID					1
#define		PAK_REG					2
#define		PAK_RMV					3


#pragma pack(push, 1)

struct HEADER
{
	UCHAR			ucSize;
	BYTE			byPacketID;
};

struct CTOS_SYNC
{
	char data[MAX_PACKET_SIZE-3];
};

struct STOC_SYNC
{
	BYTE	ID;
	char data[MAX_PACKET_SIZE-3];
};

#pragma pack(pop)