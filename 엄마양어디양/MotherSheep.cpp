#include "stdafx.h"
#include "MotherSheep.h"

void MotherSheep::draw()
{
	glPushMatrix();
	glTranslated( x, y, z );

	glRotated( 180, 0, 1, 0 );
	glScalef( 0.8, 0.8, 0.8 );


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

	// ¸öÅëÀÌ
	glPushMatrix();
	glColor3f( 1, 1, 1 );


	glColor3f( 1, 1, 1 );

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

	// ´«
	glPushMatrix();
	glColor3f( 0, 0, 0 );
	glTranslatef( 90, 0, 35 );
	glutSolidSphere( 7, 16, 16 );
	glTranslatef( 0, 0, -70 );
	glutSolidSphere( 7, 16, 16 );
	glPopMatrix();

	glPopMatrix();
}
