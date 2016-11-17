#include "stdafx.h"
#include "Network.h"
#include "Objects.h"
#include "Ground.h"
#include "Camera.h"
#include "Sheep.h"
#include "SoundPackage.h"
#include "UI.h"
#include "Box.h"
#include "Black_Sheep.h"
#include "MotherSheep.h"
#include "Pumkin.h"
#include "Hay.h"
#include "Scissors.h"

// 함수선언
GLvoid updateScene(int);
GLvoid drawScene(GLvoid);
GLvoid Keyboard(unsigned char, int, int);
GLvoid SpecialKeyboard(int, int, int);
GLvoid SpecialKeyboardUp(int, int, int);
GLvoid Reshape(int, int);
GLubyte * LoadDIBitmap(const char *, BITMAPINFO **);
void CreateWorld();
void DestroyWorld();																							

//텍스쳐 관련 변수
GLubyte *pBytes = nullptr; // 데이터를 가리킬 포인터
BITMAPINFO *info; //비트맵 헤더 저장할 변수
GLuint textures[TEXTURES_COUNT]; //텍스쳐 배열

//사운드 관련 변수
SoundPackage *pSoundPackage;
FMOD_SYSTEM *g_System;
FMOD_SOUND *g_Sound[SOUND_COUNT];
FMOD_CHANNEL *g_Channel[SOUND_COUNT];

//객체 포인터 생성
CNetwork NetworkManager;
Ui* ui;
Camera *mainCamera;
int iCurCamera;
Sheep *mainSheep;
Ground* ground[GROUND_NUM];
Object* obstacles[OB_CNT];
MotherSheep* mother_sheep;
int ob_num = 0;
int Game_Mode = MAIN_MODE;

void SetSound()
{
	FMOD_System_Create(&g_System);
	FMOD_System_Init(g_System, SOUND_COUNT, FMOD_INIT_NORMAL, NULL);
	FMOD_System_CreateSound(g_System, "./SOUND/main_bgm.mp3", FMOD_LOOP_NORMAL, 0, &g_Sound[MAIN_BGM]);
	FMOD_System_CreateSound(g_System, "./SOUND/game_bgm.mp3", FMOD_LOOP_NORMAL, 0, &g_Sound[GAME_BGM]);
	FMOD_System_CreateSound(g_System, "./SOUND/clear_bgm.mp3", FMOD_LOOP_NORMAL, 0, &g_Sound[CLEAR_BGM]);
	FMOD_System_CreateSound(g_System, "./SOUND/jump.wav", FMOD_DEFAULT, 0, &g_Sound[JUMP_E]);
	FMOD_System_CreateSound(g_System, "./SOUND/kill.wav", FMOD_DEFAULT, 0, &g_Sound[KILL_E]);
	FMOD_System_CreateSound(g_System, "./SOUND/cry.mp3", FMOD_DEFAULT, 0, &g_Sound[CRY_E]);
	FMOD_System_CreateSound(g_System, "./SOUND/button_ok.wav", FMOD_DEFAULT, 0, &g_Sound[BUTTON_OK_E]);
	FMOD_System_CreateSound(g_System, "./SOUND/button_move.wav", FMOD_DEFAULT, 0, &g_Sound[BUTTON_MOVE_E]);
	FMOD_System_CreateSound(g_System, "./SOUND/get_hurt.mp3", FMOD_DEFAULT, 0, &g_Sound[GET_HURT_E]);
	FMOD_System_CreateSound(g_System, "./SOUND/camera.wav", FMOD_DEFAULT, 0, &g_Sound[CAMERA_E]);

	//메인배경음악 재생
	FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[MAIN_BGM], 0, &g_Channel[MAIN_BGM]);
	FMOD_Channel_SetVolume(g_Channel[MAIN_BGM], 1);

	pSoundPackage = new SoundPackage;
	pSoundPackage->System = g_System;
	pSoundPackage->Channel = g_Channel;
	pSoundPackage->Sound = g_Sound;

}
void SetTextures()
{

	//n개의 이미지 텍스처 매핑을 한다.
	glGenTextures(TEXTURES_COUNT - 1, textures);

	for (int i = 0; i < TEXTURES_COUNT; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		if (i == BOX_FRONT)
		{
			pBytes = LoadDIBitmap("./RESOURCE/box_top.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 500, 500, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == BOX_TOP)
		{
			pBytes = LoadDIBitmap("./RESOURCE/box_top.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 500, 500, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == GROUND_FRONT)
		{
			pBytes = LoadDIBitmap("./RESOURCE/ground_front(1500x100).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 1500, 100, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == GROUND_TOP)
		{
			pBytes = LoadDIBitmap("./RESOURCE/ground_top(1500x400).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 1500, 400, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == BACK_GROUND)
		{
			pBytes = LoadDIBitmap("./RESOURCE/background(1500x300).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 1500, 300, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == BRICK_FRONT)
		{
			pBytes = LoadDIBitmap("./RESOURCE/brick-front(500x250).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 500, 250, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == BRICK_TOP)
		{
			pBytes = LoadDIBitmap("./RESOURCE/brick-top(200x200).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 200, 200, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == HAY_FRONT)
		{
			pBytes = LoadDIBitmap("./RESOURCE/hay_cart.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 300, 200, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == HAY_TOP)
		{
			pBytes = LoadDIBitmap("./RESOURCE/hay_cart.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 300, 200, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == BOXWALL_TOP)
		{
			pBytes = LoadDIBitmap("./RESOURCE/boxwall_top(140X800).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 140, 800, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == BOXWALL_FRONT)
		{
			pBytes = LoadDIBitmap("./RESOURCE/boxwall_front(140X420).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 140, 420, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == ESC_BUTTON)
		{
			pBytes = LoadDIBitmap("./RESOURCE/esc(96x96).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 0, 0, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MENU_0)
		{
			pBytes = LoadDIBitmap("./RESOURCE/Menu_1(400x600).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 400, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MENU_1)
		{
			pBytes = LoadDIBitmap("./RESOURCE/Menu_2(400x600).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 400, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MENU_2)
		{
			pBytes = LoadDIBitmap("./RESOURCE/Menu_3(400x600).bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 400, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MAIN_0)
		{
			pBytes = LoadDIBitmap("./RESOURCE/MainScreen.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MAIN_1)
		{
			pBytes = LoadDIBitmap("./RESOURCE/MainScreen_1.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MAIN_2)
		{
			pBytes = LoadDIBitmap("./RESOURCE/MainScreen_2.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == MAIN_3)
		{
			pBytes = LoadDIBitmap("./RESOURCE/MainScreen_3.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == DEAD_0)
		{
			pBytes = LoadDIBitmap("./RESOURCE/DeadMenu1.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 400, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == DEAD_1)
		{
			pBytes = LoadDIBitmap("./RESOURCE/DeadMenu2.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 400, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == HELP_0)
		{
			pBytes = LoadDIBitmap("./RESOURCE/HELP_0.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == HELP_1)
		{
			pBytes = LoadDIBitmap("./RESOURCE/HELP_1.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == ENDING_0)
		{
			pBytes = LoadDIBitmap("./RESOURCE/Ending_0.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == ENDING_1)
		{
			pBytes = LoadDIBitmap("./RESOURCE/Ending_1.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == ENDING_MENU_0)
		{
			pBytes = LoadDIBitmap("./RESOURCE/endingmenu_0.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		else if (i == ENDING_MENU_1)
		{
			pBytes = LoadDIBitmap("./RESOURCE/endingmenu_1.bmp", &info);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 800, 600, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}
		/*(3)
		else if (i == 텍스쳐종류)
		{
		pBytes = LoadDIBitmap("이미지파일.bmp", &info);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 가로크기, 세로크기, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
		}

		(4) 해당 draw에서 추가한다.
		*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_MODULATE);
	}
}
void CreateWorld()
{
	NetworkManager.initGameMode(&Game_Mode);
	mainSheep = NetworkManager.m_Players[0].m_pSheep;
	mainCamera = NetworkManager.m_Players[0].m_pSheep->pCamera;
	iCurCamera = 0;
	// 기본 객체

	ground[0] = new Ground(-200, -100, -100); // z축 -100~300
	ground[0]->pTextures = textures;
	for (int i = 1; i < GROUND_NUM; ++i)
	{
		ground[i] = new Ground(ground[0]->x + ground[0]->width*i, ground[0]->y, ground[0]->z);
		ground[i]->pTextures = textures;
	}

	mother_sheep = new MotherSheep();

	// 파일 입력
	ob_num = 0;
	std::ifstream fin;
	fin.open("DATA.txt");
	while (!fin.eof())
	{
		int object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz;
		fin >> object_type >> xx >> yy >> zz >> sspeed >> max_xx >> max_yy >> max_zz;
		switch (object_type)
		{
		case BOX:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case BRICK:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			NetworkManager.m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		case BOXWALL:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case SCISSORS:
			obstacles[ob_num] = new Scissors(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			NetworkManager.m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		case PUMKIN:
			obstacles[ob_num] = new Pumkin(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			NetworkManager.m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		case HAY:
			obstacles[ob_num] = new Hay(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case BLACK_SHEEP:
			obstacles[ob_num] = new Black_Sheep(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			NetworkManager.m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		}
		obstacles[ob_num]->pTextures = textures;
		obstacles[ob_num]->pCamera = mainCamera;
		++ob_num;
	}
	fin.close();

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		NetworkManager.m_Players[i].m_pSheep->setSound(pSoundPackage);
		NetworkManager.m_Players[i].m_pSheep->obCnt = ob_num;
		NetworkManager.m_Players[i].m_pSheep->pSelectedSheep = mainSheep;
		NetworkManager.m_Players[i].m_pSheep->pCamera->pSelectedCamera = mainCamera;
	}


	/*
	객체배열 파일출력
	ofstream fout;
	fout.open("OUTPUT.txt");
	for (int i = 0; i < ob_num; ++i)
	{
	fout << obstacles[i]->type; << " " << obstacles[i]->x << " " << obstacles[i]->y << " " << obstacles[i]->z << " "
	<< obstacles[i]->speed << " " << obstacles[i]->max_x << " " << obstacles[i]->max_y << " " << obstacles[i]->max_z << endl;
	}
	fout.close();
	*/

}
void DestroyWorld() {

	for (auto g : ground)
		delete g;
	delete mother_sheep;
	for (int i = 0; i < ob_num; ++i)
		delete obstacles[i];
	ob_num = 0;
	NetworkManager.m_vpMovingObject.clear();
}
void Program_Exit()
{
	delete ui;
	delete pSoundPackage;

	for (int i = 0; i < SOUND_COUNT; ++i)
	{
		FMOD_Sound_Release(g_Sound[i]);
	}
	FMOD_System_Close(g_System);
	FMOD_System_Release(g_System);

	if (ob_num)
	{
		for (auto g : ground)
			delete g;
		for (int i = 0; i < ob_num; ++i)
			delete obstacles[i];
	}
}

void main()
{
	NetworkManager.connectServer();
	NetworkManager.initGameMode(&Game_Mode);

	atexit(Program_Exit);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(300, 150);
	glutInitWindowSize(800, 600);
	glutCreateWindow("엄마양어디양");

	// 텍스쳐 설정
	SetTextures();
	// 음악설정
	SetSound();
	// ui객체 생성
	ui = new Ui(200);
	ui->pTextures = textures;
	ui->pGameMode = &Game_Mode;
	ui->pSound = pSoundPackage;
	// 커서 가리기
	//ShowCursor(false);

	glutSpecialFunc(SpecialKeyboard);
	glutSpecialUpFunc(SpecialKeyboardUp);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(FIXED_FRAME_TIME, updateScene, 1);
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

float currentTime = clock();
float accumulator = 0.0f;
GLvoid updateScene(int value)
{
	FMOD_System_Update(g_System);

	float frameTime = clock() - currentTime;
	currentTime = clock();
	accumulator += frameTime;
	//printf("FPS:%f \n", 1000.0 / frameTime);

	while (accumulator >= FIXED_FRAME_TIME)
	{
		frameTime = FIXED_FRAME_TIME;
		ui->update(frameTime);

		switch (Game_Mode)
		{
		case PLAY_MODE:
		{
			Sheep** sheeps = new Sheep*[MAX_PLAYER_CNT];
			// 각각의 양에 대한 카메라, 스탠딩 업데이트
			for (int j = 0; j < MAX_PLAYER_CNT; ++j) {
				//카메라 업데이트
				auto sheep = NetworkManager.m_Players[j].m_pSheep;
				sheeps[j] = sheep;

				if (!sheep->killed) {
					sheep->pCamera->update(frameTime);
				}

				//if(!mainSheep->killed)
				//	mainCamera->update();

				//객체 업데이트 (+스탠딩 상태 확인)
				sheep->stading_index = -1;
				for (int i = 0; i < ob_num; ++i) {
					if (sheep->stading_index == -1 && obstacles[i]->is_standing(sheep)) {
						sheep->stading_index = i;
					}
				}
			}

			// 장애물 업데이트
			for (int i = 0; i < ob_num; ++i) {
				if (obstacles[i]->type == BLACK_SHEEP) {
					obstacles[i]->update3(sheeps, obstacles, frameTime);
				}
				else
					obstacles[i]->update1(sheeps, frameTime);
			};

			//양 업데이트
			for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
				switch (sheeps[i]->iGameMode) {
				case PLAY_MODE:
					sheeps[i]->update2(ground[0], obstacles, frameTime);
					break;
				case GAME_OVER:
					sheeps[i]->dead_update(frameTime);
					break;
				case ENDING_MODE:
					if (i == 0) {
						ui->ending_screen = WIN;
					}
					else {
						ui->ending_screen = LOSE;
					}
					FMOD_Channel_Stop(pSoundPackage->Channel[GAME_BGM]);
					FMOD_System_PlaySound(pSoundPackage->System, FMOD_CHANNEL_FREE, pSoundPackage->Sound[CLEAR_BGM], 0, &pSoundPackage->Channel[CLEAR_BGM]);
					Game_Mode = ENDING_MODE;
					mainSheep = NetworkManager.m_Players[i].m_pSheep;
					for (int j = 0; j < MAX_PLAYER_CNT; ++j) {
						if (i == j) continue;
						NetworkManager.m_Players[j].m_pSheep->iGameMode = GAME_OVER;
						mainCamera = mainSheep->pCamera;
					}
					break;
				}
			}
			delete[] sheeps;
			break;
		}
		case ENDING_MODE:
			mainSheep->ending_update(frameTime);
			break;
		}

		accumulator -= FIXED_FRAME_TIME;
	}
	glutPostRedisplay();
	glutTimerFunc(FIXED_FRAME_TIME, updateScene, 1);

}

GLvoid drawScene(GLvoid)
{
	glClearColor(0, 0.8f, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();

	// UI
	ui->draw(mainSheep);

	if (Game_Mode != MAIN_MODE)
	{
		////카메라 설정
		mainCamera->setting();

		//바닥
		for (auto g : ground){
			g->draw();
		}
		//양
		//mainSheep->draw();
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			NetworkManager.m_Players[i].m_pSheep->draw();
		}
		//바운딩박스(양)
		//sheep->test_draw();
		//printf("x:%d, y:%d, z:%d \n", sheep->x, sheep->y, sheep->z);

		//객체
		for (int i = 0; i < ob_num; ++i)
		{
			obstacles[i]->draw();
			//obstacles[i]->test_draw(); //바운딩박스
		}

		mother_sheep->draw();
	}
	glPopMatrix();
	glutSwapBuffers();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	if (Game_Mode == PLAY_MODE){

		if (key == ' ') {
			if (mainSheep->iGameMode == PLAY_MODE) {
				mainCamera->keyboard(key);
				NetworkManager.keyDown(key);
			}
			else if (mainSheep->killed){
				// 카메라 시점 변환
				iCurCamera = (iCurCamera + 1) % MAX_PLAYER_CNT;
				Sheep *pCurSheep = NetworkManager.m_Players[iCurCamera].m_pSheep;
				mainCamera = pCurSheep->pCamera;
				for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
					NetworkManager.m_Players[i].m_pSheep->pSelectedSheep = pCurSheep;
					NetworkManager.m_Players[i].m_pSheep->pCamera->pSelectedCamera = mainCamera;
				}
			}
		}
	}
	int retval = ui->keyboard(key);

	switch (retval) {
	case READY_MODE:
		CreateWorld();
		NetworkManager.getReady();
		break;
	case PLAY_MODE:
		// 다시시작
		DestroyWorld();
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			NetworkManager.m_Players[i].init();
		}
		CreateWorld();
		NetworkManager.getReady();
		break;
	case MAIN_MODE:
		DestroyWorld();
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			NetworkManager.m_Players[i].init();
		}
		break;
	}
}

GLvoid SpecialKeyboard(int key, int x, int y)
{
	ui->special_key(key);
	if (Game_Mode == PLAY_MODE && mainSheep->iGameMode == PLAY_MODE) {
		if ((key == GLUT_KEY_RIGHT && !mainSheep->state[RIGHT_STATE]) || 
			(key == GLUT_KEY_LEFT && !mainSheep->state[LEFT_STATE]) ||
			(key == GLUT_KEY_UP && (!mainSheep->state[UP_STATE] && mainSheep->pCamera->view_point == DOWN_VIEW)) ||
			(key == GLUT_KEY_UP && (!mainSheep->state[JUMP_UP_STATE] && mainSheep->pCamera->view_point == FRONT_VIEW)) ||
			(key == GLUT_KEY_DOWN && (!mainSheep->state[DOWN_STATE] && mainSheep->pCamera->view_point == DOWN_VIEW))){
			mainSheep->special_key(key);
			NetworkManager.keyDown(key);
		}
	}
}

GLvoid SpecialKeyboardUp(int key, int x, int y)
{
	if (Game_Mode == PLAY_MODE && mainSheep->iGameMode == PLAY_MODE) {
		if (key == GLUT_KEY_RIGHT || GLUT_KEY_LEFT || GLUT_KEY_UP || GLUT_KEY_DOWN) {
			mainSheep->special_key_up(key);
			NetworkManager.keyUp(key);
		}
	}
}

GLvoid Reshape(int w, int h)
{
	//뷰포트 변환
	glViewport(0, 0, w, h);

	//투영 변환
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60.0, w / h, 10.0, 1000.0);
	//glOrtho(-100, 100, -100, 100, 1000, -1000);

	//모델링 변환
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLubyte *LoadDIBitmap(const char *filename, BITMAPINFO **info)
{
	FILE *fp;
	GLubyte *bits;
	int bitsize, infosize;
	BITMAPFILEHEADER header;
	// 바이너리 읽기 모드로 파일을 연다
	fopen_s( &fp, filename, "rb" );
	if ( fp == NULL)
		return NULL;
	// 비트맵 파일 헤더를 읽는다.
	if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1) {
		fclose(fp);
		return NULL;
	}
	// 파일이 BMP 파일인지 확인핚다.
	if (header.bfType != 'MB') {
		fclose(fp);
		return NULL;
	}
	// BITMAPINFOHEADER 위치로 갂다.
	infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
	// 비트맵 이미지 데이터를 넣을 메모리 핛당을 핚다.
	if ((*info = (BITMAPINFO *)malloc(infosize)) == NULL) {
		fclose(fp);
		exit(0);
		return NULL;
	}
	// 비트맵 인포 헤더를 읽는다.
	if (fread(*info, 1, infosize, fp) < (unsigned int)infosize) {
		free(*info);
		fclose(fp);
		return NULL;
	}
	// 비트맵의 크기 설정
	if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
		bitsize = ((*info)->bmiHeader.biWidth *
			(*info)->bmiHeader.biBitCount + 7) / 8.0 *
		abs((*info)->bmiHeader.biHeight);
	// 비트맵의 크기만큼 메모리를 핛당핚다.
	if ((bits = (unsigned char *)malloc(bitsize)) == NULL) {
		free(*info);
		fclose(fp);
		return NULL;
	}
	// 비트맵 데이터를 bit(GLubyte 타입)에 저장핚다.
	if (fread(bits, 1, bitsize, fp) < (unsigned int)bitsize) {
		free(*info); free(bits);
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	return bits;
}
