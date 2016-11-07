#include "stdafx.h"
#include "Hay.h"
#include "Sheep.h"

Hay::Hay( int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0 ) : Object( t, x, y, z, 100, 70, 100, sp, m_x, m_y, m_z ) {
}
Hay::~Hay() {  }
bool Hay::AABB_surface( const Object* other )
{
	if( other->x + other->width <= x ) return false;
	if( other->x >= x + width ) return false;
	if( other->y + other->height <= y ) return false;
	if( other->y >= y + height ) return false;
	if( other->z + other->depth <= z ) return false;
	if( other->z >= z + depth ) return false;
	if( other->z >= z && other->z + other->depth <= z + depth && other->y >= y && other->y + other->height <= y + height ) return false;
	return true;
}
bool Hay::is_inside( Sheep* sheep )
{
	if( sheep->z >= z && sheep->z + sheep->depth <= z + depth &&
		sheep->y >= y && sheep->y + sheep->height <= y + height &&
		sheep->x >= x && sheep->x + sheep->width <= x + width )
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
	glColor3f( 1, 1, 1 );
	//¾Õ¸é
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, pTextures[HAY_FRONT] );
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
	glBindTexture( GL_TEXTURE_2D, pTextures[HAY_TOP] );

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