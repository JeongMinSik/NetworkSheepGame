#pragma once
#include "stdafx.h"

struct Camera {
	float x, y;
	int canvas_size;
	float view_radius;
	bool view_point;
	bool is_changing;
	Camera();
	~Camera();
	void keyboard(unsigned char key);
	void update(float frameTime);
};

struct Ground {
	float x, y, z;
	float width, height, depth;
	int back_height;
	Ground(float x, float y, float z);
	~Ground();
};

struct Sheep;

struct Object {
	float x, y, z;
	float width, height, depth;
	float speed;
	float max_x, max_y, max_z;
	int state_x, state_y, state_z;
	int type;
	bool killed = false;
	Camera *pCamera;

	Object(int type, float x, float y, float z, float w, float h, float d, float sp, float m_x, float m_y, float m_z);
	virtual ~Object() {  }
	virtual void update1(Sheep**, float frameTime) {};
	virtual void update2(Sheep*, Object*[], float frameTime) { };
	virtual void update3(Sheep** sheeps, Object* obstacles[], float frameTime) {};
	virtual bool is_standing(const Object*) { return false; };
	virtual bool is_inside(Sheep*) { return false; };
	virtual bool AABB_surface(const Object*) { return false; };
	bool AABB(const Object* other);
};

struct Sheep : public Object {
	BOOL state[8]; // 이동상태
	float jump_height;		// 점프력
	float x_additional_speed, y_additional_speed, z_additional_speed;	// 추가 이동속도
	float org_y; // 점프시 y 위치
	int last_view; //마지막 바라본 방향(각도)
	int life; //목숨
	bool is_invincible; // 무적상태
	bool is_under; // 눌린 상태
	bool is_in_hay; // 지푸라기 안
	int max_invicible_time;  // 최대 무적시간 (반드시 짝수로만 설정)
	int cur_invicible_time;  // 현재 무적시간
	int stading_index; // 바닥객체 인덱스
	bool ending_finished = false;
	int iGameMode;
	int obCnt; // 장애물 수
	bool isHurted; // 다친 상태를 클라에게 전송해야 한다.

	Sheep(int t, int x, int y, int z, float sp);
	~Sheep();
	void get_hurt();
	void dead_update(float frameTime);
	virtual void update2(const Ground* ground, Object* obstacles[], float frameTime);
	void special_key(int key, Object* obstacles[]);
	void special_key_up(int key);

};
struct Box : public Object {
	float org_x, org_y, org_z;
	Box(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Box();
	virtual bool is_standing(const Object* other);
	virtual void update1(Sheep** sheep, float frameTime);
};
struct Scissors : public Object {
	float org_x, org_y, org_z;
	int Rotate_y;
	float scissor_rot;
	Scissors(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Scissors();
	virtual void update1(Sheep** sheep, float frameTime);
};
struct Pumkin : public Object {
	float org_y;
	Pumkin(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Pumkin();
	virtual void update1(Sheep** sheep, float frameTime);
	virtual bool is_standing(const Object* other);
};
struct Hay : public Object
{
	Hay(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Hay();
	virtual bool AABB_surface(const Object* other) override final;
	virtual bool is_inside(Sheep* sheep);
};
struct Black_Sheep : public Object {
	int view_rad; // 보는 방향
	int tracing_distance; //추격거리
	float org_x, org_z; // 원 위치
	bool is_tracing; // 추격준비모드
	float ouch; // 꿍한 시간
	int wait_time; // 어리둥절 하는 시간
	float y_scale = 0; // 만큼 축소시킨다.
	Black_Sheep(int t, float x, float y, float z, float sp, int area_of_activity, int none1, int none2);
	~Black_Sheep();

	void trace_return(Sheep** sheeps, Object* obstacles[], float frameTime);
	virtual void update3(Sheep** sheeps, Object* obstacles[], float frameTime);
};
struct MotherSheep {
	int x = ENDING_X + 250, y = 50, z = 100;
	void draw();
};
