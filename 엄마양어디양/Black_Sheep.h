#pragma once
#include "Objects.h"


struct Black_Sheep : public Object {
public:
	int view_rad; // 보는 방향
	int tracing_distance; //추격거리
	float org_x, org_z; // 원 위치
	bool is_tracing; // 추격준비모드
	float ouch; // 꿍한 시간
	int wait_time; // 어리둥절 하는 시간
	float y_scale = 0; // 만큼 축소시킨다.
public:
	Black_Sheep( int t, float x, float y, float z, float sp = 0, int area_of_activity = 0, int none1 = 0, int none2 = 0 );
	~Black_Sheep() = default;
public:
	virtual void draw() override final;
	void trace_return( Sheep** sheeps, Object* obstacles[], float frameTime );
	virtual void update3( Sheep** sheeps, Object* obstacles[], float frameTime );
};