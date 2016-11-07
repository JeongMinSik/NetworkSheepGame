#include "stdafx.h"
#include "Camera.h"
#include "SoundPackage.h"

Camera::Camera()
{
	x = 0; y = 100;
	canvas_size = 200;
	view_radius = 0;
	view_point = FRONT_VIEW;
	is_changing = false;
	pSelectedCamera = nullptr;
}
Camera::~Camera() {  }
void Camera::setting()
{
	glOrtho( -canvas_size, canvas_size, -canvas_size, canvas_size, 1000, -1000 );

	gluLookAt( x, y, 0, x, y, -1, 0.0, 1.0, 0.0 );
	glRotated( view_radius, 1, 0, 0 );
}
void Camera::keyboard( unsigned char key )
{
	if( key == ' ' && is_changing == false ) {
		if( this == pSelectedCamera ) {
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CAMERA_E], 0, &pSound->Channel[CAMERA_E] );
		}
		is_changing = true;
	}

}
void Camera::update( float frameTime )
{
	// 카메라 회전
	if( is_changing )
	{
		if( view_point == FRONT_VIEW )
		{
			view_radius -= 0.2f * frameTime;
			if( view_radius <= -90 )
			{
				is_changing = false;
				view_radius = -90;
				view_point = DOWN_VIEW;
			}
		}
		else if( view_point == DOWN_VIEW )
		{
			view_radius += 0.2f * frameTime;
			if( view_radius >= 0 )
			{
				is_changing = false;
				view_radius = 0;
				view_point = FRONT_VIEW;
			}
		}
	}
}