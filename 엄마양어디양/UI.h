#pragma once

#include "stdafx.h"
struct SoundPackage;
struct Sheep;
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
	Ui( int size );
	~Ui();
	int keyboard( unsigned char key );
	void special_key( int key );
	void draw( Sheep* sheep );
	void update( float frameTime );
};