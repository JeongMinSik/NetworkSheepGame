#pragma once
#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib, "fmodex_vc.lib")

#include "inc/fmod.hpp"
#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <gl/glut.h>
#include <ctime>
#include <vector>
#include <mutex>

#include "Protocol.h"
using namespace FMOD;
using namespace std;

#define		TEXTURES_COUNT			27 // 텍스쳐 수 (2)
#define		SOUND_COUNT				11
#define		GAME_BGM_VOLUME			0.4
#define		GROUND_NUM				10  // 바닥갯수
#define		ENDING_X				9500
#define		OB_CNT					320

enum { MAIN_MODE, READY_MODE, PLAY_MODE, PAUSE_MODE, GAME_OVER, ENDING_MODE }; // 게임상태
enum { DOWN_VIEW, FRONT_VIEW }; //카메라 시점
enum { STOP_STATE, LEFT_STATE, RIGHT_STATE, UP_STATE, DOWN_STATE, JUMP_UP_STATE, JUMP_DOWN_STATE, GRAVITY }; //이동 상태
enum { SHEEP, BOX, SCISSORS, PUMKIN, BRICK, HAY, BLACK_SHEEP, BOXWALL }; // 객체 타입
enum { BOX_FRONT, BOX_TOP, GROUND_FRONT, GROUND_TOP, BACK_GROUND, BRICK_FRONT, BRICK_TOP, HAY_FRONT, HAY_TOP, BOXWALL_FRONT, BOXWALL_TOP, ESC_BUTTON, MENU_0, MENU_1, MENU_2, MAIN_0, MAIN_1, MAIN_2, MAIN_3, DEAD_0, DEAD_1, HELP_0, HELP_1, ENDING_0, ENDING_1, ENDING_MENU_0, ENDING_MENU_1 }; // 텍스쳐 종류 (1)
enum { MAIN_BGM, GAME_BGM, JUMP_E, KILL_E, CRY_E, BUTTON_MOVE_E, BUTTON_OK_E, GET_HURT_E, CAMERA_E, CLEAR_BGM }; // 사운드
