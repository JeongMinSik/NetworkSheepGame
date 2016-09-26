#pragma once
#include "stdafx.h"
#include "Objects.h"

struct SoundPackage {
	FMOD_SYSTEM *System;
	FMOD_SOUND **Sound;
	FMOD_CHANNEL **Channel;
};

struct Camera {
	float x, y, z;
	int canvas_size;
	float view_radius;
	bool view_point;
	bool is_changing;
	SoundPackage *pSound;
	Camera *pSelectedCamera;
	Camera();
	~Camera();
	void Camera::setting();
	void keyboard(unsigned char key);
	void update(float frameTime);
};

struct Ground {
	float x, y, z;
	float width, height, depth;
	int back_height;
	GLuint *pTextures;
	Ground(float x, float y, float z);
	~Ground();
	void draw();
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
	GLuint *pTextures;
	Camera *pCamera;

	Object(int type, float x, float y, float z, float w, float h, float d, float sp, float m_x, float m_y, float m_z);
	virtual ~Object() {  }
	virtual void draw() = 0;
	virtual void update1(Sheep**, float frameTime) {};
	virtual void update2(Sheep*, Object*[], float frameTime) { };
	virtual void update3(Sheep** sheeps, Object* obstacles[], float frameTime) {};
	virtual bool is_standing(const Object*) { return false; };
	virtual bool is_inside(Sheep*) { return false; };
	virtual bool AABB_surface(const Object*) { return false; };
	bool AABB(const Object* other);
	
	void test_draw();
};
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

	Sheep(int t, int x, int y, int z, float sp);
	~Sheep();
	void get_hurt();
	void dead_update(float frameTime);
	void ending_update(float frameTime);
	virtual void draw() override final;
	virtual void update2(const Ground* ground, Object* obstacles[], float frameTime);
	void special_key(int key, Object* obstacles[]);
	void special_key_up(int key);
	void setSound(SoundPackage* pSound);
	bool isCloseFromSelectedSheep();
	
};
struct Box : public Object {
	float org_x, org_y, org_z;
	Box(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Box();
	virtual void draw() override final;
	virtual bool is_standing(const Object* other);
	virtual void update1(Sheep** sheep, float frameTime);
};
struct Scissors : public Object {
	float org_x, org_y, org_z;
	int Rotate_y;
	float scissor_rot;
	Scissors(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Scissors();
	virtual void draw() override final;
	virtual void update1(Sheep** sheep, float frameTime);
};
struct Pumkin : public Object {
	float org_y;
	Pumkin(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Pumkin();
	virtual void draw() override final;
	virtual void update1(Sheep** sheep, float frameTime);
	virtual bool is_standing(const Object* other);
};
struct Hay : public Object
{
	Hay(int t, float x, float y, float z, float sp, float m_x, float m_y, float m_z);
	~Hay();
	virtual bool AABB_surface(const Object* other) override final;
	virtual bool is_inside(Sheep* sheep);
	virtual void draw() override final;
};
struct Black_Sheep : public Object {
	int view_rad; // ���� ����
	int tracing_distance; //�߰ݰŸ�
	float org_x, org_z; // �� ��ġ
	bool is_tracing; // �߰��غ���
	float ouch; // ���� �ð�
	int wait_time; // ����� �ϴ� �ð�
	float y_scale = 0; // ��ŭ ��ҽ�Ų��.
	Black_Sheep(int t, float x, float y, float z, float sp, int area_of_activity, int none1, int none2);
	~Black_Sheep();
	virtual void draw() override final;

	void trace_return(Sheep** sheeps, Object* obstacles[],float frameTime);
	virtual void update3(Sheep** sheeps, Object* obstacles[], float frameTime) ;
};
struct MotherSheep {
	int x = ENDING_X + 250, y = 50, z = 100;
	void draw();
};

struct Ui {
	int canvas_size;
	int selected_menu;
	float heart_size;
	float heart_dir;
	bool presskey;
	int key_delay[2];
	int help;
	int ending_screen = 0;
	int *pGameMode;
	SoundPackage *pSound;
	GLuint *pTextures;
	Ui(int size);
	~Ui();
	int keyboard(unsigned char key);
	void special_key(int key);
	void draw(Sheep* sheep);
	void update(float frameTime);
};