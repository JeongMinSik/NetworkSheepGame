#include "Objects.h"
#include "Protocol.h"

Camera::Camera()
{
	x = 9300; y = 100;
	canvas_size = 200;
	view_radius = 0;
	view_point = FRONT_VIEW;
	is_changing = false;
}
Camera::~Camera() {  }

void Camera::keyboard(unsigned char key)
{
	if (key == ' ' && is_changing == false) {
		is_changing = true;
	}
}
void Camera::update(float frameTime)
{
	// 카메라 회전
	if (is_changing)
	{
		if (view_point == FRONT_VIEW)
		{
			view_radius -= 0.2f * frameTime;
			if (view_radius <= -90)
			{
				is_changing = false;
				view_radius = -90;
				view_point = DOWN_VIEW;
			}
		}
		else if (view_point == DOWN_VIEW)
		{
			view_radius += 0.2f * frameTime;
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


Object::Object(int type, float x, float y, float z, float w, float h, float d, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : x(x), y(y), z(z), width(w), height(h), depth(d), type(type), speed(sp*0.03), max_x(m_x), max_y(m_y), max_z(m_z) {}
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
	if (life < 1) {
		iGameMode = GAME_OVER;

	}
}
void Sheep::dead_update(float frameTime)
{
	pCamera->is_changing = false;

	//죽은 양
	if (abs(pCamera->view_radius + 40.0) > 2.0)
		(pCamera->view_radius < -40) ? pCamera->view_radius += 0.25*frameTime : pCamera->view_radius -= 0.25*frameTime;
	else if (y <= 500)
	{
		y += speed*frameTime;

		if (y > 500) {
			y = 500;
			killed = true;
		}
	}
}

void Sheep::update2(const Ground* ground, Object* obstacles[], float frameTime)
{
	//엔딩
	if (iGameMode != ENDING_MODE && x > ENDING_X){
		iGameMode = ENDING_MODE;
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
		//(cur_invicible_time % 2) ? pCamera->canvas_size += 0.4*frameTime : pCamera->canvas_size -= 0.4 * frameTime;
		cur_invicible_time += frameTime;
		if (cur_invicible_time >= max_invicible_time) {
			is_invincible = false;
			is_under = false;
			cur_invicible_time = 0;
		}
	}

	float back_distance; // 충돌 시 되돌아오는 거리값

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
			x += obstacles[stading_index]->speed*frameTime;
			pCamera->x += obstacles[stading_index]->speed*frameTime;
		}
		else if (obstacles[stading_index]->state_x == LEFT_STATE)
		{
			x -= obstacles[stading_index]->speed*frameTime;
			pCamera->x -= obstacles[stading_index]->speed*frameTime;
		}
		if (obstacles[stading_index]->state_z == UP_STATE)
		{
			z += obstacles[stading_index]->speed*frameTime;
		}
		else if (obstacles[stading_index]->state_z == DOWN_STATE)
		{
			z -= obstacles[stading_index]->speed*frameTime;
		}
		if (obstacles[stading_index]->state_y == JUMP_UP_STATE)
		{
			y += obstacles[stading_index]->speed*frameTime;
		}
		else if (obstacles[stading_index]->state_y == JUMP_DOWN_STATE)
		{
			y -= obstacles[stading_index]->speed*frameTime;
		}
	}
	else
	{
		//추가 이동속력 초기화
		x_additional_speed = z_additional_speed = 0;

		//중력
		if (state[JUMP_UP_STATE] == false && state[JUMP_DOWN_STATE] == false && y > 0)
		{
			//추가속도 및 점프감소력 초기화
			y_additional_speed = minus_height = 0;
			state[GRAVITY] = true;
			y -= speed*1.2*frameTime;
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
						int last_y = y + speed*1.2*frameTime;
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


	// 기본이동 및 충돌체크
	if (state[RIGHT_STATE])
	{
		x += (speed + x_additional_speed)*frameTime;
		pCamera->x += (speed + x_additional_speed)*frameTime;
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
		x -= (speed + x_additional_speed)*frameTime;
		pCamera->x -= (speed + x_additional_speed)*frameTime;
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
		z += (speed + z_additional_speed)*frameTime;
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
						int last_z = z - (speed + z_additional_speed)*frameTime;
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
		z -= (speed + z_additional_speed)*frameTime;
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
						int last_z = z + (speed + z_additional_speed)*frameTime;
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
		y += (speed + y_additional_speed)*frameTime;
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
						int last_y = y - (speed + y_additional_speed)*frameTime;
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
		y -= speed*1.2*frameTime;
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
						int last_y = y + speed*1.2*frameTime;
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
					obstacles[i]->killed = true;
					state[JUMP_DOWN_STATE] = false;
					state[JUMP_UP_STATE] = true;
					org_y = y;
				}
			}
		}
	}

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
		else if (pCamera->view_point == FRONT_VIEW && state[JUMP_DOWN_STATE] == false && state[JUMP_UP_STATE] == false && state[GRAVITY] == false)
		{
			state[JUMP_UP_STATE] = true;
			org_y = y;
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
bool Box::is_standing(const Object* other)
{
	if (y + height != other->y) return false;
	if (x + width <= other->x) return false;
	if (x >= other->x + other->width) return false;
	if (z + depth <= other->z) return false;
	if (z >= other->z + other->depth) return false;
	return true;
}
void Box::update1(Sheep** sheeps, float frameTime)
{
	if (state_x == RIGHT_STATE)
	{
		x += speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->x += speed*frameTime;
				pCamera->x += speed*frameTime;
			}
		}

		if (abs(x - org_x) >= abs(max_x)) {
			state_x = LEFT_STATE;
			x = org_x + max_x - speed*frameTime;
		}
	}
	if (state_x == LEFT_STATE)
	{
		x -= speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->x -= speed*frameTime;
				pCamera->x -= speed*frameTime;
			}
		}
		if (abs(x - org_x) >= abs(max_x)) {
			state_x = RIGHT_STATE;
			x = org_x - max_x + speed*frameTime;
		}
	}
	if (state_z == UP_STATE)
	{
		z += speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->z += speed*frameTime;
			}
		}
		if (abs(z - org_z) >= abs(max_z))
		{
			state_z = DOWN_STATE;
			z -= 2 * speed*frameTime;
		}
	}
	else if (state_z == DOWN_STATE)
	{
		z -= speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->z -= speed*frameTime;
			}
		}
		if (abs(z - org_z) >= abs(max_z))
		{
			state_z = UP_STATE;
			z += 2 * speed*frameTime;
		}
	}
	if (state_y == JUMP_UP_STATE)
	{
		y += speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->y += speed*frameTime;
			}
		}
		if (abs(y - org_y) >= abs(max_y))
		{
			state_y = JUMP_DOWN_STATE;
			y -= 2 * speed*frameTime;
		}
	}
	else if (state_y == JUMP_DOWN_STATE)
	{
		y -= speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->y -= speed*frameTime;
			}
		}
		if (abs(y - org_y) >= abs(max_y))
		{
			state_y = JUMP_UP_STATE;
			y += 2 * speed*frameTime;
		}
	}
}


Scissors::Scissors(int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0) : Object(t, x, y, z, 40, 8, 35, sp, m_x, m_y, m_z) {
	max_x = m_x;	max_y = m_y;	max_z = m_z;
	org_x = x;		org_y = y;		org_z = z;
	Rotate_y = 0;
	scissor_rot = 0.1;
	if (max_x > 0) state_x = RIGHT_STATE;
	else if (max_x < 0) state_x = LEFT_STATE;
	else state_x = STOP_STATE;
	if (max_z > 0) state_z = UP_STATE;
	else if (max_z < 0) state_z = DOWN_STATE;
	else state_z = STOP_STATE;
}
Scissors::~Scissors() {  }
void Scissors::update1(Sheep** sheeps, float frameTime)
{
	Rotate_y += scissor_rot*frameTime;
	if (Rotate_y >= 15)
		scissor_rot *= -1;
	if (Rotate_y <= -5)
		scissor_rot *= -1;

	if (state_x == RIGHT_STATE)
	{
		x += speed*frameTime;
		if (abs(x - org_x) >= abs(max_x)) state_x = LEFT_STATE;
	}
	if (state_x == LEFT_STATE)
	{
		x -= speed*frameTime;
		if (abs(x - org_x) >= abs(max_x)) state_x = RIGHT_STATE;
	}
	if (state_z == UP_STATE)
	{
		z += speed*frameTime;
		if (abs(z - org_z) >= abs(max_z))
		{
			state_z = DOWN_STATE;
		}
	}
	else if (state_z == DOWN_STATE)
	{
		z -= speed*frameTime;
		if (abs(z - org_z) >= abs(max_z))
		{
			state_z = UP_STATE;
		}
	}
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (sheeps[i]->is_invincible == false && AABB(sheeps[i])) {
			sheeps[i]->get_hurt();
		}
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
void Pumkin::update1(Sheep** sheeps, float frameTime)
{
	if (state_y == JUMP_UP_STATE)
	{
		y += speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->y += speed*frameTime;
			}
		}
		if (abs(y - org_y) >= abs(max_y))
		{
			state_y = JUMP_DOWN_STATE;
			y -= (2 * speed*frameTime);
		}
	}
	else if (state_y == JUMP_DOWN_STATE)
	{
		y -= speed*frameTime;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (AABB(sheeps[i])) {
				sheeps[i]->y -= speed*frameTime;
				if (sheeps[i]->y < 0)
				{
					sheeps[i]->y = 0;
					if (sheeps[i]->is_invincible == false)
					{
						sheeps[i]->is_under = true;
						sheeps[i]->get_hurt();
					}
				}
			}
		}
		if (abs(y - org_y) >= abs(max_y))
		{
			state_y = JUMP_UP_STATE;
			y += (2 * speed*frameTime);
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
void Black_Sheep::trace_return(Sheep** sheeps, Object* obstacles[], float frameTime)
{
	// 가장 거리가 가까운 양 계산
	int sx, sz;
	float d;
	int minIndex = 0;
	float min = tracing_distance + 1;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		sx = sheeps[i]->x; sz = sheeps[i]->z;
		d = sqrt(float(sx - x)*float(sx - x) + float(sz - z)*float(sz - z));
		if (d < min && !sheeps[minIndex]->is_in_hay) {
			min = d;
			minIndex = i;
		}
	}
	sx = sheeps[minIndex]->x; sz = sheeps[minIndex]->z;
	d = min;

	// 원위치와의 계산
	float org_d = sqrt(float(org_x - x)*float(org_x - x) + float(org_z - z)*float(org_z - z));
	int vx, vz; // 이동량

	if (is_tracing)
	{
		// 추격 종료
		if (org_d > tracing_distance || d > tracing_distance || ouch > 1000) {
			is_tracing = false;
		}
		else {
			// 바라보는 각도 계산
			view_rad = atan2(sz - z, sx - x) * 180 / 3.1415926535;

			// 이동
			if (d > speed*frameTime)
			{
				vx = (sx - x) / d*speed*frameTime;
				vz = (sz - z) / d*speed*frameTime;
			}
			else
			{
				vx = 0;
				vz = 0;
			}
			x += vx;
			z += vz;

			// 양과의 충돌체크
			for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
				if (AABB(sheeps[i]))
				{
					x -= vx;
					z -= vz;
					if (sheeps[i]->is_invincible == false) {
						sheeps[i]->get_hurt();
					}
					return;
				}
			}

			// 타 장애물들과의 충돌체크
			for (int i = 0; i < OB_CNT; ++i)
			{
				if (obstacles[i] == nullptr) break;
				if (obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN || obstacles[i]->type == HAY)
				{
					if (AABB(obstacles[i]))
					{
						ouch += frameTime;
						x -= vx;
						z -= vz;
						break;
					}
				}
			}
		}
	}
	else
	{
		wait_time += frameTime;
		if (wait_time > 1000) {
			ouch = 0;
			// 바라보는 각도 계산
			view_rad = atan2(org_z - z, org_x - x) * 180 / 3.1415926535;

			if (org_d > speed*frameTime) {
				vx = (org_x - x) / org_d*speed*frameTime;
				vz = (org_z - z) / org_d*speed*frameTime;
			}
			else {
				vx = 0;
				vz = 0;
			}
			x += vx;
			z += vz;
		}
		//추격 재개
		else if (org_d <= tracing_distance && d <= tracing_distance && ouch <= 1000)
		{
			is_tracing = true;
			wait_time = 0;
		}

		// 원점으로 돌아옴
		if (speed*frameTime >= abs(org_x - x) && speed*frameTime >= abs(org_z - z) && d <= tracing_distance && sheeps[minIndex]->y == y)
		{
			is_tracing = true;
			wait_time = 0;
		}
	}

}
void Black_Sheep::update3(Sheep** sheeps, Object* obstacles[], float frameTime)
{
	if (killed)
	{
		if (height > 0) {
			y--;
			height -= sheeps[0]->speed*frameTime * 2;
			//y_scale += 0.01*frameTime;
		}
	}
	else { trace_return(sheeps, obstacles, frameTime); }
}
