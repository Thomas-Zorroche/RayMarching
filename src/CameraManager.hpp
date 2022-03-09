#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"


class Camera
{
public:
	Camera(int width, int height)
	{
		_width = width;
		_height = height;

		_eye = { 0, 0, -4 };
		_center = { 0, 0, 0 };
		_up = { 0, 1, 0 };

		const auto projMatrix = glm::perspective(70.f, float(_width) / _height, 0.001f, 1000.0f);
		_cameraInverseProjection = glm::inverse(projMatrix);
		updateCamera();
	}

	void updateCamera()
	{
		_cameraToWorld = glm::lookAt(_eye, _center, _up);
	}

	const glm::mat4& getCameraToWorld() const { return _cameraToWorld; }
	const glm::mat4& getCameraInverseProjection() const { return _cameraInverseProjection; }

public:
	glm::vec3 _eye;
	glm::vec3 _center;
	glm::vec3 _up;

private:	  
	glm::mat4 _cameraToWorld;
	glm::mat4 _cameraInverseProjection;

	int _width;
	int _height;
};