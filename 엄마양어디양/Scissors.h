#pragma once
#include "Objects.h"

struct Scissors : public Object {
	float org_x, org_y, org_z;
	int Rotate_y;
	float scissor_rot;
	Scissors( int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z );
	~Scissors();
	virtual void draw() override final;
	virtual void update1( Sheep** sheep, float frameTime );
};