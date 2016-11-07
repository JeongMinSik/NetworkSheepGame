#include "stdafx.h"
#include "Box.h"
#include "Sheep.h"
#include "Camera.h"

Box::Box( int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0 ) : Object( t, x, y, z, 0, 0, 0, sp, m_x, m_y, m_z ) {
	if( type == BOX )
	{
		width = 70; height = 70; depth = 80;
	}
	else if( type == BRICK )
	{
		width = 70; height = 35; depth = 80;
	}
	else if( type == BOXWALL )
	{
		width = 70; height = 210; depth = 400;
	}
	max_x = m_x;	max_y = m_y;	max_z = m_z;
	org_x = x;		org_y = y;		org_z = z;
	if( max_x > 0 ) state_x = RIGHT_STATE;
	else if( max_x < 0 ) state_x = LEFT_STATE;
	else state_x = STOP_STATE;
	if( max_y > 0 ) state_y = JUMP_UP_STATE;
	else if( max_y < 0 ) state_y = JUMP_DOWN_STATE;
	else state_y = STOP_STATE;
	if( max_z > 0 ) state_z = UP_STATE;
	else if( max_z < 0 ) state_z = DOWN_STATE;
	else state_z = STOP_STATE;
}
Box::~Box() {  }
void Box::draw()
{
	glColor3f( 1, 1, 1 );

	//¾Õ¸é
	glEnable( GL_TEXTURE_2D );
	if( type == BOX ) { glBindTexture( GL_TEXTURE_2D, pTextures[BOX_FRONT] ); }
	else if( type == BRICK ) { glBindTexture( GL_TEXTURE_2D, pTextures[BRICK_FRONT] ); }
	else if( type == BOXWALL ) { glBindTexture( GL_TEXTURE_2D, pTextures[BOXWALL_FRONT] ); }
	glBegin( GL_QUADS );
	glTexCoord2i( 0, 0 );
	glVertex3f( x, y, z );
	glTexCoord2i( 1, 0 );
	glVertex3f( x + width, y, z );
	glTexCoord2i( 1, 1 );
	glVertex3f( x + width, y + height, z );
	glTexCoord2i( 0, 1 );
	glVertex3f( x, y + height, z );
	glEnd();

	//À­¸é
	if( type == BOX ) { glBindTexture( GL_TEXTURE_2D, pTextures[BOX_TOP] ); }
	else if( type == BRICK ) { glBindTexture( GL_TEXTURE_2D, pTextures[BRICK_TOP] ); }
	else if( type == BOXWALL ) { glBindTexture( GL_TEXTURE_2D, pTextures[BOXWALL_TOP] ); }
	glBegin( GL_QUADS );
	glTexCoord2i( 0, 0 );
	glVertex3f( x, y + height, z );
	glTexCoord2i( 1, 0 );
	glVertex3f( x + width, y + height, z );
	glTexCoord2i( 1, 1 );
	glVertex3f( x + width, y + height, z + depth );
	glTexCoord2i( 0, 1 );
	glVertex3f( x, y + height, z + depth );
	glEnd();
	glDisable( GL_TEXTURE_2D );

	//¹Ù´Ú¸é
	glBegin( GL_QUADS );
	glVertex3f( x, y, z );
	glVertex3f( x, y, z + depth );
	glVertex3f( x + width, y, z + depth );
	glVertex3f( x + width, y, z );
	glEnd();

	//¿ÞÂÊ¸é
	glBegin( GL_QUADS );
	glVertex3f( x, y, z );
	glVertex3f( x, y + height, z );
	glVertex3f( x, y + height, z + depth );
	glVertex3f( x, y, z + depth );
	glEnd();
	//¿À¸¥ÂÊ¸é
	glBegin( GL_QUADS );
	glVertex3f( x + width, y, z );
	glVertex3f( x + width, y, z + depth );
	glVertex3f( x + width, y + height, z + depth );
	glVertex3f( x + width, y + height, z );
	glEnd();
	//µÞ¸é
	glBegin( GL_QUADS );
	glVertex3f( x, y, z + depth );
	glVertex3f( x, y + height, z + depth );
	glVertex3f( x + width, y + height, z + depth );
	glVertex3f( x + width, y, z + depth );
	glEnd();
}
bool Box::is_standing( const Object* other )
{
	if( y + height != other->y ) return false;
	if( x + width <= other->x ) return false;
	if( x >= other->x + other->width ) return false;
	if( z + depth <= other->z ) return false;
	if( z >= other->z + other->depth ) return false;
	return true;
}
void Box::update1( Sheep** sheeps, float frameTime )
{
	if( state_x == RIGHT_STATE )
	{
		x += speed*frameTime;
		for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
			if( AABB( sheeps[i] ) ) {
				sheeps[i]->x += speed*frameTime;
				sheeps[i]->pCamera->x += speed*frameTime;
			}
		}

		if( abs( x - org_x ) >= abs( max_x ) ) {
			state_x = LEFT_STATE;
			x = org_x + max_x - speed*frameTime;
		}
	}
	if( state_x == LEFT_STATE )
	{
		x -= speed*frameTime;
		for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
			if( AABB( sheeps[i] ) ) {
				sheeps[i]->x -= speed*frameTime;
				sheeps[i]->pCamera->x -= speed*frameTime;
			}
		}
		if( abs( x - org_x ) >= abs( max_x ) ) {
			state_x = RIGHT_STATE;
			x = org_x - max_x + speed*frameTime;
		}
	}
	if( state_z == UP_STATE )
	{
		z += speed*frameTime;
		for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
			if( AABB( sheeps[i] ) ) {
				sheeps[i]->z += speed*frameTime;
			}
		}
		if( abs( z - org_z ) >= abs( max_z ) )
		{
			state_z = DOWN_STATE;
			z -= 2 * speed*frameTime;
		}
	}
	else if( state_z == DOWN_STATE )
	{
		z -= speed*frameTime;
		for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
			if( AABB( sheeps[i] ) ) {
				sheeps[i]->z -= speed*frameTime;
			}
		}
		if( abs( z - org_z ) >= abs( max_z ) )
		{
			state_z = UP_STATE;
			z += 2 * speed*frameTime;
		}
	}
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
			y -= 2 * speed*frameTime;
		}
	}
	else if( state_y == JUMP_DOWN_STATE )
	{
		y -= speed*frameTime;
		for( int i = 0; i < MAX_PLAYER_CNT; ++i ) {
			if( AABB( sheeps[i] ) ) {
				sheeps[i]->y -= speed*frameTime;
			}
		}
		if( abs( y - org_y ) >= abs( max_y ) )
		{
			state_y = JUMP_UP_STATE;
			y += 2 * speed*frameTime;
		}
	}
}