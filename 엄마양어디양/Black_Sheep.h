#pragma once
#include "Objects.h"


struct Black_Sheep : public Object {
public:
	int view_rad; // ���� ����
	int tracing_distance; //�߰ݰŸ�
	float org_x, org_z; // �� ��ġ
	bool is_tracing; // �߰��غ���
	float ouch; // ���� �ð�
	int wait_time; // ����� �ϴ� �ð�
	float y_scale = 0; // ��ŭ ��ҽ�Ų��.
public:
	Black_Sheep( int t, float x, float y, float z, float sp = 0, int area_of_activity = 0, int none1 = 0, int none2 = 0 );
	~Black_Sheep() = default;
public:
	virtual void draw() override final;
	void trace_return( Sheep** sheeps, Object* obstacles[], float frameTime );
	virtual void update3( Sheep** sheeps, Object* obstacles[], float frameTime );
};