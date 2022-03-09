#pragma once

#include "glm/glm.hpp"
#include <vector>

#include "CameraManager.hpp"

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 o, glm::vec3 d)
        : origin(o), direction(d) {}
};

struct Shape
{
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;

    //int shapeType;
    //int operation;
    //float blendStrength;
    //int numChildren;

    Shape(glm::vec3 p, glm::vec3 s, glm::vec3 c)
        : position(p), size(s), color(c) {}

};

struct RayMarchingSettings
{
	const float maxDst = 5.0f;
	const float epsilon = 0.5f;

	bool positionLight = false;
	glm::vec3 Light = { 0.9, 0.9, 0.9 };

	const int numShapes = 1;
	std::vector<Shape> shapes;
	
	

};

class RayMarchingManager
{
public:
    RayMarchingManager(int width, int height);

    void update();

    glm::vec3 estimateNormal(glm::vec3 p);

    glm::vec4 getSceneInfo(glm::vec3 eye);

    Ray createCameraRay(glm::vec2 uv);


    const std::vector<unsigned char>& getBuffer() const {
        return _buffer;
    }

private:
    RayMarchingSettings _settings;

    Camera _camera;

    int _width;
    int _height;
    int _nbpixels;
    int _bufferSize;

    glm::vec3 _rayOrigin;

    std::vector<unsigned char> _buffer;

    int currentSample = 0;
};

