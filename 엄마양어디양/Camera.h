#pragma once

struct SoundPackage;
struct Camera {
public:
	float x, y;
	int canvas_size;
	float view_radius;
	bool view_point;
	bool is_changing;
	SoundPackage *pSound;
	Camera *pSelectedCamera;
	Camera();
	~Camera();
	void Camera::setting();
	void keyboard( unsigned char key );
	void update( float frameTime );
};