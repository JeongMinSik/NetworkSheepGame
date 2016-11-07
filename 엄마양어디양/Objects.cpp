#include "Objects.h"
#include "Camera.h"
#include "SoundPackage.h"
#include "Ground.h"

bool Object::AABB( const Object* other )
{
	if( x + width <= other->x ) return false;
	if( x >= other->x + other->width ) return false;
	if( y + height <= other->y ) return false;
	if( y >= other->y + other->height ) return false;
	if( z + depth <= other->z ) return false;
	if( z >= other->z + other->depth ) return false;
	return true;
}
void Object::test_draw()
{
	glPushMatrix();
	glColor3f( 1, 0, 0 );

	int yy = y;
	for( int i = 0; i < 2; ++i )
	{
		if( i == 1 ) yy = y + height;
		glPushMatrix();
		glTranslated( x, yy, z );
		glutSolidSphere( 2, 10, 10 );
		glPopMatrix();
		glPushMatrix();
		glTranslated( x + width, yy, z );
		glutSolidSphere( 2, 10, 10 );
		glPopMatrix();
		glPushMatrix();
		glTranslated( x, yy, z + depth );
		glutSolidSphere( 2, 10, 10 );
		glPopMatrix();
		glPushMatrix();
		glTranslated( x + width, yy, z + depth );
		glutSolidSphere( 2, 10, 10 );
		glPopMatrix();
	}


	glPopMatrix();
}