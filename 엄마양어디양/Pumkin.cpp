#include "stdafx.h"
#include "Pumkin.h"
#include "Sheep.h"

Pumkin::Pumkin( int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0 ) : Object( t, x, y, z, 50, 50, 50, sp, m_x, m_y, m_z ) {
	max_y = m_y;
	org_y = y;
	if( max_y > 0 ) state_y = JUMP_UP_STATE;
	else if( max_y < 0 ) state_y = JUMP_DOWN_STATE;
	else state_y = STOP_STATE;
}
Pumkin::~Pumkin() {  }
void Pumkin::draw()
{
	glPushMatrix();
	glTranslated( ( x + width / 2 ), ( y + height / 2 ), z + depth / 2 );

	glScaled( 0.45, 0.45, 0.45 );
	// ²ÀÁö
	glPushMatrix();
	glColor3f( 0.309804, 0.309804, 0.184314 );
	glTranslated( 0, 55, 0 );
	glRotated( -90, 1, 0, 0 );
	glutSolidCone( 20, 50, 12, 1 );

	glTranslated( 0, 0, 45 );
	glRotated( -180, 1, 0, 0 );
	glutSolidCone( 20, 20, 12, 1 );
	glPopMatrix();

	// ¹Ú
	glPushMatrix();
	glScaled( 1, 0.8, 1 );

	glColor3f( 1, 0.5, 0 );
	glutSolidTorus( 50, 40, 16, 12 );

	glRotated( 45, 0, 1, 0 );
	glColor3f( 1, 0.25, 0 );
	glutSolidTorus( 50, 40, 16, 12 );

	glRotated( 45, 0, 1, 0 );
	glColor3f( 1, 0.5, 0 );
	glutSolidTorus( 50, 40, 16, 12 );

	glRotated( 45, 0, 1, 0 );
	glColor3f( 1, 0.25, 0 );
	glutSolidTorus( 50, 40, 16, 12 );

	glPopMatrix();

	glPopMatrix();
}
void Pumkin::update1( Sheep** sheeps, float frameTime )
{
	if( state_y == JUMP_UP_STATE )
	{
		y += speed*frameTime;
		for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
			if( AABB( sheeps[i] ) ) {
				sheeps[i]->y += speed*frameTime;
			}
		}
		if( abs( y - org_y ) >= abs( max_y ) )
		{
			state_y = JUMP_DOWN_STATE;
			y -= ( 2 * speed*frameTime );
		}
	}
	else if( state_y == JUMP_DOWN_STATE )
	{
		y -= speed*frameTime;
		if( abs( y - org_y ) >= abs( max_y ) )
		{
			state_y = JUMP_UP_STATE;
			y += ( 2 * speed*frameTime );
		}
	}
}
bool Pumkin::is_standing( const Object* other )
{
	if( y + height != other->y ) return false;
	if( x + width <= other->x ) return false;
	if( x >= other->x + other->width ) return false;
	if( z + depth <= other->z ) return false;
	if( z >= other->z + other->depth ) return false;
	return true;
}