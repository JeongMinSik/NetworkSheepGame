#pragma once

struct Ground {
	float x, y, z;
	float width, height, depth;
	int back_height;
	GLuint *pTextures;
	Ground( float x, float y, float z );
	~Ground();
	void draw();
};