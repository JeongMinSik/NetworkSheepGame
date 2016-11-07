#pragma once
#include "Objects.h"

struct SoundPackage;
struct Ground; 
struct Sheep : public Object {
	bool state[8]; // 이동상태
	float jump_height;		// 점프력
	float minus_height; //감소 점프력
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
	SoundPackage *pSound;
	int iGameMode;
	int obCnt; // 장애물 수
	Sheep* pSelectedSheep; // 사운드 기준 양

	Sheep( int t, int x, int y, int z, float sp );
	~Sheep();
	void get_hurt();
	void dead_update( float frameTime );
	void ending_update( float frameTime );
	virtual void draw() override final;
	virtual void update2( const Ground* ground, Object* obstacles[], float frameTime );
	void special_key( int key );
	void special_key_up( int key );
	void setSound( SoundPackage* pSound );
	bool isCloseFromSelectedSheep();

};