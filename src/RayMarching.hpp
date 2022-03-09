#pragma once

#include "glm/glm.hpp"
#include <vector>

#include "CameraManager.hpp"
#include "Framebuffer.hpp"

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 o = glm::vec3(0), glm::vec3 d = glm::vec3(0))
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
	const float maxDst = 6.0f;
	const float epsilon = 0.01f;

	bool positionLight = false;
	glm::vec3 Light = { 0.9, 0.9, 0.9 };

	const int numShapes = 2;
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

    void free();

    Framebuffer& getFbo() { return _fbo; }

    const std::vector<unsigned char>& getBuffer() const {
        return _buffer;
    }

private:

    void updateRays();

private:
    RayMarchingSettings _settings;

    Camera _camera;
    
    int _width;
    int _height;
    int _nbpixels;
    int _bufferSize;
    std::vector<unsigned char> _buffer;

    Framebuffer _fbo;

    glm::vec3 _rayOrigin;

    std::vector<Ray> _rays;

    int currentSample = 0;
    const int maxSamples = 10;

    bool _needToUpdateRays = true; // If camera move, we must compute new rays
};

