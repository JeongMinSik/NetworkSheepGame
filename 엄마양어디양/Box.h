#pragma once
#include "Objects.h"

struct Box : public Object {
	float org_x, org_y, org_z;
	Box( int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z );
	~Box();
	virtual void draw() override final;
	virtual bool is_standing( const Object* other );
	virtual void update1( Sheep** sheep, float frameTime );
};