#include "stdafx.h"
#include "Ground.h"

Ground::Ground( float x, float y, float z ) :x( x ), y( y ), z( z ), width( 1000 ), height( 100 ), depth( 400 ) {
	back_height = 300;
}
Ground::~Ground() { }
void Ground::draw() {
	glColor3f( 1, 1, 1 );

	glEnable( GL_TEXTURE_2D );

	//µÞ¹è°æ
	glBindTexture( GL_TEXTURE_2D, pTextures[BACK_GROUND] );
	glBegin( GL_QUADS );
	glTexCoord2i( 0, 0 );
	glVertex3f( x, y + height, z + depth );
	glTexCoord2i( 1, 0 );
	glVertex3f( x + width, y + height, z + depth );
	glTexCoord2i( 1, 1 );
	glVertex3f( x + width, y + height + back_height, z + depth );
	glTexCoord2i( 0, 1 );
	glVertex3f( x, y + height + back_height, z + depth );
	glEnd();

	//¾Õ¸é
	glBindTexture( GL_TEXTURE_2D, pTextures[GROUND_FRONT] );
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
	glBindTexture( GL_TEXTURE_2D, pTextures[GROUND_TOP] );
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