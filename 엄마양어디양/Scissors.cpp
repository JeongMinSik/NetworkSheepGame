#include "stdafx.h"
#include "Scissors.h"

Scissors::Scissors( int t, float x, float y, float z, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0 ) : Object( t, x, y, z, 40, 8, 35, sp, m_x, m_y, m_z ) {
	max_x = m_x;	max_y = m_y;	max_z = m_z;
	org_x = x;		org_y = y;		org_z = z;
	Rotate_y = 0;
	scissor_rot = 0.1;
	if( max_x > 0 ) state_x = RIGHT_STATE;
	else if( max_x < 0 ) state_x = LEFT_STATE;
	else state_x = STOP_STATE;
	if( max_z > 0 ) state_z = UP_STATE;
	else if( max_z < 0 ) state_z = DOWN_STATE;
	else state_z = STOP_STATE;
}
Scissors::~Scissors() {  }
void Scissors::draw()
{
	glPushMatrix();
	glTranslated( ( x + width / 2 ) + 28, ( y + height / 2 ) + 12, z + depth / 2 );
	if( state_x == RIGHT_STATE )
	{
		glRotated( 180, 0, 1, 0 );
		glTranslated( 57, 0, 0 );
	}
	else if( state_z == UP_STATE )
	{
		glRotated( 90, 0, 1, 0 );
		glTranslated( 30, 0, -30 );
	}
	else if( state_z == DOWN_STATE )
	{
		glRotated( 270, 0, 1, 0 );
		glTranslated( 30, 0, 30 );
	}
	glScaled( 0.4, 0.45, 0.35 );

	glPushMatrix();
	glRotatef( Rotate_y + 15, 0, 1, 0 );

	glColor3f( 0, 0, 0 );         // 손잡이
	glBegin( GL_POLYGON );
	glVertex3f( 0, -20, 5 );
	glVertex3f( 100, -20, 20 );
	glVertex3f( 80, -20, -30 );
	glVertex3f( 0, -20, -5 );
	glEnd();

	glBegin( GL_POLYGON );      // FRONT
	glVertex3f( 0, -20, -5 );
	glVertex3f( 0, -50, 0 );
	glVertex3f( 80, -50, 0 );
	glVertex3f( 80, -20, 0 );
	glEnd();

	glColor3f( 0.8, 0.8, 0.8 );   // 날
	glBegin( GL_POLYGON );
	glVertex3f( 0, -20, 0 );
	glVertex3f( -160, -20, 0 );
	glVertex3f( -80, -20, 40 );
	glEnd();

	glBegin( GL_POLYGON );    //FRONT
	glVertex3f( -160, -20, 0 );
	glVertex3f( -160, -50, 0 );
	glVertex3f( 0, -50, 0 );
	glVertex3f( 0, -20, 0 );
	glEnd();
	glPopMatrix();


	// 오른쪽 날
	glPushMatrix();
	glTranslatef( 0, -40, 0 );
	glRotatef( 180, 1, 0, 0 );
	glRotatef( Rotate_y + 15, 0, 1, 0 );

	glColor3f( 0, 0, 0 );         // 손잡이
	glBegin( GL_POLYGON );
	glVertex3f( 0, -20, 5 );
	glVertex3f( 100, -20, 20 );
	glVertex3f( 80, -20, -30 );
	glVertex3f( 0, -20, -5 );
	glEnd();


	glBegin( GL_POLYGON );      // FRONT
	glVertex3f( 0, -20, -5 );
	glVertex3f( 0, 10, 0 );
	glVertex3f( 80, 10, 0 );
	glVertex3f( 80, -20, 0 );
	glEnd();

	glColor3f( 0.8, 0.8, 0.8 );   // 날
	glBegin( GL_POLYGON );
	glVertex3f( 0, -20, 0 );
	glVertex3f( -160, -20, 0 );
	glVertex3f( -80, -20, 40 );
	glEnd();

	glBegin( GL_POLYGON );
	glVertex3f( -160, -20, 0 );
	glVertex3f( -160, 10, 0 );
	glVertex3f( 0, 10, 0 );
	glVertex3f( 0, -20, 0 );
	glEnd();
	glPopMatrix();

	glPopMatrix();
}
void Scissors::update1( Sheep** sheeps, float frameTime )
{
	Rotate_y += scissor_rot*frameTime;
	if( Rotate_y >= 15 )
		scissor_rot *= -1;
	if( Rotate_y <= -5 )
		scissor_rot *= -1;

	if( state_x == RIGHT_STATE )
	{
		x += speed*frameTime;
		if( abs( x - org_x ) >= abs( max_x ) ) state_x = LEFT_STATE;
	}
	if( state_x == LEFT_STATE )
	{
		x -= speed*frameTime;
		if( abs( x - org_x ) >= abs( max_x ) ) state_x = RIGHT_STATE;
	}
	if( state_z == UP_STATE )
	{
		z += speed*frameTime;
		if( abs( z - org_z ) >= abs( max_z ) )
		{
			state_z = DOWN_STATE;
		}
	}
	else if( state_z == DOWN_STATE )
	{
		z -= speed*frameTime;
		if( abs( z - org_z ) >= abs( max_z ) )
		{
			state_z = UP_STATE;
		}
	}

}