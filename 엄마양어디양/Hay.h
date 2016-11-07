#pragma once
#include "Objects.h"

struct Hay : public Object
{
	Hay( int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z );
	~Hay();
	virtual bool AABB_surface( const Object* other ) override final;
	virtual bool is_inside( Sheep* sheep );
	virtual void draw() override final;
};