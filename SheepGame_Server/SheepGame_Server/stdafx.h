#pragma once
#pragma comment(lib,"ws2_32.lib")

#include <WinSock2.h>
#include <stdio.h>
#include <vector>
#include <thread>
#include <fstream>

using namespace std;

#define		GROUND_NUM				10  // 바닥갯수
#define		ENDING_X				9500
#define		OB_CNT					320
#define		MOVING_OB_CNT			127

#define		GLUT_KEY_LEFT			100
#define		GLUT_KEY_UP				101
#define		GLUT_KEY_RIGHT			102
#define		GLUT_KEY_DOWN			103

enum { MAIN_MODE, READY_MODE, PLAY_MODE, PAUSE_MODE, GAME_OVER, ENDING_MODE }; // 게임상태
enum { DOWN_VIEW, FRONT_VIEW }; //카메라 시점
enum { STOP_STATE, LEFT_STATE, RIGHT_STATE, UP_STATE, DOWN_STATE, JUMP_UP_STATE, JUMP_DOWN_STATE, GRAVITY }; //이동 상태
enum { SHEEP, BOX, SCISSORS, PUMKIN, BRICK, HAY, BLACK_SHEEP, BOXWALL }; // 객체 타입