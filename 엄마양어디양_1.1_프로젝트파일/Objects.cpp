#include "Objects.h"


Camera::Camera()
{
	x = 0; y = 100;
	canvas_size = 200;
	view_radius = 0;
	view_point = FRONT_VIEW;
	is_changing = false;
}
Camera::~Camera() {  }
void Camera::setting()
	{
		glOrtho(-canvas_size, canvas_size, -canvas_size, canvas_size, 1000, -1000);

		gluLookAt(x, y, 0, x, y, -1, 0.0, 1.0, 0.0);
		glRotated(view_radius, 1, 0, 0);
	}
void Camera::keyboard(unsigned char key)
	{
		if (key == ' ' && is_changing == false)
		{
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CAMERA_E], 0, &pSound->Channel[CAMERA_E]);
			is_changing = true;
		}
	}
void Camera::update()
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


Ground::Ground(float x, float y, float z) :x(x), y(y), z(z), width(1000), height(100), depth(400) {
	back_height = 300;
}
Ground::~Ground() { }
void Ground::draw() {
		glColor3f(1, 1, 1);

		glEnable(GL_TEXTURE_2D);

		//뒷배경
		glBindTexture(GL_TEXTURE_2D, pTextures[BACK_GROUND]);
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
		glBindTexture(GL_TEXTURE_2D, pTextures[GROUND_FRONT]);
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
		glBindTexture(GL_TEXTURE_2D, pTextures[GROUND_TOP]);
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


Object::Object(int type, float x, float y, float z, float w, float h, float d, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : x(x), y(y), z(z), width(w), height(h), depth(d), type(type), speed(sp*DELTA_TIME*0.03), max_x(m_x), max_y(m_y), max_z(m_z) {}
bool Object::AABB(const Object* other)
{
	if (x + width <= other->x) return false;
	if (x >= other->x + other->width) return false;
	if (y + height <= other->y) return false;
	if (y >= other->y + other->height) return false;
	if (z + depth <= other->z) return false;
	if (z >= other->z + other->depth) return false;
	return true;
}
void Object::test_draw()
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


Sheep::Sheep(int t, int x, int y, int z, float sp) : Object(t, x, y, z, 45, 30, 35, sp) {
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
Sheep::~Sheep() { 
	delete pCamera;
}
void Sheep::get_hurt()
	{
		is_invincible = true;
		--life;
		if (life < 1)
		{
			*pGameMode = GAME_OVER;
			FMOD_Channel_Stop(pSound->Channel[GAME_BGM]);
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CRY_E], 0, &pSound->Channel[CRY_E]);
		}
		else
		{
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[GET_HURT_E], 0, &pSound->Channel[GET_HURT_E]);
		}
	}
void Sheep::dead_update()
	{
		pCamera->is_changing = false;

		//죽은 양
		if (pCamera->view_radius != -40)
			(pCamera->view_radius < -40) ? pCamera->view_radius += 0.25*DELTA_TIME : pCamera->view_radius -= 0.25*DELTA_TIME;
		else if (y <= 500)
		{
			y += 0.5f*DELTA_TIME;

			if (y > 500)
			{
				killed = true;
			}
		}
	}
void Sheep::ending_update()
	{

		pCamera->is_changing = false;

		static int dir = 1, jump_cnt;
		static bool bsound = true;
		const int JUMP_MAX = 3;

		// 카메라이동
		if (pCamera->view_radius != -20)
		{
			(pCamera->view_radius < -20) ? pCamera->view_radius += 0.5*DELTA_TIME : pCamera->view_radius -= 0.5*DELTA_TIME;
			dir = 1, jump_cnt = 0;
			bsound = true;
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
		pCamera->x += vx;
		z += vz;

		if (vx == 0 && vz == 0 && jump_cnt < JUMP_MAX)
		{
			if (bsound)
			{
				FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[JUMP_E], 0, &pSound->Channel[JUMP_E]);
				bsound = false;
			}

			y += speed*dir;
			if (y > jump_height)
				dir = -1;
			if (y < 0)
			{
				bsound = true;
				y = 0;
				dir = 1;
				++jump_cnt;
			}
		}
		else if (jump_cnt == JUMP_MAX) { ending_finished = true; }

	}
void Sheep::draw() 
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
		if (*pGameMode == GAME_OVER)
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
void Sheep::update2(const Ground* ground, Object* obstacles[])
	{
		//엔딩
		if (*pGameMode != ENDING_MODE && x > ENDING_X)
		{
			*pGameMode = ENDING_MODE;
			FMOD_Channel_Stop(pSound->Channel[GAME_BGM]);
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CLEAR_BGM], 0, &pSound->Channel[CLEAR_BGM]);
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
		for (int i = 0; i < obCnt; ++i)
		{
			if (obstacles[i]->type == HAY 
				&& obstacles[i]->is_inside(this))
			{
				is_in_hay = true;
				break;
			}
			is_in_hay = false;
		}

		//무적상태
		if (is_invincible) {
			(cur_invicible_time % 20) ? pCamera->canvas_size += 0.4*DELTA_TIME : pCamera->canvas_size -= 0.4 * DELTA_TIME;
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
				pCamera->x += obstacles[stading_index]->speed;
			}
			else if (obstacles[stading_index]->state_x == LEFT_STATE)
			{
				x -= obstacles[stading_index]->speed;
				pCamera->x -= obstacles[stading_index]->speed;
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
			x += speed + x_additional_speed; pCamera->x += speed + x_additional_speed;
			if (x + width > ground->x + ground->width *GROUND_NUM)
			{
				back_distance = (x + width) - (ground->x + ground->width*GROUND_NUM);
				x -= back_distance;
				pCamera->x -= back_distance;
			}
			else
			{
				for (int i = 0; i < obCnt; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || (obstacles[i]->type == PUMKIN && is_under == false))
					{
						if (AABB(obstacles[i]))
						{
							back_distance = x + width - obstacles[i]->x;
							x -= back_distance;
							pCamera->x -= back_distance;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							back_distance = x + width - obstacles[i]->x;
							x -= back_distance;
							pCamera->x -= back_distance;
							break;
						}
					}
				}
			}
		}
		if (state[LEFT_STATE])
		{
			x -= (speed + x_additional_speed); pCamera->x -= (speed + x_additional_speed);
			if (x < 0)
			{
				x = 0;
				pCamera->x = 0;
			}
			else
			{
				for (int i = 0; i < obCnt; ++i)
				{
					if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || (obstacles[i]->type == PUMKIN && is_under == false))
					{
						if (AABB(obstacles[i]))
						{
							back_distance = obstacles[i]->x + obstacles[i]->width - x;
							x += back_distance;
							pCamera->x += back_distance;
							break;
						}
					}
					else if (obstacles[i]->type == HAY)
					{
						if (obstacles[i]->AABB_surface(this))
						{
							back_distance = obstacles[i]->x + obstacles[i]->width - x;
							x += back_distance;
							pCamera->x += back_distance;
							break;
						}
					}
				}
			}
		}
		if (state[UP_STATE] && pCamera->view_point == DOWN_VIEW)
		{
			z += (speed + z_additional_speed);
			if (z + depth > ground->z + ground->depth - 5)
			{
				back_distance = (z + depth) - (ground->z + ground->depth - 5);
				z -= back_distance;
			}
			else
			{
				for (int i = 0; i < obCnt; ++i)
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
		if (state[DOWN_STATE] && pCamera->view_point == DOWN_VIEW)
		{
			z -= (speed + z_additional_speed);
			if (z < ground->z + 10)
			{
				back_distance = ground->z - z + 10;
				z += back_distance;
			}
			else
			{
				for (int i = 0; i < obCnt; ++i)
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
				for (int i = 0; i < obCnt; ++i)
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
				for (int i = 0; i < obCnt; ++i)
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
						FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[KILL_E], 0, &pSound->Channel[KILL_E]);
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
				for (int i = 0; i < obCnt; ++i)
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
						FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[KILL_E], 0, &pSound->Channel[KILL_E]);
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
void Sheep::special_key(int key, Object* obstacles[])
{
	if (key == GLUT_KEY_RIGHT) {
		state[RIGHT_STATE] = true;
	}
	else if (key == GLUT_KEY_LEFT) {
		state[LEFT_STATE] = true;
	}
	else if (key == GLUT_KEY_UP) {
		if (pCamera->view_point == DOWN_VIEW)
		{
			state[UP_STATE] = true;
		}
		else if (pCamera->view_point == FRONT_VIEW && state[JUMP_DOWN_STATE] == false && state[JUMP_UP_STATE] == false && state[GRAVITY] == false && *pGameMode == PLAY_MODE)
		{
			state[JUMP_UP_STATE] = true;
			org_y = y;
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[JUMP_E], 0, &pSound->Channel[JUMP_E]);
		}
	}
	else if (key == GLUT_KEY_DOWN) {
		if (pCamera->view_point == DOWN_VIEW)
		{
			state[DOWN_STATE] = true;
		}
	}
}
void Sheep::special_key_up(int key)
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

void Sheep::setSound(SoundPackage *sound)
{
	pSound = sound;
	pCamera->pSound = sound;
}


Box::Box(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 0, 0, 0, sp, m_x, m_y, m_z) {
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
Box::~Box() {  }
void Box::draw()
	{
		glColor3f(1, 1, 1);

		//앞면
		glEnable(GL_TEXTURE_2D);
		if (type == BOX) { glBindTexture(GL_TEXTURE_2D, pTextures[BOX_FRONT]); }
		else if (type == BRICK) { glBindTexture(GL_TEXTURE_2D, pTextures[BRICK_FRONT]); }
		else if (type == BOXWALL) { glBindTexture(GL_TEXTURE_2D, pTextures[BOXWALL_FRONT]); }
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
		if (type == BOX) { glBindTexture(GL_TEXTURE_2D, pTextures[BOX_TOP]); }
		else if (type == BRICK) { glBindTexture(GL_TEXTURE_2D, pTextures[BRICK_TOP]); }
		else if (type == BOXWALL) { glBindTexture(GL_TEXTURE_2D, pTextures[BOXWALL_TOP]); }
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
bool Box::is_standing(const Object* other)
	{
		if (y + height != other->y) return false;
		if (x + width <= other->x) return false;
		if (x >= other->x + other->width) return false;
		if (z + depth <= other->z) return false;
		if (z >= other->z + other->depth) return false;
		return true;
	}
void Box::update1(Sheep* sheep)
	{
		if (state_x == RIGHT_STATE)
		{
			x += speed;
			if (AABB(sheep))
			{
				sheep->x += speed;
				pCamera->x += speed;
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
				pCamera->x -= speed;
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


Scissors::Scissors(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 40, 8, 35, sp, m_x, m_y, m_z) {
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
Scissors::~Scissors() {  }
void Scissors::draw()
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
void Scissors::update1(Sheep* sheep)
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


Pumkin::Pumkin(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 50, 50, 50, sp, m_x, m_y, m_z) {
		max_y = m_y;
		org_y = y;
		if (max_y > 0) state_y = JUMP_UP_STATE;
		else if (max_y < 0) state_y = JUMP_DOWN_STATE;
		else state_y = STOP_STATE;
	}
Pumkin::~Pumkin() {  }
void Pumkin::draw() 
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
void Pumkin::update1(Sheep* sheep)
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
bool Pumkin::is_standing(const Object* other)
	{
		if (y + height != other->y) return false;
		if (x + width <= other->x) return false;
		if (x >= other->x + other->width) return false;
		if (z + depth <= other->z) return false;
		if (z >= other->z + other->depth) return false;
		return true;
	}


Hay::Hay(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 100, 70, 100, sp, m_x, m_y, m_z) {
	}
Hay::~Hay() {  }
bool Hay::AABB_surface(const Object* other) 
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
bool Hay::is_inside(Sheep* sheep)
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
void Hay::draw()
	{
		glColor3f(1, 1, 1);
		//앞면
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pTextures[HAY_FRONT]);
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
		glBindTexture(GL_TEXTURE_2D, pTextures[HAY_TOP]);

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


Black_Sheep::Black_Sheep(int t, float x, float y, float z, float sp = 0, int area_of_activity = 0, int none1 = 0, int none2 = 0) : Object(t, x, y, z, 55, 35, 55, sp, area_of_activity, none1, none2) {
		org_x = x;
		org_z = z;
		tracing_distance = area_of_activity;
		view_rad = 0;
		is_tracing = false;
		ouch = 0;
		wait_time = 11;
	}
Black_Sheep::~Black_Sheep() { }
void Black_Sheep::draw() 
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
void Black_Sheep::trace_return(Sheep* sheep, Object* obstacles[])
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
					for (int i = 0; i < OB_CNT; ++i)
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
void Black_Sheep::update2(Sheep* sheep, Object* obstacles[])
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
void MotherSheep::draw()
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


Ui::Ui(int size) : canvas_size(size), selected_menu(0), heart_size(0.5), heart_dir(1), presskey(false), help(0) {

		for (int i = 0; i < 2; ++i)
			key_delay[i] = 0;
	}
Ui::~Ui() { }
int Ui::keyboard(unsigned char key, Sheep* sheep)
	{
		if (*pGameMode == MAIN_MODE && selected_menu == 1 && key_delay[0] == 0)
		{
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E]);
			key_delay[0] = 3;
			help = (help + 1) % 3;
		}
		//if (key == 27 && key_delay[0] == 0) // ESC
		//{
		//	if (Game_Mode == PLAY_MODE)
		//	{
		//		key_delay[0] = 5;
		//		FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
		//		FMOD_Channel_SetPaused(g_Channel[GAME_BGM], true);
		//		selected_menu = 0;
		//		Game_Mode = PAUSE_MODE;
		//	}
		//	else if (Game_Mode == PAUSE_MODE)
		//	{
		//		key_delay[0] = 5;
		//		FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
		//		FMOD_Channel_SetPaused(g_Channel[GAME_BGM], false);
		//		selected_menu = 0;
		//		Game_Mode = PLAY_MODE;
		//	}
		//}
		if ((key == ' ' || key == 13) && key_delay[0] == 0)
		{
			if (*pGameMode == MAIN_MODE)
			{
				FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E]);
				if (presskey == false)
				{
					FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E]);
					presskey = true;
				}
				else if (selected_menu == 0)
				{
					//NetworkManager.getReady();
					//게임시작
					//추가필요
					//CreateWorld();
					FMOD_Channel_Stop(pSound->Channel[MAIN_BGM]);
					FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[GAME_BGM], 0, &pSound->Channel[GAME_BGM]);
					FMOD_Channel_SetVolume(pSound->Channel[GAME_BGM], GAME_BGM_VOLUME);
					FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CRY_E], 0, &pSound->Channel[CRY_E]);
					return READY_MODE;
				}
				else if (selected_menu == 2)
				{
					//종료
					exit(0);
				}
			}
			else if (*pGameMode == PAUSE_MODE || sheep->killed || ending_screen == 3)
			{
				if (ending_screen == 3)
				{
					FMOD_Channel_Stop(pSound->Channel[CLEAR_BGM]);
					ending_screen = 0;
					sheep->ending_finished = false;
				}
				FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E]);
				if (selected_menu == 0) {
					//이어하기
					FMOD_Channel_SetPaused(pSound->Channel[GAME_BGM], false);
					*pGameMode = PLAY_MODE;
				}
				else if (selected_menu == 1)
				{
					FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[GAME_BGM], 0, &pSound->Channel[GAME_BGM]);
					FMOD_Channel_SetVolume(pSound->Channel[GAME_BGM], GAME_BGM_VOLUME);
					//다시하기
					//추가필요
					//DestroyWorld();
					//CreateWorld();
					*pGameMode = PLAY_MODE;
				}
				else if (selected_menu == 2)
				{
					//메인메뉴로
					//추가필요
					//DestroyWorld();
					*pGameMode = MAIN_MODE;
					FMOD_Channel_Stop(pSound->Channel[GAME_BGM]);
					FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[MAIN_BGM], 0, &pSound->Channel[MAIN_BGM]);
				}
				selected_menu = 0;
			}
		}
		if (presskey == false)
		{
			presskey = true;
		}

		return -1;
	}
void Ui::special_key(int key)
	{
		if (presskey == false)
		{
			FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E]);
			presskey = true;
		}
		else
		{
			if ((*pGameMode == MAIN_MODE || *pGameMode == PAUSE_MODE) && (key_delay[1] == 0))
			{
				key_delay[1] = 3;
				FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_MOVE_E], 0, &pSound->Channel[BUTTON_MOVE_E]);
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
			//추가필요
			//else if ((key_delay[1] == 0) && (sheep->killed || ending_screen == 3))
			else if ((key_delay[1] == 0) && ending_screen == 3)
			{
				key_delay[1] = 3;
				FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_MOVE_E], 0, &pSound->Channel[BUTTON_MOVE_E]);
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
void Ui::draw(Sheep* sheep)
	{
		glPushMatrix();
		glOrtho(-canvas_size, canvas_size, -canvas_size, canvas_size, 1000, -1000);

		// 메인 메뉴
		if (*pGameMode == MAIN_MODE)
		{
			glPushMatrix();
			glColor3f(1, 1, 1);
			int x = -200, y = -200, z = -500;
			int width = 400, height = 400;
			glEnable(GL_TEXTURE_2D);
			if (presskey == false) { glBindTexture(GL_TEXTURE_2D, pTextures[MAIN_0]); }
			else if (selected_menu == 0) { glBindTexture(GL_TEXTURE_2D, pTextures[MAIN_1]); }
			else if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[MAIN_2]); }
			else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[MAIN_3]); }
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
				if (help == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[HELP_0]); }
				else if (help == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[HELP_1]); }
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
			if (*pGameMode == ENDING_MODE)
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
					if (ending_screen == 0) { glBindTexture(GL_TEXTURE_2D, pTextures[ENDING_0]); }
					else if (ending_screen == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[ENDING_1]); }
					else if (ending_screen == 3)
					{
						if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[ENDING_MENU_0]); }
						else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[ENDING_MENU_1]); }
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

			if (*pGameMode == PLAY_MODE)
			{
				//ESC
				glPushMatrix();
				glColor3f(1, 1, 1);
				int x = 140, y = 140, z = -500;
				int width = 50, height = 50;
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, pTextures[ESC_BUTTON]);
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
			if (*pGameMode == PAUSE_MODE)
			{
				glPushMatrix();
				glColor3f(1, 1, 1);
				int x = -70, y = -130, z = -500;
				int width = 155, height = 250;
				glEnable(GL_TEXTURE_2D);
				if (selected_menu == 0) { glBindTexture(GL_TEXTURE_2D, pTextures[MENU_0]); }
				else if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[MENU_1]); }
				else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[MENU_2]); }
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
				if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[DEAD_0]); }
				else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[DEAD_1]); }
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
void Ui::update()
	{
		if ((*pGameMode == GAME_OVER || *pGameMode == ENDING_MODE) && selected_menu == 0)
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

