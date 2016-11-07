#pragma once
#include "Objects.h"

struct SoundPackage;
struct Ground; 
struct Sheep : public Object {
	bool state[8]; // �̵�����
	float jump_height;		// ������
	float minus_height; //���� ������
	float x_additional_speed, y_additional_speed, z_additional_speed;	// �߰� �̵��ӵ�
	float org_y; // ������ y ��ġ
	int last_view; //������ �ٶ� ����(����)
	int life; //���
	bool is_invincible; // ��������
	bool is_under; // ���� ����
	bool is_in_hay; // ��Ǫ��� ��
	int max_invicible_time;  // �ִ� �����ð� (�ݵ�� ¦���θ� ����)
	int cur_invicible_time;  // ���� �����ð�
	int stading_index; // �ٴڰ�ü �ε���
	bool ending_finished = false;
	SoundPackage *pSound;
	int iGameMode;
	int obCnt; // ��ֹ� ��
	Sheep* pSelectedSheep; // ���� ���� ��

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