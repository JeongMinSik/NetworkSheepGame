#include "stdafx.h"
#include "Sheep.h"
#include "SoundPackage.h"
#include "Camera.h"
#include "Ground.h"

Sheep::Sheep( int t, int x, int y, int z, float sp ) : Object( t, x, y, z, 45, 30, 35, sp ) {
	jump_height = 100;
	x_additional_speed = y_additional_speed = z_additional_speed = 0;
	last_view = 0;
	life = 3;
	is_invincible = is_under = is_in_hay = false;
	max_invicible_time = 2000;
	cur_invicible_time = 0;
	stading_index = -1;
	pSelectedSheep = nullptr;
	for( int i = 0; i < 8; ++i )
		state[i] = false;
}
Sheep::~Sheep() {
	delete pCamera;
}
void Sheep::get_hurt()
{
	is_invincible = true;
	cur_invicible_time += max_invicible_time;
	--life;
	if( life < 1 ) {
		iGameMode = GAME_OVER;
		if( isCloseFromSelectedSheep() ) {
			if( this == pSelectedSheep ) {
				FMOD_Channel_Stop( pSound->Channel[GAME_BGM] );
			}
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CRY_E], 0, &pSound->Channel[CRY_E] );
		}
	}
	else {
		if( isCloseFromSelectedSheep() ) {
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[GET_HURT_E], 0, &pSound->Channel[GET_HURT_E] );
		}
	}
}
void Sheep::dead_update( float frameTime )
{
	pCamera->is_changing = false;

	//죽은 양
	if( abs( pCamera->view_radius + 40.0 ) > 2.0 )
		( pCamera->view_radius < -40 ) ? pCamera->view_radius += 0.25*frameTime : pCamera->view_radius -= 0.25*frameTime;
	else if( y <= 500 )
	{
		y += speed*frameTime;

		if( y > 500 ) {
			y = 500;
			killed = true;
		}
	}
}
void Sheep::ending_update( float frameTime )
{

	pCamera->is_changing = false;

	static int dir = 1, jump_cnt;
	static bool bsound = true;
	const int JUMP_MAX = 3;

	// 카메라이동
	if( abs( pCamera->view_radius + 30.0 ) > 2.0 ) {
		( pCamera->view_radius < -30 ) ? pCamera->view_radius += 0.25*frameTime : pCamera->view_radius -= 0.25*frameTime;
		dir = 1, jump_cnt = 0;
		bsound = true;
	}

	// 이동
	int aim_x = 9600, aim_z = 78;
	int vx, vz; // 이동량
	float d = sqrt( float( aim_x - x )*float( aim_x - x ) + float( aim_z - z )*float( aim_z - z ) );

	last_view = -atan2( aim_z - z, aim_x - x ) * 180 / 3.1415926535;

	if( d > speed*frameTime )
	{
		vx = ( aim_x - x ) / d*speed*frameTime;
		vz = ( aim_z - z ) / d*speed*frameTime;
		y -= speed*frameTime;
		if( y < 0 ) { y = 0; }
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

	if( vx == 0 && vz == 0 && jump_cnt < JUMP_MAX )
	{
		if( bsound )
		{
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[JUMP_E], 0, &pSound->Channel[JUMP_E] );
			bsound = false;
		}

		y += speed*dir*frameTime;
		if( y > jump_height )
			dir = -1;
		if( y < 0 )
		{
			bsound = true;
			y = 0;
			dir = 1;
			++jump_cnt;
		}
	}
	else if( jump_cnt == JUMP_MAX ) { ending_finished = true; }

}
void Sheep::draw()
{
	glPushMatrix();
	glTranslated( ( x + width / 2 ), ( y + height / 2 ) + 5, z + depth / 2 );
	if( last_view == 0 )
	{
		glTranslated( -5, 0, 0 );
	}
	else if( last_view == 180 )
	{
		glTranslated( 5, 0, 0 );
	}
	else if( last_view == 90 )
	{
		glTranslated( 0, 0, 6 );
	}
	else if( last_view == 270 )
	{
		glTranslated( 0, 0, -6 );
	}
	glRotated( last_view, 0, 1, 0 );
	glScalef( 0.25, 0.3, 0.3 );

	//호박에 깔림
	if( is_under )
	{
		glScalef( 1, 0.5, 1 );
		glTranslated( 0, -60, 0 );
	}

	glPushMatrix();
	glColor3f( 1, 0.9, 0.9 );
	glTranslatef( 70, -20, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	glPushMatrix();
	glColor3f( 1, 0.8, 0.8 );
	glTranslatef( 10, -20, 0 );
	glScalef( 2, 1, 1 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	// 몸통이
	glPushMatrix();
	glColor3f( 1, 1, 1 );

	//무적애니메이션
	if( is_invincible ) {
		if( cur_invicible_time % 2 == 0 ) {
			glColor3f( 1, 0, 0 );
		}
		else {
			glColor3f( 1, 1, 1 );
		}
	}

	glTranslatef( 30, 0, 30 );
	glutSolidSphere( 50, 16, 16 );

	glTranslatef( -50, 0, 0 );
	glutSolidSphere( 50, 16, 16 );

	glTranslatef( -20, -10, 0 );
	glutSolidSphere( 40, 16, 16 );

	glTranslatef( -10, -20, 0 );
	glutSolidSphere( 40, 16, 16 );

	glTranslatef( 30, -10, 0 );
	glutSolidSphere( 50, 16, 16 );

	glTranslatef( 40, 0, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef( 30, 0, -30 );
	glutSolidSphere( 50, 16, 16 );

	glTranslatef( -50, 0, 0 );
	glutSolidSphere( 50, 16, 16 );

	glTranslatef( -20, -10, 0 );
	glutSolidSphere( 40, 16, 16 );

	glTranslatef( 10, -20, 0 );
	glutSolidSphere( 40, 16, 16 );

	glTranslatef( 30, 0, 0 );
	glutSolidSphere( 50, 16, 16 );

	glTranslatef( 40, 0, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef( -40, -20, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef( 30, 10, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef( -50, -30, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	// 눈
	glPushMatrix();
	if( is_invincible )
	{
		glPushMatrix();
		glColor3f( 0, 0, 0 );
		glRotated( -3, 0, 1, 0 );
		glTranslatef( 90, 0, 35 );
		glRotated( 45, 0, 0, 1 );
		glScaled( 1, 4, 1 );
		glutSolidCube( 5 );
		glPopMatrix();

		glPushMatrix();
		glColor3f( 0, 0, 0 );
		glRotated( -3, 0, 1, 0 );
		glTranslatef( 90, 0, 38 );
		glRotated( -45, 0, 0, 1 );
		glScaled( 1, 4, 1 );
		glutSolidCube( 5 );
		glPopMatrix();
		glTranslated( 0, 0, -85 );
		glPushMatrix();
		glColor3f( 0, 0, 0 );
		glRotated( -3, 0, 1, 0 );
		glTranslatef( 90, 0, 35 );
		glRotated( 45, 0, 0, 1 );
		glScaled( 1, 4, 1 );
		glutSolidCube( 5 );
		glPopMatrix();

		glPushMatrix();
		glColor3f( 0, 0, 0 );
		glRotated( -3, 0, 1, 0 );
		glTranslatef( 90, 0, 38 );
		glRotated( -45, 0, 0, 1 );
		glScaled( 1, 4, 1 );
		glutSolidCube( 5 );
		glPopMatrix();
	}
	else
	{
		glColor3f( 0, 0, 0 );
		glTranslatef( 90, 0, 35 );
		glutSolidSphere( 7, 16, 16 );
		glTranslatef( 0, 0, -70 );
		glutSolidSphere( 7, 16, 16 );
	}
	glPopMatrix();

	//천사링
	if( iGameMode == GAME_OVER )
	{
		glPushMatrix();
		glColor3f( 0.97, 0.97, 0.97 );
		glTranslatef( 50, 100, 0 );
		glRotated( 90, 1, 0, 0 );
		glutSolidTorus( 10, 40, 20, 20 );
		glPopMatrix();
	}

	glPopMatrix();
}
void Sheep::update2( const Ground* ground, Object* obstacles[], float frameTime )
{
	// 바라보는 방향
	if( state[RIGHT_STATE] )
	{
		last_view = 0;
		if( state[UP_STATE] ) last_view -= 45;
		if( state[DOWN_STATE] ) last_view += 45;
	}
	else if( state[LEFT_STATE] )
	{
		last_view = 180;
		if( state[UP_STATE] ) last_view += 45;
		if( state[DOWN_STATE] ) last_view -= 45;
	}
	else if( state[DOWN_STATE] )
		last_view = 90;
	else if( state[UP_STATE] )
		last_view = 270;

	//지푸라기 안쪽 상태
	for( int i = 0; i < obCnt; ++i )
	{
		if( obstacles[i]->type == HAY
			&& obstacles[i]->is_inside( this ) )
		{
			is_in_hay = true;
			break;
		}
		is_in_hay = false;
	}

	//무적상태
	if( is_invincible ) {
		//(cur_invicible_time % 2) ? pCamera->canvas_size += 0.4*frameTime : pCamera->canvas_size -= 0.4 * frameTime;
		cur_invicible_time -= frameTime;
		if( cur_invicible_time < 0 ) {
			cur_invicible_time = 0;
			is_invincible = false;
			is_under = false;
		}
	}

	float back_distance; // 충돌 시 되돌아오는 거리값

						 //스탠딩 상태
	if( stading_index >= 0 ) {

		// 추가 이동속도 변경
		if( obstacles[stading_index]->state_y == JUMP_UP_STATE )
			y_additional_speed = obstacles[stading_index]->speed;

		if( obstacles[stading_index]->state_x == RIGHT_STATE && state[LEFT_STATE] )
			x_additional_speed = obstacles[stading_index]->speed;
		else if( obstacles[stading_index]->state_x == LEFT_STATE && state[RIGHT_STATE] )
			x_additional_speed = obstacles[stading_index]->speed;
		else
			x_additional_speed = 0;
		if( obstacles[stading_index]->state_z == UP_STATE && state[DOWN_STATE] )
			z_additional_speed = obstacles[stading_index]->speed;
		else if( obstacles[stading_index]->state_z == DOWN_STATE && state[UP_STATE] )
			z_additional_speed = obstacles[stading_index]->speed;
		else
			z_additional_speed = 0;

		//이동
		if( obstacles[stading_index]->state_x == RIGHT_STATE )
		{
			x += obstacles[stading_index]->speed*frameTime;
			pCamera->x += obstacles[stading_index]->speed*frameTime;
		}
		else if( obstacles[stading_index]->state_x == LEFT_STATE )
		{
			x -= obstacles[stading_index]->speed*frameTime;
			pCamera->x -= obstacles[stading_index]->speed*frameTime;
		}
		if( obstacles[stading_index]->state_z == UP_STATE )
		{
			z += obstacles[stading_index]->speed*frameTime;
		}
		else if( obstacles[stading_index]->state_z == DOWN_STATE )
		{
			z -= obstacles[stading_index]->speed*frameTime;
		}
		if( obstacles[stading_index]->state_y == JUMP_UP_STATE )
		{
			y += obstacles[stading_index]->speed*frameTime;
		}
		else if( obstacles[stading_index]->state_y == JUMP_DOWN_STATE )
		{
			y -= obstacles[stading_index]->speed*frameTime;
		}
	}
	else
	{
		//추가 이동속력 초기화
		x_additional_speed = z_additional_speed = 0;

		//중력
		if( state[JUMP_UP_STATE] == false && state[JUMP_DOWN_STATE] == false && y > 0 )
		{
			//추가속도 및 점프감소력 초기화
			y_additional_speed = 0;
			state[GRAVITY] = true;
			y -= speed*1.2*frameTime;
			if( y <= 0 )
			{
				y = 0;
				state[GRAVITY] = false;
			}
			else {
				for( int i = 0; i < obCnt; ++i )
				{
					if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN )
					{
						if( AABB( obstacles[i] ) )
						{
							back_distance = obstacles[i]->y + obstacles[i]->height - y;
							y += back_distance;
							state[GRAVITY] = false;
							break;
						}
					}
					else if( obstacles[i]->type == HAY )
					{
						int last_y = y + speed*1.2*frameTime;
						if( obstacles[i]->AABB_surface( this ) )
						{
							if( last_y < obstacles[i]->y || last_y + height > obstacles[i]->y + obstacles[i]->height )
							{
								back_distance = obstacles[i]->y + obstacles[i]->height - y;
								y += back_distance;
								state[GRAVITY] = false;
								break;
							}
						}
					}
					else if( obstacles[i]->type == BLACK_SHEEP && AABB( obstacles[i] ) )
					{
						if( isCloseFromSelectedSheep() ) {
							FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[KILL_E], 0, &pSound->Channel[KILL_E] );
						}
						obstacles[i]->killed = true;
						state[JUMP_UP_STATE] = true;
						state[GRAVITY] = false;
						org_y = y;
					}
				}
			}
		}
		else if( y == 0 ) state[GRAVITY] = false;
	}


	// 기본이동 및 충돌체크
	if( state[RIGHT_STATE] )
	{
		x += ( speed + x_additional_speed )*frameTime;
		pCamera->x += ( speed + x_additional_speed )*frameTime;
		if( x + width > ground->x + ground->width *GROUND_NUM )
		{
			back_distance = ( x + width ) - ( ground->x + ground->width*GROUND_NUM );
			x -= back_distance;
			pCamera->x -= back_distance;
		}
		else
		{
			for( int i = 0; i < obCnt; ++i )
			{
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || ( obstacles[i]->type == PUMKIN && is_under == false ) )
				{
					if( AABB( obstacles[i] ) )
					{
						back_distance = x + width - obstacles[i]->x;
						x -= back_distance;
						pCamera->x -= back_distance;
						break;
					}
				}
				else if( obstacles[i]->type == HAY )
				{
					if( obstacles[i]->AABB_surface( this ) )
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
	if( state[LEFT_STATE] )
	{
		x -= ( speed + x_additional_speed )*frameTime;
		pCamera->x -= ( speed + x_additional_speed )*frameTime;
		if( x < 0 )
		{
			x = 0;
			pCamera->x = 0;
		}
		else
		{
			for( int i = 0; i < obCnt; ++i )
			{
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || ( obstacles[i]->type == PUMKIN && is_under == false ) )
				{
					if( AABB( obstacles[i] ) )
					{
						back_distance = obstacles[i]->x + obstacles[i]->width - x;
						x += back_distance;
						pCamera->x += back_distance;
						break;
					}
				}
				else if( obstacles[i]->type == HAY )
				{
					if( obstacles[i]->AABB_surface( this ) )
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
	if( state[UP_STATE] && pCamera->view_point == DOWN_VIEW )
	{
		z += ( speed + z_additional_speed )*frameTime;
		if( z + depth > ground->z + ground->depth - 5 )
		{
			back_distance = ( z + depth ) - ( ground->z + ground->depth - 5 );
			z -= back_distance;
		}
		else
		{
			for( int i = 0; i < obCnt; ++i )
			{
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || ( obstacles[i]->type == PUMKIN && is_under == false ) )
				{
					if( AABB( obstacles[i] ) )
					{
						back_distance = z + depth - obstacles[i]->z;
						z -= back_distance;
						break;
					}
				}
				else if( obstacles[i]->type == HAY )
				{
					if( obstacles[i]->AABB_surface( this ) )
					{
						int last_z = z - ( speed + z_additional_speed )*frameTime;
						if( last_z >= obstacles[i]->z && last_z + depth <= obstacles[i]->z + obstacles[i]->depth )
						{
							back_distance = z + depth - ( obstacles[i]->z + obstacles[i]->depth );
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
	if( state[DOWN_STATE] && pCamera->view_point == DOWN_VIEW )
	{
		z -= ( speed + z_additional_speed )*frameTime;
		if( z < ground->z + 10 )
		{
			back_distance = ground->z - z + 10;
			z += back_distance;
		}
		else
		{
			for( int i = 0; i < obCnt; ++i )
			{
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || ( obstacles[i]->type == PUMKIN && is_under == false ) )
				{
					if( AABB( obstacles[i] ) )
					{
						back_distance = obstacles[i]->z + obstacles[i]->depth - z;
						z += back_distance;
						break;
					}
				}
				else if( obstacles[i]->type == HAY )
				{
					if( obstacles[i]->AABB_surface( this ) )
					{
						int last_z = z + ( speed + z_additional_speed )*frameTime;
						if( last_z >= obstacles[i]->z && last_z + depth <= obstacles[i]->z + obstacles[i]->depth )
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
	if( state[JUMP_UP_STATE] )
	{
		y += ( speed + y_additional_speed )*frameTime;
		if( y > org_y + jump_height )
		{
			state[JUMP_UP_STATE] = false;
			state[JUMP_DOWN_STATE] = true;
		}
		else
		{
			for( int i = 0; i < obCnt; ++i )
			{
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BLACK_SHEEP || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN )
				{
					if( AABB( obstacles[i] ) )
					{
						back_distance = y + height - obstacles[i]->y;
						y -= back_distance;
						state[JUMP_UP_STATE] = false;
						state[JUMP_DOWN_STATE] = true;
						break;
					}
				}
				else if( obstacles[i]->type == HAY )
				{
					if( obstacles[i]->AABB_surface( this ) )
					{
						int last_y = y - ( speed + y_additional_speed )*frameTime;
						if( last_y >= obstacles[i]->y && last_y + height <= obstacles[i]->y + obstacles[i]->height )
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
	else if( state[JUMP_DOWN_STATE] )
	{
		//추가속도 및 점프감소력 초기화
		y_additional_speed = 0;
		y -= speed*1.2*frameTime;
		if( y < 0 )
		{
			y = 0;
			state[JUMP_DOWN_STATE] = false;
		}
		else
		{
			for( int i = 0; i < obCnt; ++i )
			{
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN )
				{
					if( AABB( obstacles[i] ) )
					{
						back_distance = obstacles[i]->y + obstacles[i]->height - y;
						y += back_distance;
						state[JUMP_DOWN_STATE] = false;
						break;
					}
				}
				else if( obstacles[i]->type == HAY )
				{
					if( obstacles[i]->AABB_surface( this ) )
					{
						int last_y = y + speed*1.2*frameTime;
						if( last_y >= obstacles[i]->y && last_y + height <= obstacles[i]->y + obstacles[i]->height )
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
				else if( obstacles[i]->type == BLACK_SHEEP && AABB( obstacles[i] ) )
				{
					if( isCloseFromSelectedSheep() ) {
						FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[KILL_E], 0, &pSound->Channel[KILL_E] );
					}
					obstacles[i]->killed = true;
					state[JUMP_DOWN_STATE] = false;
					state[JUMP_UP_STATE] = true;
					org_y = y;
				}
			}
		}
	}

}
void Sheep::special_key( int key )
{
	if( key == GLUT_KEY_RIGHT ) {
		state[RIGHT_STATE] = true;
	}
	else if( key == GLUT_KEY_LEFT ) {
		state[LEFT_STATE] = true;
	}
	else if( key == GLUT_KEY_UP ) {
		if( pCamera->view_point == DOWN_VIEW )
		{
			state[UP_STATE] = true;
		}
		else if( pCamera->view_point == FRONT_VIEW && state[JUMP_DOWN_STATE] == false && state[JUMP_UP_STATE] == false && state[GRAVITY] == false )
		{
			state[JUMP_UP_STATE] = true;
			org_y = y;
			if( isCloseFromSelectedSheep() ) {
				FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[JUMP_E], 0, &pSound->Channel[JUMP_E] );
			}
		}
	}
	else if( key == GLUT_KEY_DOWN ) {
		if( pCamera->view_point == DOWN_VIEW )
		{
			state[DOWN_STATE] = true;
		}
	}
}
void Sheep::special_key_up( int key )
{
	if( key == GLUT_KEY_RIGHT ) {
		state[RIGHT_STATE] = false;
	}
	else if( key == GLUT_KEY_LEFT ) {
		state[LEFT_STATE] = false;
	}
	else if( key == GLUT_KEY_UP ) {
		state[UP_STATE] = false;
	}
	else if( key == GLUT_KEY_DOWN ) {
		state[DOWN_STATE] = false;
	}
}
void Sheep::setSound( SoundPackage *sound )
{
	pSound = sound;
	pCamera->pSound = sound;
}
bool Sheep::isCloseFromSelectedSheep()
{
	if( this == pSelectedSheep ) return true;
	if( abs( x - pSelectedSheep->x ) < 250.0f ) return true;
	return false;
}