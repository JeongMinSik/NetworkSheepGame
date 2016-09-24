#include "Network.h"
#include "stdafx.h"

CNetwork NetworkManager;


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
#define TEXTURES_COUNT 27 // 텍스쳐 수 (2)
GLuint textures[TEXTURES_COUNT]; //텍스쳐 배열

//사운드 관련 변수
#define SOUND_COUNT 11
FMOD_SYSTEM *g_System;
FMOD_SOUND *g_Sound[SOUND_COUNT];
FMOD_CHANNEL *g_Channel[SOUND_COUNT];
#define GAME_BGM_VOLUME 0.4

struct Camera;  struct Sheep; struct Object; struct Ground; struct Ui; struct MotherSheep;
//객체 포인터 생성
Ui* ui;
Camera* camera;
#define GROUND_NUM 10  // 바닥갯수
Ground* ground[GROUND_NUM];
Sheep* sheep;
Object* obstacles[500];
MotherSheep* mother_sheep;
int ob_num = 0;
int Game_Mode = MAIN_MODE;
#define ENDING_X 9500


struct Camera {
	float x, y, z;
	int canvas_size;
	float view_radius;
	bool view_point;
	bool is_changing;
	Camera()
	{
		x = 0; y = 100;
		canvas_size = 200;
		view_radius = 0;
		view_point = FRONT_VIEW;
		is_changing = false;
	}
	~Camera() {  }
	void setting()
	{
		glOrtho(-canvas_size, canvas_size, -canvas_size, canvas_size, 1000, -1000);

		gluLookAt(x, y, 0, x, y, -1, 0.0, 1.0, 0.0);
		glRotated(view_radius, 1, 0, 0);
	}
	void keyboard(unsigned char key)
	{
		if (key == ' ' && is_changing == false)
		{
			FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[CAMERA_E], 0, &g_Channel[CAMERA_E]);
			is_changing = true;
		}
	}
	void update()
	{
		// 카메라 회전
		if (is_changing)
		{
			if (view_point == FRONT_VIEW)
			{
				view_radius -= 0.2f * DELTA_TIME;
				if (view_radius <= -90)
				{
					is_changing = false;
					view_radius = -90;
					view_point = DOWN_VIEW;
				}
			}
			else if (view_point == DOWN_VIEW)
			{
				view_radius += 0.2f * DELTA_TIME;
				if (view_radius >= 0)
				{
					is_changing = false;
					view_radius = 0;
					view_point = FRONT_VIEW;
				}
			}
		}
	}

};
struct Ground {
	float x, y, z;
	float width, height, depth;
	int back_height;
	Ground(float x, float y, float z) :x(x), y(y), z(z), width(1000), height(100), depth(400) {
		back_height = 300;
	}
	~Ground() { }
	void draw() {
		glColor3f(1, 1, 1);

		glEnable(GL_TEXTURE_2D);

		//뒷배경
		glBindTexture(GL_TEXTURE_2D, textures[BACK_GROUND]);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y + height, z + depth);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y + height, z + depth);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height + back_height, z + depth);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height + back_height, z + depth);
		glEnd();

		//앞면
		glBindTexture(GL_TEXTURE_2D, textures[GROUND_FRONT]);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z);
		glEnd();

		//윗면
		glBindTexture(GL_TEXTURE_2D, textures[GROUND_TOP]);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y + height, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z + depth);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z + depth);
		glEnd();

		glDisable(GL_TEXTURE_2D);

		//바닥면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x, y, z + depth);
		glVertex3f(x + width, y, z + depth);
		glVertex3f(x + width, y, z);
		glEnd();

		//왼쪽면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x, y + height, z);
		glVertex3f(x, y + height, z + depth);
		glVertex3f(x, y, z + depth);
		glEnd();
		//오른쪽면
		glBegin(GL_QUADS);
		glVertex3f(x + width, y, z);
		glVertex3f(x + width, y, z + depth);
		glVertex3f(x + width, y + height, z + depth);
		glVertex3f(x + width, y + height, z);
		glEnd();
		//뒷면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z + depth);
		glVertex3f(x, y + height, z + depth);
		glVertex3f(x + width, y + height, z + depth);
		glVertex3f(x + width, y, z + depth);
		glEnd();
	}
};
struct Object {
	float x, y, z;
	float width, height, depth;
	float speed;
	float max_x, max_y, max_z;
	int state_x, state_y, state_z;
	int type;
	bool killed = false;

	Object(int type, float x, float y, float z, float w, float h, float d, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : x(x), y(y), z(z), width(w), height(h), depth(d), type(type), speed(sp*DELTA_TIME*0.03), max_x(m_x), max_y(m_y), max_z(m_z) {}
	virtual ~Object() {  }
	virtual void draw() = 0;
	virtual void update(Sheep*) {};
	virtual void update(Sheep*, Object*[]) { };
	virtual bool is_standing(const Object*) { return false; };
	virtual bool is_inside(Sheep*) { return false; };
	virtual bool AABB_surface(const Object*) { return false; };
	bool AABB(const Object* other)
	{
		if (x + width <= other->x) return false;
		if (x >= other->x + other->width) return false;
		if (y + height <= other->y) return false;
		if (y >= other->y + other->height) return false;
		if (z + depth <= other->z) return false;
		if (z >= other->z + other->depth) return false;
		return true;
	}
	void test_draw()
	{
		glPushMatrix();
		glColor3f(1, 0, 0);

		int yy = y;
		for (int i = 0; i < 2; ++i)
		{
			if (i == 1) yy = y + height;
			glPushMatrix();
			glTranslated(x, yy, z);
			glutSolidSphere(2, 10, 10);
			glPopMatrix();
			glPushMatrix();
			glTranslated(x + width, yy, z);
			glutSolidSphere(2, 10, 10);
			glPopMatrix();
			glPushMatrix();
			glTranslated(x, yy, z + depth);
			glutSolidSphere(2, 10, 10);
			glPopMatrix();
			glPushMatrix();
			glTranslated(x + width, yy, z + depth);
			glutSolidSphere(2, 10, 10);
			glPopMatrix();
		}


		glPopMatrix();
	}
};
struct Sheep : public Object {
	bool state[8]; // 이동상태
	float jump_height;		// 점프력
	float minus_height; //감소 점프력
	float x_additional_speed, y_additional_speed, z_additional_speed;	// 추가 이동속도
	float org_y; // 점프시 y 위치
	int last_view; //마지막 바라본 방향(각도)
	int life; //목숨
	bool is_invincible; // 무적상태
	bool is_under; // 눌린 상태
	bool is_in_hay; // 지푸라기 안
	int max_invicible_time;  // 최대 무적시간 (반드시 짝수로만 설정)
	int cur_invicible_time;  // 현재 무적시간
	int stading_index; // 바닥객체 인덱스
	bool ending_finished = false;
	Sheep(int t, int x, int y, int z, float sp) : Object(t, x, y, z, 45, 30, 35, sp) {
		jump_height = 100;
		minus_height = 0;
		x_additional_speed = y_additional_speed = z_additional_speed = 0;
		last_view = 0;
		life = 3;
		is_invincible = is_under = is_in_hay = false;
		max_invicible_time = 2000;
		cur_invicible_time = 0;
		stading_index = -1;
		for (int i = 0; i < 8; ++i)
			state[i] = false;
	}
	~Sheep() {  }
	void get_hurt()
	{
		sheep->is_invincible = true;
		--sheep->life;
		if (sheep->life < 1)
		{
			Game_Mode = GAME_OVER;
			FMOD_Channel_Stop(g_Channel[GAME_BGM]);
			FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[CRY_E], 0, &g_Channel[CRY_E]);
		}
		else
		{
			FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[GET_HURT_E], 0, &g_Channel[GET_HURT_E]);
		}
	}
	void dead_update()
	{
		camera->is_changing = false;

		//죽은 양
		if (camera->view_radius != -40)
			(camera->view_radius < -40) ? camera->view_radius += 0.25*DELTA_TIME : camera->view_radius -= 0.25*DELTA_TIME;
		else if (y <= 500)
		{
			y += 0.5f*DELTA_TIME;

			if (y > 500)
			{
				sheep->killed = true;
			}
		}
	}
	void ending_update()
	{

		camera->is_changing = false;

		static int dir = 1, jump_cnt;
		static bool sound = true;
		const int JUMP_MAX = 3;

		// 카메라이동
		if (camera->view_radius != -20)
		{
			(camera->view_radius < -20) ? camera->view_radius += 0.5*DELTA_TIME : camera->view_radius -= 0.5*DELTA_TIME;
			dir = 1, jump_cnt = 0;
			sound = true;
		}

		// 이동
		int aim_x = 9600, aim_z = 78;
		int vx, vz; // 이동량
		float d = sqrt(float(aim_x - x)*float(aim_x - x) + float(aim_z - z)*float(aim_z - z));

		last_view = -atan2(aim_z - z, aim_x - x) * 180 / 3.1415926535;

		if (d > speed)
		{
			vx = (aim_x - x) / d*speed;
			vz = (aim_z - z) / d*speed;
			y -= speed;
			if (y < 0) { y = 0; }
		}
		else
		{
			vx = 0;
			vz = 0;
			last_view = 0;
		}
		x += vx;
		camera->x += vx;
		z += vz;

		if (vx == 0 && vz == 0 && jump_cnt < JUMP_MAX)
		{
			if (sound)
			{
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[JUMP_E], 0, &g_Channel[JUMP_E]);
				sound = false;
			}

			y += speed*dir;
			if (y > jump_height)
				dir = -1;
			if (y < 0)
			{
				sound = true;
				y = 0;
				dir = 1;
				++jump_cnt;
			}
		}
		else if (jump_cnt == JUMP_MAX) { ending_finished = true; }

	}
	virtual void draw() override final
	{
		glPushMatrix();
		glTranslated((x + width / 2), (y + height / 2) + 5, z + depth / 2);
		if (last_view == 0)
		{
			glTranslated(-5, 0, 0);
		}
		else if (last_view == 180)
		{
			glTranslated(5, 0, 0);
		}
		else if (last_view == 90)
		{
			glTranslated(0, 0, 6);
		}
		else if (last_view == 270)
		{
			glTranslated(0, 0, -6);
		}
		glRotated(last_view, 0, 1, 0);
		glScalef(0.25, 0.3, 0.3);

		//호박에 깔림
		if (is_under)
		{
			glScalef(1, 0.5, 1);
			glTranslated(0, -60, 0);
		}

		glPushMatrix();
		glColor3f(1, 0.9, 0.9);
		glTranslatef(70, -20, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glColor3f(1, 0.8, 0.8);
		glTranslatef(10, -20, 0);
		glScalef(2, 1, 1);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		// 몸통이
		glPushMatrix();
		glColor3f(1, 1, 1);

		//무적애니메이션
		if (is_invincible) {
			if (cur_invicible_time % 20 == 0) {
				glColor3f(1, 0, 0);
			}
			else {
				glColor3f(1, 1, 1);
			}
		}

		glTranslatef(30, 0, 30);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-50, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-20, -10, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(-10, -20, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(30, -10, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(40, 0, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(30, 0, -30);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-50, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-20, -10, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(10, -20, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(30, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(40, 0, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-40, -20, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(30, 10, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-50, -30, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		// 눈
		glPushMatrix();
		if (is_invincible)
		{
			glPushMatrix();
			glColor3f(0, 0, 0);
			glRotated(-3, 0, 1, 0);
			glTranslatef(90, 0, 35);
			glRotated(45, 0, 0, 1);
			glScaled(1, 4, 1);
			glutSolidCube(5);
			glPopMatrix();

			glPushMatrix();
			glColor3f(0, 0, 0);
			glRotated(-3, 0, 1, 0);
			glTranslatef(90, 0, 38);
			glRotated(-45, 0, 0, 1);
			glScaled(1, 4, 1);
			glutSolidCube(5);
			glPopMatrix();
			glTranslated(0, 0, -85);
			glPushMatrix();
			glColor3f(0, 0, 0);
			glRotated(-3, 0, 1, 0);
			glTranslatef(90, 0, 35);
			glRotated(45, 0, 0, 1);
			glScaled(1, 4, 1);
			glutSolidCube(5);
			glPopMatrix();

			glPushMatrix();
			glColor3f(0, 0, 0);
			glRotated(-3, 0, 1, 0);
			glTranslatef(90, 0, 38);
			glRotated(-45, 0, 0, 1);
			glScaled(1, 4, 1);
			glutSolidCube(5);
			glPopMatrix();
		}
		else
		{
			glColor3f(0, 0, 0);
			glTranslatef(90, 0, 35);
			glutSolidSphere(7, 16, 16);
			glTranslatef(0, 0, -70);
			glutSolidSphere(7, 16, 16);
		}
		glPopMatrix();

		//천사링
		if (Game_Mode == GAME_OVER)
		{
			glPushMatrix();
			glColor3f(0.97, 0.97, 0.97);
			glTranslatef(50, 100, 0);
			glRotated(90, 1, 0, 0);
			glutSolidTorus(10, 40, 20, 20);
			glPopMatrix();
		}

		glPopMatrix();
	}
	virtual void update(const Ground* ground, Object* obstacles[])
	{
		//엔딩
		if (Game_Mode != ENDING_MODE && x > ENDING_X)
		{
			Game_Mode = ENDING_MODE;
			FMOD_Channel_Stop(g_Channel[GAME_BGM]);
			FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[CLEAR_BGM], 0, &g_Channel[CLEAR_BGM]);
		}

		// 바라보는 방향
		if (state[RIGHT_STATE])
		{
			last_view = 0;
			if (state[UP_STATE]) last_view -= 45;
			if (state[DOWN_STATE]) last_view += 45;
		}
		else if (state[LEFT_STATE])
		{
			last_view = 180;
			if (state[UP_STATE]) last_view += 45;
			if (state[DOWN_STATE]) last_view -= 45;
		}
		else if (state[DOWN_STATE])
			last_view = 90;
		else if (state[UP_STATE])
			last_view = 270;

		//지푸라기 안쪽 상태
		for (int i = 0; i < ob_num; ++i)
		{
			if (obstacles[i]->type == HAY && obstacles[i]->is_inside(this))
			{
				sheep->is_in_hay = true;
				break;
			}
			sheep->is_in_hay = false;
		}

		//무적상태
		if (is_invincible) {
			(cur_invicible_time % 20) ? camera->canvas_size += 0.4*DELTA_TIME : camera->canvas_size -= 0.4 * DELTA_TIME;
			cur_invicible_time += DELTA_TIME;
			if (cur_invicible_time >= max_invicible_time) {
				is_invincible = false;
				is_under = false;
				cur_invicible_time = 0;
			}
		}

		//스탠딩 상태
		if (stading_index >= 0) {

			// 추가 이동속도 변경
			if (obstacles[stading_index]->state_y == JUMP_UP_STATE)
				y_additional_speed = obstacles[stading_index]->speed;
			else if (obstacles[stading_index]->state_y == JUMP_DOWN_STATE)
				minus_height = jump_height *0.5;
			if (obstacles[stading_index]->state_x == RIGHT_STATE && state[LEFT_STATE])
				x_additional_speed = obstacles[stading_index]->speed;
			else if (obstacles[stading_index]->state_x == LEFT_STATE && state[RIGHT_STATE])
				x_additional_speed = obstacles[stading_index]->speed;
			else
				x_additional_speed = 0;
			if (obstacles[stading_index]->state_z == UP_STATE && state[DOWN_STATE])
				z_additional_speed = obstacles[stading_index]->speed;
			else if (obstacles[stading_index]->state_z == DOWN_STATE && state[UP_STATE])
				z_additional_speed = obstacles[stading_index]->speed;
			else
				z_additional_speed = 0;

			//이동
			if (obstacles[stading_index]->state_x == RIGHT_STATE)
			{
				x += obstacles[stading_index]->speed;
				camera->x += obstacles[stading_index]->speed;
			}
			else if (obstacles[stading_index]->state_x == LEFT_STATE)
			{
				x -= obstacles[stading_index]->speed;
				camera->x -= obstacles[stading_index]->speed;
			}
			if (obstacles[stading_index]->state_z == UP_STATE)
			{
				z += obstacles[stading_index]->speed;
			}
			else if (obstacles[stading_index]->state_z == DOWN_STATE)
			{
				z -= obstacles[stading_index]->speed;
			}
			if (obstacles[stading_index]->state_y == JUMP_UP_STATE)
			{
				y += obstacles[stading_index]->speed;
			}
			else if (obstacles[stading_index]->state_y == JUMP_DOWN_STATE)
			{
				y -= obstacles[stading_index]->speed;
			}
		}
		else
		{
			//추가 이동속력 초기화
			x_additional_speed = z_additional_speed = 0;
		}

		float back_distance; // 충돌 시 되돌아오는 거리값
							 // 기본이동 및 충돌체크
		if (state[RIGHT_STATE])
		{
			x += speed + x_additional_speed; camera->x += speed + x_additional_speed;
			if (x + width > ground->x + ground->width *GROUND_NUM)
			{
				back_distance = (x + width) - (ground->x + ground->width*GROUND_NUM);
				x -= back_distance;
				camera->x -= back_distance;
			}
			else
			{
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || (obstacles[i]->type == PUMKIN && is_under == false))
					{
						if (AABB(obstacles[i]))
						{
							back_distance = x + width - obstacles[i]->x;
							x -= back_distance;
							camera->x -= back_distance;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							back_distance = x + width - obstacles[i]->x;
							x -= back_distance;
							camera->x -= back_distance;
							break;
						}
					}
				}
			}
		}
		if (state[LEFT_STATE])
		{
			x -= (speed + x_additional_speed); camera->x -= (speed + x_additional_speed);
			if (x < 0)
			{
				x = 0;
				camera->x = 0;
			}
			else
			{
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || (obstacles[i]->type == PUMKIN && is_under == false))
					{
						if (AABB(obstacles[i]))
						{
							back_distance = obstacles[i]->x + obstacles[i]->width - x;
							x += back_distance;
							camera->x += back_distance;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							back_distance = obstacles[i]->x + obstacles[i]->width - x;
							x += back_distance;
							camera->x += back_distance;
							break;
						}
					}
				}
			}
		}
		if (state[UP_STATE] && camera->view_point == DOWN_VIEW)
		{
			z += (speed + z_additional_speed);
			if (z + depth > ground->z + ground->depth - 5)
			{
				back_distance = (z + depth) - (ground->z + ground->depth - 5);
				z -= back_distance;
			}
			else
			{
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || (obstacles[i]->type == PUMKIN && is_under == false))
					{
						if (AABB(obstacles[i]))
						{
							back_distance = z + depth - obstacles[i]->z;
							z -= back_distance;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							int last_z = z - (speed + z_additional_speed);
							if (last_z >= obstacles[i]->z && last_z + depth <= obstacles[i]->z + obstacles[i]->depth)
							{
								back_distance = z + depth - (obstacles[i]->z + obstacles[i]->depth);
								z -= back_distance;
							}
							else
							{
								back_distance = z + depth - obstacles[i]->z;
								z -= back_distance;
							}
							break;
						}
					}
				}
			}
		}
		if (state[DOWN_STATE] && camera->view_point == DOWN_VIEW)
		{
			z -= (speed + z_additional_speed);
			if (z < ground->z + 10)
			{
				back_distance = ground->z - z + 10;
				z += back_distance;
			}
			else
			{
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || (obstacles[i]->type == PUMKIN && is_under == false))
					{
						if (AABB(obstacles[i]))
						{
							back_distance = obstacles[i]->z + obstacles[i]->depth - z;
							z += back_distance;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							int last_z = z + (speed + z_additional_speed);
							if (last_z >= obstacles[i]->z && last_z + depth <= obstacles[i]->z + obstacles[i]->depth)
							{
								back_distance = obstacles[i]->z - z;
								z += back_distance;
							}
							else
							{
								back_distance = obstacles[i]->z + obstacles[i]->depth - z;
								z += back_distance;
							}
							break;
						}
					}
				}
			}
		}
		if (state[JUMP_UP_STATE])
		{
			y += (speed + y_additional_speed);
			if (y > org_y + jump_height - minus_height)
			{
				state[JUMP_UP_STATE] = false;
				state[JUMP_DOWN_STATE] = true;
			}
			else
			{
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN)
					{
						if (AABB(obstacles[i]))
						{
							back_distance = y + height - obstacles[i]->y;
							y -= back_distance;
							state[JUMP_UP_STATE] = false;
							state[JUMP_DOWN_STATE] = true;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							int last_y = y - (speed + y_additional_speed);
							if (last_y >= obstacles[i]->y && last_y + height <= obstacles[i]->y + obstacles[i]->height)
							{
								y = last_y;
								state[JUMP_UP_STATE] = false;
								state[JUMP_DOWN_STATE] = true;
							}
							else
							{
								back_distance = y + height - obstacles[i]->y;
								y -= back_distance;
								state[JUMP_UP_STATE] = false;
								state[JUMP_DOWN_STATE] = true;
							}
							break;
						}
					}
				}
			}
		}
		else if (state[JUMP_DOWN_STATE])
		{
			//추가속도 및 점프감소력 초기화
			y_additional_speed = minus_height = 0;
			y -= speed*1.3;
			if (y < 0)
			{
				y = 0;
				state[JUMP_DOWN_STATE] = false;
			}
			else
			{
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN)
					{
						if (AABB(obstacles[i]))
						{
							back_distance = obstacles[i]->y + obstacles[i]->height - y;
							y += back_distance;
							state[JUMP_DOWN_STATE] = false;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							int last_y = y + speed*1.3;
							if (last_y >= obstacles[i]->y && last_y + height <= obstacles[i]->y + obstacles[i]->height)
							{
								y = last_y;
								state[JUMP_DOWN_STATE] = false;
							}
							else
							{
								back_distance = obstacles[i]->y + obstacles[i]->height - y;
								y += back_distance;
								state[JUMP_DOWN_STATE] = false;
							}
							break;
						}
					}
					else if (obstacles[i]->type == BLACK_SHEEP && AABB(obstacles[i]))
					{
						FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[KILL_E], 0, &g_Channel[KILL_E]);
						obstacles[i]->killed = true;
						state[JUMP_DOWN_STATE] = false;
						state[JUMP_UP_STATE] = true;
						org_y = y;
					}
				}
			}
		}

		//중력
		if (state[JUMP_UP_STATE] == false && state[JUMP_DOWN_STATE] == false && y > 0)
		{
			//추가속도 및 점프감소력 초기화
			y_additional_speed = minus_height = 0;
			state[GRAVITY] = true;
			y -= speed*1.3;
			if (y <= 0)
			{
				y = 0;
				state[GRAVITY] = false;
			}
			else {
				for (int i = 0; i < ob_num; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN)
					{
						if (AABB(obstacles[i]))
						{
							back_distance = obstacles[i]->y + obstacles[i]->height - y;
							y += back_distance;
							state[GRAVITY] = false;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						int last_y = y + speed*1.3;
						if (obstacles[i]->AABB_surface(this))
						{
							if (last_y < obstacles[i]->y || last_y + height > obstacles[i]->y + obstacles[i]->height)
							{
								back_distance = obstacles[i]->y + obstacles[i]->height - y;
								y += back_distance;
								state[GRAVITY] = false;
								break;
							}
						}
					}
					else if (obstacles[i]->type == BLACK_SHEEP && AABB(obstacles[i]))
					{
						FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[KILL_E], 0, &g_Channel[KILL_E]);
						obstacles[i]->killed = true;
						state[JUMP_UP_STATE] = true;
						state[GRAVITY] = false;
						org_y = y;
					}
				}
			}
		}
		else if (y == 0) state[GRAVITY] = false;

	}
	void special_key(int key, Object* obstacles[])
	{
		if (key == GLUT_KEY_RIGHT) {
			state[RIGHT_STATE] = true;
		}
		else if (key == GLUT_KEY_LEFT) {
			state[LEFT_STATE] = true;
		}
		else if (key == GLUT_KEY_UP) {
			if (camera->view_point == DOWN_VIEW)
			{
				state[UP_STATE] = true;
			}
			else if (camera->view_point == FRONT_VIEW && state[JUMP_DOWN_STATE] == false && state[JUMP_UP_STATE] == false && state[GRAVITY] == false && Game_Mode == PLAY_MODE)
			{
				state[JUMP_UP_STATE] = true;
				org_y = y;
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[JUMP_E], 0, &g_Channel[JUMP_E]);
			}
		}
		else if (key == GLUT_KEY_DOWN) {
			if (camera->view_point == DOWN_VIEW)
			{
				state[DOWN_STATE] = true;
			}
		}
	}
	void special_key_up(int key)
	{
		if (key == GLUT_KEY_RIGHT) {
			state[RIGHT_STATE] = false;
		}
		else if (key == GLUT_KEY_LEFT) {
			state[LEFT_STATE] = false;
		}
		else if (key == GLUT_KEY_UP) {
			state[UP_STATE] = false;
		}
		else if (key == GLUT_KEY_DOWN) {
			state[DOWN_STATE] = false;
		}
	}
};
struct Box : public Object {
	int org_x, org_y, org_z;
	Box(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 0, 0, 0, sp, m_x, m_y, m_z) {
		if (type == BOX)
		{
			width = 70; height = 70; depth = 80;
		}
		else if (type == BRICK)
		{
			width = 70; height = 35; depth = 80;
		}
		else if (type == BOXWALL)
		{
			width = 70; height = 210; depth = 400;
		}
		max_x = m_x;	max_y = m_y;	max_z = m_z;
		org_x = x;		org_y = y;		org_z = z;
		if (max_x > 0) state_x = RIGHT_STATE;
		else if (max_x < 0) state_x = LEFT_STATE;
		else state_x = STOP_STATE;
		if (max_y > 0) state_y = JUMP_UP_STATE;
		else if (max_y < 0) state_y = JUMP_DOWN_STATE;
		else state_y = STOP_STATE;
		if (max_z > 0) state_z = UP_STATE;
		else if (max_z < 0) state_z = DOWN_STATE;
		else state_z = STOP_STATE;
	}
	~Box() {  }
	virtual void draw() override final
	{
		glColor3f(1, 1, 1);

		//앞면
		glEnable(GL_TEXTURE_2D);
		if (type == BOX) { glBindTexture(GL_TEXTURE_2D, textures[BOX_FRONT]); }
		else if (type == BRICK) { glBindTexture(GL_TEXTURE_2D, textures[BRICK_FRONT]); }
		else if (type == BOXWALL) { glBindTexture(GL_TEXTURE_2D, textures[BOXWALL_FRONT]); }
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z);
		glEnd();

		//윗면
		if (type == BOX) { glBindTexture(GL_TEXTURE_2D, textures[BOX_TOP]); }
		else if (type == BRICK) { glBindTexture(GL_TEXTURE_2D, textures[BRICK_TOP]); }
		else if (type == BOXWALL) { glBindTexture(GL_TEXTURE_2D, textures[BOXWALL_TOP]); }
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y + height, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z + depth);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z + depth);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		//바닥면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x, y, z + depth);
		glVertex3f(x + width, y, z + depth);
		glVertex3f(x + width, y, z);
		glEnd();

		//왼쪽면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x, y + height, z);
		glVertex3f(x, y + height, z + depth);
		glVertex3f(x, y, z + depth);
		glEnd();
		//오른쪽면
		glBegin(GL_QUADS);
		glVertex3f(x + width, y, z);
		glVertex3f(x + width, y, z + depth);
		glVertex3f(x + width, y + height, z + depth);
		glVertex3f(x + width, y + height, z);
		glEnd();
		//뒷면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z + depth);
		glVertex3f(x, y + height, z + depth);
		glVertex3f(x + width, y + height, z + depth);
		glVertex3f(x + width, y, z + depth);
		glEnd();
	}
	virtual bool is_standing(const Object* other)
	{
		if (y + height != other->y) return false;
		if (x + width <= other->x) return false;
		if (x >= other->x + other->width) return false;
		if (z + depth <= other->z) return false;
		if (z >= other->z + other->depth) return false;
		return true;
	}
	virtual void update(Sheep* sheep)
	{
		if (state_x == RIGHT_STATE)
		{
			x += speed;
			if (AABB(sheep))
			{
				sheep->x += speed;
				camera->x += speed;
			}
			if (abs(x - org_x) >= abs(max_x)) {
				state_x = LEFT_STATE;
				x -= 2 * speed;
			}
		}
		if (state_x == LEFT_STATE)
		{
			x -= speed;
			if (AABB(sheep))
			{
				sheep->x -= speed;
				camera->x -= speed;
			}
			if (abs(x - org_x) >= abs(max_x)) {
				state_x = RIGHT_STATE;
				x += 2 * speed;
			}
		}
		if (state_z == UP_STATE)
		{
			z += speed;
			if (AABB(sheep))
			{
				sheep->z += speed;
			}
			if (abs(z - org_z) >= abs(max_z))
			{
				state_z = DOWN_STATE;
				z -= 2 * speed;
			}
		}
		else if (state_z == DOWN_STATE)
		{
			z -= speed;
			if (AABB(sheep))
			{
				sheep->z -= speed;
			}
			if (abs(z - org_z) >= abs(max_z))
			{
				state_z = UP_STATE;
				z += 2 * speed;
			}
		}
		if (state_y == JUMP_UP_STATE)
		{
			y += speed;
			if (AABB(sheep))
			{
				sheep->y += speed;
			}
			if (abs(y - org_y) >= abs(max_y))
			{
				state_y = JUMP_DOWN_STATE;
				y -= 2 * speed;
			}
		}
		else if (state_y == JUMP_DOWN_STATE)
		{
			y -= speed;
			if (AABB(sheep))
			{
				sheep->y -= speed;
			}
			if (abs(y - org_y) >= abs(max_y))
			{
				state_y = JUMP_UP_STATE;
				y += 2 * speed;
			}
		}
	}
};
struct Scissors : public Object {
	float org_x, org_y, org_z;
	int Rotate_y;
	int scissor_rot;
	Scissors(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 40, 8, 35, sp, m_x, m_y, m_z) {
		max_x = m_x;	max_y = m_y;	max_z = m_z;
		org_x = x;		org_y = y;		org_z = z;
		Rotate_y = 0;
		scissor_rot = 4;
		if (max_x > 0) state_x = RIGHT_STATE;
		else if (max_x < 0) state_x = LEFT_STATE;
		else state_x = STOP_STATE;
		if (max_z > 0) state_z = UP_STATE;
		else if (max_z < 0) state_z = DOWN_STATE;
		else state_z = STOP_STATE;
	}
	~Scissors() {  }
	virtual void draw() override final
	{
		glPushMatrix();
		glTranslated((x + width / 2) + 28, (y + height / 2) + 12, z + depth / 2);
		if (state_x == RIGHT_STATE)
		{
			glRotated(180, 0, 1, 0);
			glTranslated(57, 0, 0);
		}
		else if (state_z == UP_STATE)
		{
			glRotated(90, 0, 1, 0);
			glTranslated(30, 0, -30);
		}
		else if (state_z == DOWN_STATE)
		{
			glRotated(270, 0, 1, 0);
			glTranslated(30, 0, 30);
		}
		glScaled(0.4, 0.45, 0.35);

		glPushMatrix();
		glRotatef(Rotate_y + 15, 0, 1, 0);

		glColor3f(0, 0, 0);         // 손잡이
		glBegin(GL_POLYGON);
		glVertex3f(0, -20, 5);
		glVertex3f(100, -20, 20);
		glVertex3f(80, -20, -30);
		glVertex3f(0, -20, -5);
		glEnd();

		glBegin(GL_POLYGON);      // FRONT
		glVertex3f(0, -20, -5);
		glVertex3f(0, -50, 0);
		glVertex3f(80, -50, 0);
		glVertex3f(80, -20, 0);
		glEnd();

		glColor3f(0.8, 0.8, 0.8);   // 날
		glBegin(GL_POLYGON);
		glVertex3f(0, -20, 0);
		glVertex3f(-160, -20, 0);
		glVertex3f(-80, -20, 40);
		glEnd();

		glBegin(GL_POLYGON);    //FRONT
		glVertex3f(-160, -20, 0);
		glVertex3f(-160, -50, 0);
		glVertex3f(0, -50, 0);
		glVertex3f(0, -20, 0);
		glEnd();
		glPopMatrix();


		// 오른쪽 날
		glPushMatrix();
		glTranslatef(0, -40, 0);
		glRotatef(180, 1, 0, 0);
		glRotatef(Rotate_y + 15, 0, 1, 0);

		glColor3f(0, 0, 0);         // 손잡이
		glBegin(GL_POLYGON);
		glVertex3f(0, -20, 5);
		glVertex3f(100, -20, 20);
		glVertex3f(80, -20, -30);
		glVertex3f(0, -20, -5);
		glEnd();


		glBegin(GL_POLYGON);      // FRONT
		glVertex3f(0, -20, -5);
		glVertex3f(0, 10, 0);
		glVertex3f(80, 10, 0);
		glVertex3f(80, -20, 0);
		glEnd();

		glColor3f(0.8, 0.8, 0.8);   // 날
		glBegin(GL_POLYGON);
		glVertex3f(0, -20, 0);
		glVertex3f(-160, -20, 0);
		glVertex3f(-80, -20, 40);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex3f(-160, -20, 0);
		glVertex3f(-160, 10, 0);
		glVertex3f(0, 10, 0);
		glVertex3f(0, -20, 0);
		glEnd();
		glPopMatrix();

		glPopMatrix();
	}
	virtual void update(Sheep* sheep)
	{
		Rotate_y += scissor_rot;
		if (Rotate_y >= 15)
			scissor_rot *= -1;
		if (Rotate_y <= -5)
			scissor_rot *= -1;

		if (state_x == RIGHT_STATE)
		{
			x += speed;
			if (abs(x - org_x) >= abs(max_x)) state_x = LEFT_STATE;
		}
		if (state_x == LEFT_STATE)
		{
			x -= speed;
			if (abs(x - org_x) >= abs(max_x)) state_x = RIGHT_STATE;
		}
		if (state_z == UP_STATE)
		{
			z += speed;
			if (abs(z - org_z) >= abs(max_z))
			{
				state_z = DOWN_STATE;
			}
		}
		else if (state_z == DOWN_STATE)
		{
			z -= speed;
			if (abs(z - org_z) >= abs(max_z))
			{
				state_z = UP_STATE;
			}
		}
		if (sheep->is_invincible == false && AABB(sheep))
		{
			sheep->get_hurt();
		}
	}
};
struct Pumkin : public Object {
	int org_y;
	Pumkin(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 50, 50, 50, sp, m_x, m_y, m_z) {
		max_y = m_y;
		org_y = y;
		if (max_y > 0) state_y = JUMP_UP_STATE;
		else if (max_y < 0) state_y = JUMP_DOWN_STATE;
		else state_y = STOP_STATE;
	}
	~Pumkin() {  }
	virtual void draw() override final
	{
		glPushMatrix();
		glTranslated((x + width / 2), (y + height / 2), z + depth / 2);

		glScaled(0.45, 0.45, 0.45);
		// 꼭지
		glPushMatrix();
		glColor3f(0.309804, 0.309804, 0.184314);
		glTranslated(0, 55, 0);
		glRotated(-90, 1, 0, 0);
		glutSolidCone(20, 50, 12, 1);

		glTranslated(0, 0, 45);
		glRotated(-180, 1, 0, 0);
		glutSolidCone(20, 20, 12, 1);
		glPopMatrix();

		// 박
		glPushMatrix();
		glScaled(1, 0.8, 1);

		glColor3f(1, 0.5, 0);
		glutSolidTorus(50, 40, 16, 12);

		glRotated(45, 0, 1, 0);
		glColor3f(1, 0.25, 0);
		glutSolidTorus(50, 40, 16, 12);

		glRotated(45, 0, 1, 0);
		glColor3f(1, 0.5, 0);
		glutSolidTorus(50, 40, 16, 12);

		glRotated(45, 0, 1, 0);
		glColor3f(1, 0.25, 0);
		glutSolidTorus(50, 40, 16, 12);

		glPopMatrix();

		glPopMatrix();
	}
	virtual void update(Sheep* sheep)
	{
		if (state_y == JUMP_UP_STATE)
		{
			y += speed;
			if (AABB(sheep))
			{
				sheep->y += speed;
			}
			if (abs(y - org_y) >= abs(max_y))
			{
				state_y = JUMP_DOWN_STATE;
				y -= (2 * speed);
			}
		}
		else if (state_y == JUMP_DOWN_STATE)
		{
			y -= speed;
			if (AABB(sheep))
			{
				sheep->y -= speed;
				if (sheep->y < 0)
				{
					sheep->y = 0;
					if (sheep->is_invincible == false)
					{
						sheep->is_under = true;
						sheep->get_hurt();
					}
				}
			}
			if (abs(y - org_y) >= abs(max_y))
			{
				state_y = JUMP_UP_STATE;
				y += (2 * speed);
			}
		}
	}
	virtual bool is_standing(const Object* other)
	{
		if (y + height != other->y) return false;
		if (x + width <= other->x) return false;
		if (x >= other->x + other->width) return false;
		if (z + depth <= other->z) return false;
		if (z >= other->z + other->depth) return false;
		return true;
	}
};
struct Hay : public Object
{
	Hay(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 100, 70, 100, sp, m_x, m_y, m_z) {
	}
	~Hay() {  }
	virtual bool AABB_surface(const Object* other) override final
	{
		if (other->x + other->width <= x) return false;
		if (other->x >= x + width) return false;
		if (other->y + other->height <= y) return false;
		if (other->y >= y + height) return false;
		if (other->z + other->depth <= z) return false;
		if (other->z >= z + depth) return false;
		if (other->z >= z && other->z + other->depth <= z + depth && other->y >= y && other->y + other->height <= y + height) return false;
		return true;
	}
	virtual bool is_inside(Sheep* sheep)
	{
		if (sheep->z >= z && sheep->z + sheep->depth <= z + depth &&
			sheep->y >= y && sheep->y + sheep->height <= y + height &&
			sheep->x >= x && sheep->x + sheep->width <= x + width)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	virtual void draw() override final
	{
		glColor3f(1, 1, 1);
		//앞면
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[HAY_FRONT]);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z);
		glEnd();

		//윗면
		glBindTexture(GL_TEXTURE_2D, textures[HAY_TOP]);

		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y + height, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z + depth);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z + depth);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		//바닥면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x, y, z + depth);
		glVertex3f(x + width, y, z + depth);
		glVertex3f(x + width, y, z);
		glEnd();

		//왼쪽면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z);
		glVertex3f(x, y + height, z);
		glVertex3f(x, y + height, z + depth);
		glVertex3f(x, y, z + depth);
		glEnd();
		//오른쪽면
		glBegin(GL_QUADS);
		glVertex3f(x + width, y, z);
		glVertex3f(x + width, y, z + depth);
		glVertex3f(x + width, y + height, z + depth);
		glVertex3f(x + width, y + height, z);
		glEnd();
		//뒷면
		glBegin(GL_QUADS);
		glVertex3f(x, y, z + depth);
		glVertex3f(x, y + height, z + depth);
		glVertex3f(x + width, y + height, z + depth);
		glVertex3f(x + width, y, z + depth);
		glEnd();
	}
};
struct Black_Sheep : public Object {
	int view_rad; // 보는 방향
	int tracing_distance; //추격거리
	int org_x, org_z; // 원 위치
	bool is_tracing; // 추격준비모드
	float ouch; // 꿍한 시간
	int wait_time; // 어리둥절 하는 시간
	float y_scale = 0; // 만큼 축소시킨다.
	Black_Sheep(int t, float x, float y, float z, float sp = 0, int area_of_activity = 0, int none1 = 0, int none2 = 0) : Object(t, x, y, z, 55, 35, 55, sp, area_of_activity, none1, none2) {
		org_x = x;
		org_z = z;
		tracing_distance = area_of_activity;
		view_rad = 0;
		is_tracing = false;
		ouch = 0;
		wait_time = 11;
	}
	~Black_Sheep() { }
	virtual void draw() override final
	{
		glPushMatrix();
		glTranslated((x + width / 2), (y + height / 2) + 5, z + depth / 2);

		glRotated(-view_rad, 0, 1, 0);
		glScalef(0.30, 0.35 - y_scale, 0.35);
		glPushMatrix();
		glColor3f(1, 0.9, 0.9);
		glTranslatef(70, -20, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glColor3f(0, 0, 0);
		glTranslatef(30, 0, 30);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-50, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-20, -10, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(-10, -20, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(30, -10, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(40, 0, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(30, 0, -30);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-50, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-20, -10, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(10, -20, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(30, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(40, 0, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-40, -20, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(30, 10, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-50, -30, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		// 눈
		glPushMatrix();
		glColor3f(1, 0, 0);
		glTranslatef(95, -10, 25);
		glRotated(70, 1, 0, 0);
		glutSolidTorus(9, 10, 5, 12);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(95, -10, -25);
		glRotated(-70, 1, 0, 0);
		glutSolidTorus(9, 10, 5, 12);
		glPopMatrix();


		//  입
		glPushMatrix();
		glColor3f(1, 0, 0);
		glTranslated(90, -30, 0);
		glRotated(95, 1, 0, 0);
		glutSolidTorus(9, 25, 5, 12);
		glPopMatrix();

		/*glPushMatrix();
		glColor3f(1, 0, 0);
		glTranslatef(90, 0, 35);
		glutSolidSphere(7, 16, 16);
		glTranslatef(0, 0, -70);
		glutSolidSphere(7, 16, 16);
		glPopMatrix();*/

		// 뿔
		glPushMatrix();
		glTranslated(70, 5, 30);
		glRotated(-45, 1, 0, 0);
		glColor3f(0.7, 0.7, 0.7);
		glutSolidCone(20, 50, 10, 10);
		glPopMatrix();

		glPushMatrix();
		glTranslated(70, 5, -30);
		glRotated(-135, 1, 0, 0);
		glColor3f(0.7, 0.7, 0.7);
		glutSolidCone(20, 50, 10, 10);
		glPopMatrix();
		glPopMatrix();
	}

	void trace_return(Sheep* sheep, Object* obstacles[])
	{
		// 양과의 좌표거리 계산
		int sx = sheep->x, sz = sheep->z;
		float d = sqrt(float(sx - x)*float(sx - x) + float(sz - z)*float(sz - z));
		// 원위치와의 계산
		float org_d = sqrt(float(org_x - x)*float(org_x - x) + float(org_z - z)*float(org_z - z));
		int vx, vz; // 이동량

		if (is_tracing)
		{
			// 추격 종료
			if (org_d > tracing_distance || sheep->is_in_hay || d > tracing_distance || ouch > 1000)
			{
				is_tracing = false;
			}
			else
			{
				// 바라보는 각도 계산
				view_rad = atan2(sheep->z - z, sheep->x - x) * 180 / 3.1415926535;

				// 이동
				if (d > speed)
				{
					vx = (sx - x) / d*speed;
					vz = (sz - z) / d*speed;
				}
				else
				{
					vx = 0;
					vz = 0;
				}
				x += vx;
				z += vz;

				// 충돌체크
				if (AABB(sheep))
				{
					x -= vx;
					z -= vz;
					if (sheep->is_invincible == false)
					{
						sheep->get_hurt();
					}
				}
				else
				{
					for (int i = 0; i < ob_num; ++i)
					{
						if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN || obstacles[i]->type == HAY)
						{
							if (AABB(obstacles[i]))
							{
								ouch += DELTA_TIME;
								x -= vx;
								z -= vz;
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			wait_time += DELTA_TIME;
			if (wait_time > 1000)
			{
				ouch = 0;
				// 바라보는 각도 계산
				view_rad = atan2(org_z - z, org_x - x) * 180 / 3.1415926535;

				if (org_d > speed)
				{
					vx = (org_x - x) / org_d*speed;
					vz = (org_z - z) / org_d*speed;
				}
				else
				{
					vx = 0;
					vz = 0;
				}
				x += vx;
				z += vz;
			}
			//추격 재개
			else if (org_d <= tracing_distance && sheep->is_in_hay == false && d <= tracing_distance && ouch <= 1000)
			{
				is_tracing = true;
				wait_time = 0;
			}

			// 원점으로 돌아옴
			if (speed >= abs(org_x - x) && speed >= abs(org_z - z) && d <= tracing_distance && sheep->y == y)
			{
				is_tracing = true;
				wait_time = 0;
			}
		}

	}
	virtual void update(Sheep* sheep, Object* obstacles[])
	{
		if (killed)
		{
			if (height >0)
			{
				y--;
				height -= sheep->speed;
				//y_scale += 0.01*DELTA_TIME;
			}
		}
		else { trace_return(sheep, obstacles); }
	}
};
struct MotherSheep {
	int x = ENDING_X + 250, y = 50, z = 100;
	void draw()
	{
		glPushMatrix();
		glTranslated(x, y, z);

		glRotated(180, 0, 1, 0);
		glScalef(0.8, 0.8, 0.8);


		glPushMatrix();
		glColor3f(1, 0.9, 0.9);
		glTranslatef(70, -20, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glColor3f(1, 0.8, 0.8);
		glTranslatef(10, -20, 0);
		glScalef(2, 1, 1);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		// 몸통이
		glPushMatrix();
		glColor3f(1, 1, 1);


		glColor3f(1, 1, 1);

		glTranslatef(30, 0, 30);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-50, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-20, -10, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(-10, -20, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(30, -10, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(40, 0, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(30, 0, -30);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-50, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(-20, -10, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(10, -20, 0);
		glutSolidSphere(40, 16, 16);

		glTranslatef(30, 0, 0);
		glutSolidSphere(50, 16, 16);

		glTranslatef(40, 0, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-40, -20, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(30, 10, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-50, -30, 0);
		glutSolidSphere(50, 16, 16);
		glPopMatrix();

		// 눈
		glPushMatrix();
		glColor3f(0, 0, 0);
		glTranslatef(90, 0, 35);
		glutSolidSphere(7, 16, 16);
		glTranslatef(0, 0, -70);
		glutSolidSphere(7, 16, 16);
		glPopMatrix();

		glPopMatrix();
	}
};

struct Ui {
	int canvas_size;
	int selected_menu;
	float heart_size;
	float heart_dir;
	bool presskey;
	int key_delay[2];
	int help;
	int ending_screen = 0;
	Ui(int size) : canvas_size(size), selected_menu(0), heart_size(0.5), heart_dir(1), presskey(false), help(0) {

		for (int i = 0; i < 2; ++i)
			key_delay[i] = 0;
	}
	~Ui() { }
	void keyboard(unsigned char key)
	{
		if (Game_Mode == MAIN_MODE && selected_menu == 1 && key_delay[0] == 0)
		{
			FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
			key_delay[0] = 3;
			help = (help + 1) % 3;
		}
		if (key == 27 && key_delay[0] == 0) // ESC
		{
			if (Game_Mode == PLAY_MODE)
			{
				key_delay[0] = 5;
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
				FMOD_Channel_SetPaused(g_Channel[GAME_BGM], true);
				selected_menu = 0;
				Game_Mode = PAUSE_MODE;
			}
			else if (Game_Mode == PAUSE_MODE)
			{
				key_delay[0] = 5;
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
				FMOD_Channel_SetPaused(g_Channel[GAME_BGM], false);
				selected_menu = 0;
				Game_Mode = PLAY_MODE;
			}
		}
		else if ((key == ' ' || key == 13) && key_delay[0] == 0)
		{
			if (Game_Mode == MAIN_MODE)
			{
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
				if (presskey == false)
				{
					FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
					presskey = true;
				}
				else if (selected_menu == 0)
				{
					NetworkManager.getReady();
					//게임시작
					CreateWorld();
					Game_Mode = READY_MODE;
					FMOD_Channel_Stop(g_Channel[MAIN_BGM]);
					FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[GAME_BGM], 0, &g_Channel[GAME_BGM]);
					FMOD_Channel_SetVolume(g_Channel[GAME_BGM], GAME_BGM_VOLUME);
					FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[CRY_E], 0, &g_Channel[CRY_E]);
				}
				else if (selected_menu == 2)
				{
					//종료
					exit(0);
				}
			}
			else if (Game_Mode == PAUSE_MODE || sheep->killed || ending_screen == 3)
			{
				if (ending_screen == 3)
				{
					FMOD_Channel_Stop(g_Channel[CLEAR_BGM]);
					ending_screen = 0;
					sheep->ending_finished = false;
				}
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
				if (selected_menu == 0) {
					//이어하기
					FMOD_Channel_SetPaused(g_Channel[GAME_BGM], false);
					Game_Mode = PLAY_MODE;
				}
				else if (selected_menu == 1)
				{
					FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[GAME_BGM], 0, &g_Channel[GAME_BGM]);
					FMOD_Channel_SetVolume(g_Channel[GAME_BGM], GAME_BGM_VOLUME);
					//다시하기
					DestroyWorld();
					CreateWorld();
					Game_Mode = PLAY_MODE;
				}
				else if (selected_menu == 2)
				{
					//메인메뉴로
					DestroyWorld();
					Game_Mode = MAIN_MODE;
					FMOD_Channel_Stop(g_Channel[GAME_BGM]);
					FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[MAIN_BGM], 0, &g_Channel[MAIN_BGM]);
				}
				selected_menu = 0;
			}
		}
		if (presskey == false)
		{
			presskey = true;
		}
	}
	void special_key(int key)
	{
		if (presskey == false)
		{
			FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
			presskey = true;
		}
		else
		{
			if ((Game_Mode == MAIN_MODE || Game_Mode == PAUSE_MODE) && (key_delay[1] == 0))
			{
				key_delay[1] = 3;
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_MOVE_E], 0, &g_Channel[BUTTON_MOVE_E]);
				if (key == GLUT_KEY_RIGHT) {
					if (selected_menu == 1)
					{
						help = (help + 1) % 3;
						if (help < 1) help = 1;
					}
					else
					{
						selected_menu = (selected_menu + 1) % 3;
					}
				}
				else if (key == GLUT_KEY_LEFT) {
					if (selected_menu == 1)
					{
						--help;
						if (help < 1) help = 2;
					}
					else
					{
						--selected_menu;
						if (selected_menu < 0) selected_menu = 2;
					}
				}
				else if (key == GLUT_KEY_UP) {
					help = 0;
					--selected_menu;
					if (selected_menu < 0) selected_menu = 2;
				}
				else if (key == GLUT_KEY_DOWN) {
					help = 0;
					selected_menu = (selected_menu + 1) % 3;
				}
			}
			else if ((key_delay[1] == 0) && (sheep->killed || ending_screen == 3))
			{
				key_delay[1] = 3;
				FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_MOVE_E], 0, &g_Channel[BUTTON_MOVE_E]);
				if (key == GLUT_KEY_RIGHT) {
					++selected_menu;
					if (selected_menu > 2) selected_menu = 1;
				}
				else if (key == GLUT_KEY_LEFT) {
					--selected_menu;
					if (selected_menu < 1) selected_menu = 2;
				}
				else if (key == GLUT_KEY_UP) {
					--selected_menu;
					if (selected_menu < 1) selected_menu = 2;
				}
				else if (key == GLUT_KEY_DOWN) {
					++selected_menu;
					if (selected_menu > 2) selected_menu = 1;
				}
			}
		}
	}
	void draw(Sheep* sheep)
	{
		glPushMatrix();
		glOrtho(-canvas_size, canvas_size, -canvas_size, canvas_size, 1000, -1000);

		// 메인 메뉴
		if (Game_Mode == MAIN_MODE)
		{
			glPushMatrix();
			glColor3f(1, 1, 1);
			int x = -200, y = -200, z = -500;
			int width = 400, height = 400;
			glEnable(GL_TEXTURE_2D);
			if (presskey == false) { glBindTexture(GL_TEXTURE_2D, textures[MAIN_0]); }
			else if (selected_menu == 0) { glBindTexture(GL_TEXTURE_2D, textures[MAIN_1]); }
			else if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, textures[MAIN_2]); }
			else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, textures[MAIN_3]); }
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0);
			glVertex3f(x, y, z);
			glTexCoord2i(1, 0);
			glVertex3f(x + width, y, z);
			glTexCoord2i(1, 1);
			glVertex3f(x + width, y + height, z);
			glTexCoord2i(0, 1);
			glVertex3f(x, y + height, z);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();

			//도움말
			if (help > 0)
			{
				glPushMatrix();
				glColor3f(1, 1, 1);
				int x = -180, y = -180, z = -550;
				int width = 270, height = 270;
				glEnable(GL_TEXTURE_2D);
				if (help == 1) { glBindTexture(GL_TEXTURE_2D, textures[HELP_0]); }
				else if (help == 2) { glBindTexture(GL_TEXTURE_2D, textures[HELP_1]); }
				glBegin(GL_QUADS);
				glTexCoord2i(0, 0);
				glVertex3f(x, y, z);
				glTexCoord2i(1, 0);
				glVertex3f(x + width, y, z);
				glTexCoord2i(1, 1);
				glVertex3f(x + width, y + height, z);
				glTexCoord2i(0, 1);
				glVertex3f(x, y + height, z);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				glPopMatrix();
			}
		}
		else
		{
			if (Game_Mode == ENDING_MODE)
			{
				// 엔딩 클리어화면
				static int x = 0, y = 0, z = -500;
				static int width = 0, height = 0;
				static int time;
				if (sheep->ending_finished == false)
				{
					x = 0, y = 0, z = -500;
					width = 0, height = 0;
					time = 0;
				}
				else
				{
					glPushMatrix();
					glColor3f(1, 1, 1);
					if (x > -150)
					{
						x -= DELTA_TIME*0.1;
						y -= DELTA_TIME*0.1;
						width += 0.2 * DELTA_TIME;
						height += 0.2 * DELTA_TIME;
					}
					else if (ending_screen != 3)
					{
						time += DELTA_TIME;
						if (time % 200 == 0)
						{
							ending_screen = (ending_screen + 1) % 2;
						}
						if (time >= 2000)
						{
							ending_screen = 3;
						}
					}
					glEnable(GL_TEXTURE_2D);
					if (ending_screen == 0) { glBindTexture(GL_TEXTURE_2D, textures[ENDING_0]); }
					else if (ending_screen == 1) { glBindTexture(GL_TEXTURE_2D, textures[ENDING_1]); }
					else if (ending_screen == 3)
					{
						if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, textures[ENDING_MENU_0]); }
						else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, textures[ENDING_MENU_1]); }
					}
					glBegin(GL_QUADS);
					glTexCoord2i(0, 0);
					glVertex3f(x, y, z);
					glTexCoord2i(1, 0);
					glVertex3f(x + width, y, z);
					glTexCoord2i(1, 1);
					glVertex3f(x + width, y + height, z);
					glTexCoord2i(0, 1);
					glVertex3f(x, y + height, z);
					glEnd();
					glDisable(GL_TEXTURE_2D);
					glPopMatrix();
				}
			}
			else
			{
				// 목숨
				glPushMatrix();
				glColor3f(0.9, 0.2, 0.2); //하트 색
				glTranslated(-170, 170, -500); //화면상의 하트 위치
				for (int i = 0; i < sheep->life; ++i)
				{
					glPushMatrix();
					glTranslated(i * 43, 0, 0); //하트사이 x간격 
					glRotated(90, 1, 0, 0);
					glScaled(heart_size, heart_size, heart_size); // 하트크기
																  //왼쪽부분
					glPushMatrix();
					glTranslated(-9, 0, 0);
					glRotated(-45, 0, 1, 0);
					glutSolidTorus(18, 10, 20, 20);
					glPopMatrix();
					//오른쪽부분
					glPushMatrix();
					glTranslated(9, 0, 0);
					glRotated(45, 0, 1, 0);
					glutSolidTorus(18, 10, 20, 20);
					glPopMatrix();
					glPopMatrix();
				}
				glPopMatrix();
			}

			if (Game_Mode == PLAY_MODE)
			{
				//ESC
				glPushMatrix();
				glColor3f(1, 1, 1);
				int x = 140, y = 140, z = -500;
				int width = 50, height = 50;
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, textures[ESC_BUTTON]);
				glBegin(GL_QUADS);
				glTexCoord2i(0, 0);
				glVertex3f(x, y, z);
				glTexCoord2i(1, 0);
				glVertex3f(x + width, y, z);
				glTexCoord2i(1, 1);
				glVertex3f(x + width, y + height, z);
				glTexCoord2i(0, 1);
				glVertex3f(x, y + height, z);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				glPopMatrix();
			}

			// 메뉴 - PAUSE_MODE
			if (Game_Mode == PAUSE_MODE)
			{
				glPushMatrix();
				glColor3f(1, 1, 1);
				int x = -70, y = -130, z = -500;
				int width = 155, height = 250;
				glEnable(GL_TEXTURE_2D);
				if (selected_menu == 0) { glBindTexture(GL_TEXTURE_2D, textures[MENU_0]); }
				else if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, textures[MENU_1]); }
				else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, textures[MENU_2]); }
				glBegin(GL_QUADS);
				glTexCoord2i(0, 0);
				glVertex3f(x, y, z);
				glTexCoord2i(1, 0);
				glVertex3f(x + width, y, z);
				glTexCoord2i(1, 1);
				glVertex3f(x + width, y + height, z);
				glTexCoord2i(0, 1);
				glVertex3f(x, y + height, z);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				glPopMatrix();
			}
			//사망메뉴
			else if (sheep->killed)
			{
				glPushMatrix();
				glColor3f(1, 1, 1);
				int x = -70, y = -130, z = -500;
				int width = 155, height = 250;
				glEnable(GL_TEXTURE_2D);
				if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, textures[DEAD_0]); }
				else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, textures[DEAD_1]); }
				glBegin(GL_QUADS);
				glTexCoord2i(0, 0);
				glVertex3f(x, y, z);
				glTexCoord2i(1, 0);
				glVertex3f(x + width, y, z);
				glTexCoord2i(1, 1);
				glVertex3f(x + width, y + height, z);
				glTexCoord2i(0, 1);
				glVertex3f(x, y + height, z);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				glPopMatrix();
			}


		}

		glPopMatrix();
	}
	void update()
	{
		if ((Game_Mode == GAME_OVER || Game_Mode == ENDING_MODE) && selected_menu == 0)
		{
			selected_menu = 1;
		}

		for (auto &k : key_delay)
		{
			if (k) { --k; }
		}

		heart_size += heart_dir*0.00025 * DELTA_TIME;

		if (heart_size > 0.6) { heart_dir = -1; } // 하트 최대 크기
		else if (heart_size < 0.5) { heart_dir = +1; } // 하트 최소 크기
	}
};


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
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 96, 96, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pBytes);
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
	// 기본 객체
	camera = new Camera();
	ground[0] = new Ground(-200, -100, -100); // z축 -100~300
	for (int i = 1; i < GROUND_NUM; ++i)
	{
		ground[i] = new Ground(ground[0]->x + ground[0]->width*i, ground[0]->y, ground[0]->z);
	}
	sheep = new Sheep(SHEEP, camera->x, 0, -30, 9);
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
			break;
		case BOXWALL:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case SCISSORS:
			obstacles[ob_num] = new Scissors(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case PUMKIN:
			obstacles[ob_num] = new Pumkin(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case HAY:
			obstacles[ob_num] = new Hay(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case BLACK_SHEEP:
			obstacles[ob_num] = new Black_Sheep(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		}
		++ob_num;
	}
	fin.close();

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

	delete camera;
	for (auto g : ground)
		delete g;
	delete sheep;
	delete mother_sheep;
	for (int i = 0; i < ob_num; ++i)
		delete obstacles[i];
	ob_num = 0;
}
void Program_Exit()
{

	delete ui;
	for (int i = 0; i < SOUND_COUNT; ++i)
	{
		FMOD_Sound_Release(g_Sound[i]);
	}
	FMOD_System_Close(g_System);
	FMOD_System_Release(g_System);

	if (ob_num)
	{
		delete camera;
		for (auto g : ground)
			delete g;
		delete sheep;
		for (int i = 0; i < ob_num; ++i)
			delete obstacles[i];
	}
}

void main()
{
	NetworkManager.connectServer();

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
	// 커서 가리기
	//ShowCursor(false);

	glutSpecialFunc(SpecialKeyboard);
	glutSpecialUpFunc(SpecialKeyboardUp);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(50, updateScene, 1);
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

float currentTime = clock();
float accumulator = 0.0;
GLvoid updateScene(int value)
{
	FMOD_System_Update(g_System);

	float newTime = clock();
	float frameTime = newTime - currentTime;

	while (frameTime < FIXED_FRAME_TIME) {
		frameTime = clock() - currentTime;
	}

	if (frameTime > MAX_FRAME_TIME) {
		frameTime = MAX_FRAME_TIME;
	}

	currentTime = newTime;
	//printf("FPS:%f \n", 1000.0 / frameTime);
	accumulator += frameTime;
	while (accumulator >= DELTA_TIME) {

		ui->update();

		switch (Game_Mode)
		{
		case PLAY_MODE:

			//카메라 업데이트
			if(!sheep->killed)
				camera->update();
			//객체 업데이트 (+스탠딩 상태 확인)
			sheep->stading_index = -1;
			for (int i = 0; i < ob_num; ++i) {

				if (sheep->stading_index == -1 && obstacles[i]->is_standing(sheep)) {
					sheep->stading_index = i;
				}

				if (obstacles[i]->type == BLACK_SHEEP) {
					obstacles[i]->update(sheep, obstacles);
				}
				else {
					obstacles[i]->update(sheep);
				}
			}
			//양 업데이트
			sheep->update(ground[0], obstacles);
			break;
		case GAME_OVER:
			sheep->dead_update();
			break;
		case ENDING_MODE:
			sheep->ending_update();
			break;
		}

		accumulator -= DELTA_TIME;
	}
	glutPostRedisplay();
	glutTimerFunc(1, updateScene, 1);

}

GLvoid drawScene(GLvoid)
{
	glClearColor(0, 0.8f, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();

	// UI
	ui->draw(sheep);

	if (Game_Mode != MAIN_MODE)
	{
		////카메라 설정
		camera->setting();

		//바닥
		for (auto g : ground)
		{
			g->draw();
		}
		//양
		sheep->draw();
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
	if (Game_Mode == PLAY_MODE)
	{
		camera->keyboard(key);
	}
	ui->keyboard(key);
}

GLvoid SpecialKeyboard(int key, int x, int y)
{
	ui->special_key(key);
	if (Game_Mode != MAIN_MODE)
		sheep->special_key(key, obstacles);
}

GLvoid SpecialKeyboardUp(int key, int x, int y)
{
	if (Game_Mode != MAIN_MODE)
		sheep->special_key_up(key);
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
	if ((fp = fopen(filename, "rb")) == NULL)
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
