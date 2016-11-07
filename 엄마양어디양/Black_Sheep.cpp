#include "stdafx.h"
#include "Black_Sheep.h"
#include "Sheep.h"

Black_Sheep::Black_Sheep( int t, float x, float y, float z, float sp/* = 0*/, int area_of_activity/* = 0*/, int none1/* = 0*/, int none2/* = 0*/ )
	: Object( t, x, y, z, 55, 35, 55, sp, area_of_activity, none1, none2 )
{
	org_x = x;
	org_z = z;
	tracing_distance = area_of_activity;
	view_rad = 0;
	is_tracing = false;
	ouch = 0;
	wait_time = 11;
}

void Black_Sheep::draw()
{
	glPushMatrix();
	glTranslated( ( x + width / 2 ), ( y + height / 2 ) + 5, z + depth / 2 );

	glRotated( -view_rad, 0, 1, 0 );
	glScalef( 0.30, 0.35 - y_scale, 0.35 );
	glPushMatrix();
	glColor3f( 1, 0.9, 0.9 );
	glTranslatef( 70, -20, 0 );
	glutSolidSphere( 50, 16, 16 );
	glPopMatrix();

	glPushMatrix();
	glColor3f( 0, 0, 0 );
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
	glColor3f( 1, 0, 0 );
	glTranslatef( 95, -10, 25 );
	glRotated( 70, 1, 0, 0 );
	glutSolidTorus( 9, 10, 5, 12 );
	glPopMatrix();

	glPushMatrix();
	glTranslatef( 95, -10, -25 );
	glRotated( -70, 1, 0, 0 );
	glutSolidTorus( 9, 10, 5, 12 );
	glPopMatrix();


	//  입
	glPushMatrix();
	glColor3f( 1, 0, 0 );
	glTranslated( 90, -30, 0 );
	glRotated( 95, 1, 0, 0 );
	glutSolidTorus( 9, 25, 5, 12 );
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
	glTranslated( 70, 5, 30 );
	glRotated( -45, 1, 0, 0 );
	glColor3f( 0.7, 0.7, 0.7 );
	glutSolidCone( 20, 50, 10, 10 );
	glPopMatrix();

	glPushMatrix();
	glTranslated( 70, 5, -30 );
	glRotated( -135, 1, 0, 0 );
	glColor3f( 0.7, 0.7, 0.7 );
	glutSolidCone( 20, 50, 10, 10 );
	glPopMatrix();
	glPopMatrix();
}

void Black_Sheep::trace_return( Sheep** sheeps, Object* obstacles[], float frameTime )
{
	// 가장 거리가 가까운 양 계산
	int sx, sz;
	float d;
	int minIndex = 0;
	float min = tracing_distance + 1;
	for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
		sx = sheeps[i]->x; sz = sheeps[i]->z;
		d = sqrt( float( sx - x )*float( sx - x ) + float( sz - z )*float( sz - z ) );
		if( d < min && !sheeps[minIndex]->is_in_hay ) {
			min = d;
			minIndex = i;
		}
	}
	sx = sheeps[minIndex]->x; sz = sheeps[minIndex]->z;
	d = min;

	// 원위치와의 계산
	float org_d = sqrt( float( org_x - x )*float( org_x - x ) + float( org_z - z )*float( org_z - z ) );
	int vx, vz; // 이동량

	if( is_tracing )
	{
		// 추격 종료
		if( org_d > tracing_distance || d > tracing_distance || ouch > 1000 ) {
			is_tracing = false;
		}
		else {
			// 바라보는 각도 계산
			view_rad = atan2( sz - z, sx - x ) * 180 / 3.1415926535;

			// 이동
			if( d > speed*frameTime )
			{
				vx = ( sx - x ) / d*speed*frameTime;
				vz = ( sz - z ) / d*speed*frameTime;
			}
			else
			{
				vx = 0;
				vz = 0;
			}
			x += vx;
			z += vz;



			// 타 장애물들과의 충돌체크
			for( int i = 0; i < OB_CNT; ++i )
			{
				if( obstacles[i] == nullptr ) break;
				if( obstacles[i]->type == BOX || obstacles[i]->type == BOXWALL || obstacles[i]->type == BRICK || obstacles[i]->type == PUMKIN || obstacles[i]->type == HAY )
				{
					if( AABB( obstacles[i] ) )
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
		if( wait_time > 1000 ) {
			ouch = 0;
			// 바라보는 각도 계산
			view_rad = atan2( org_z - z, org_x - x ) * 180 / 3.1415926535;

			if( org_d > speed*frameTime ) {
				vx = ( org_x - x ) / org_d*speed*frameTime;
				vz = ( org_z - z ) / org_d*speed*frameTime;
			}
			else {
				vx = 0;
				vz = 0;
			}
			x += vx;
			z += vz;
		}
		//추격 재개
		else if( org_d <= tracing_distance && d <= tracing_distance && ouch <= 1000 )
		{
			is_tracing = true;
			wait_time = 0;
		}

		// 원점으로 돌아옴
		if( speed*frameTime >= abs( org_x - x ) && speed*frameTime >= abs( org_z - z ) && d <= tracing_distance && sheeps[minIndex]->y == y )
		{
			is_tracing = true;
			wait_time = 0;
		}
	}

}

void Black_Sheep::update3( Sheep** sheeps, Object* obstacles[], float frameTime )
{
	if( killed )
	{
		if( height > 0 ) {
			y--;
			height -= sheeps[0]->speed*frameTime * 2;
			//y_scale += 0.01*frameTime;
		}
	}
	else { trace_return( sheeps, obstacles, frameTime ); }
}