#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////

#define		MAX_PACKET_SIZE			1024
#define		SERVER_PORT				9000
#define		SERVERADDR				"127.0.0.1"
#define		MAX_ID_CNT				10

#define		PAK_LOGIN				0
#define		PAK_REG					1
#define		PAK_RMV					2
#define		PAK_KEY_UP				3
#define		PAK_KEY_DOWN			4
#define		PAK_READY				5

//////////////////////////////////////////////////////////////////////////////////

// ������ �и�������, FPS 62.5
#define FIXED_FRAME_TIME	16   // ���������ӽð�
#define MAX_FRAME_TIME		250  // ���� �� ���� ���� �ִ������ӽð�
#define DELTA_TIME			10.0	 // �ɰ��� ����� ����

enum { MAIN_MODE, READY_MODE, PLAY_MODE, PAUSE_MODE, GAME_OVER, ENDING_MODE }; // ���ӻ���
enum { DOWN_VIEW, FRONT_VIEW }; //ī�޶� ����
enum { STOP_STATE, LEFT_STATE, RIGHT_STATE, UP_STATE, DOWN_STATE, JUMP_UP_STATE, JUMP_DOWN_STATE, GRAVITY }; //�̵� ����
enum { SHEEP, BOX, SCISSORS, PUMKIN, BRICK, HAY, BLACK_SHEEP, BOXWALL }; // ��ü Ÿ��
enum { BOX_FRONT, BOX_TOP, GROUND_FRONT, GROUND_TOP, BACK_GROUND, BRICK_FRONT, BRICK_TOP, HAY_FRONT, HAY_TOP, BOXWALL_FRONT, BOXWALL_TOP, ESC_BUTTON, MENU_0, MENU_1, MENU_2, MAIN_0, MAIN_1, MAIN_2, MAIN_3, DEAD_0, DEAD_1, HELP_0, HELP_1, ENDING_0, ENDING_1, ENDING_MENU_0, ENDING_MENU_1 }; // �ؽ��� ���� (1)
enum { MAIN_BGM, GAME_BGM, JUMP_E, KILL_E, CRY_E, BUTTON_MOVE_E, BUTTON_OK_E, GET_HURT_E, CAMERA_E, CLEAR_BGM }; // ����


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


struct SC_SYNC {
	HEADER header;
	BYTE ID;
	CHAR key;
};

//////////////////////////////////////////////////////////////////////////////////

struct CS_SYNC {
	HEADER header;
	CHAR KEY;
};

//////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)