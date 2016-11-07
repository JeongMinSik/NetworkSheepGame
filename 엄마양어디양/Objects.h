#pragma once
#include "stdafx.h"

struct Sheep;
struct Camera;

struct Object {
	float x, y, z;
	float width, height, depth;
	float speed;
	float max_x, max_y, max_z;
	int state_x, state_y, state_z;
	int type;
	bool killed = false;
	GLuint *pTextures;
	Camera *pCamera;

	Object( int type, float x, float y, float z, float w, float h, float d, float sp = 0, float m_x = 0, float m_y = 0, float m_z = 0 ) : x( x ), y( y ), z( z ), width( w ), height( h ), depth( d ), type( type ), speed( sp*0.02 ), max_x( m_x ), max_y( m_y ), max_z( m_z ) {}

	virtual ~Object() {  }
	virtual void draw() = 0;
	virtual void update1( Sheep**, float frameTime ) {};
	virtual void update2( Sheep*, Object*[], float frameTime ) { };
	virtual void update3( Sheep** sheeps, Object* obstacles[], float frameTime ) {};
	virtual bool is_standing( const Object* ) { return false; };
	virtual bool is_inside( Sheep* ) { return false; };
	virtual bool AABB_surface( const Object* ) { return false; };
	bool AABB( const Object* other );

	void test_draw();
};


