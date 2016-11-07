#pragma once
#include "Objects.h"


struct Pumkin : public Object {
	float org_y;
	Pumkin( int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z );
	~Pumkin();
	virtual void draw() override final;
	virtual void update1( Sheep** sheep, float frameTime );
	virtual bool is_standing( const Object* other );
};