#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <string>

#include "CameraManager.hpp"
#include "Framebuffer.hpp"

#include <klein/klein.hpp>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
    kln::point org;

    Ray(const glm::vec3& o = glm::vec3(0), const glm::vec3& d = glm::vec3(0))
        : origin(o), direction(d), org(origin.x, origin.y, origin.z)
    {
       
    }
};

enum class EOperation
{
    DEFAULT = 0,
    BLEND = 1,
};

struct Shape
{
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;

    kln::point center;

    std::string name = "shape";

    //int shapeType;
    EOperation operation = EOperation::DEFAULT;
    float blendStrength = 0.1f;

    Shape(const glm::vec3& p, const glm::vec3& s, const glm::vec3& c, const std::string& n)
        : position(p), size(s), color(c), name(n), center(position.x, position.y, position.z)
    {
    }

};

struct RayMarchingSettings
{
	float maxDst = 10.0f;
	float epsilon = 0.05f;

	bool positionLight = false;
	glm::vec3 Light = { 0.9, 0.9, 0.9 };

	int numShapes = 1;
	std::vector<Shape> shapes;

    bool useP3GA = true;
};

class RayMarchingManager
{
public:
    RayMarchingManager(int width, int height);

    void update();

    glm::vec3 estimateNormal(const glm::vec3& p);

    glm::vec4 getSceneInfo(const Ray& eyeRay);

    Ray createCameraRay(const glm::vec2& uv);

    void free();

    // Getters
    Framebuffer& getFbo() { return _fbo; }
    Camera& getCamera() { return _camera; }
    int getCurrentSample() const { return currentSample; }
    int getNumShapes() const { return _settings.numShapes; }
    const std::vector<Shape>& getShapes() const { return _settings.shapes; }
    std::vector<Shape>& getShapes() { return _settings.shapes; }
    const Shape& getShapeAtIndex(int index) const { return _settings.shapes[index]; }
    Shape& getShapeAtIndex(int index) { return _settings.shapes[index]; }
    const std::vector<unsigned char>& getBuffer() const { return _buffer; }

    // Ray Marching Settings
    float& getEpsilon() { return _settings.epsilon; }
    float& getMaxDistance() { return _settings.maxDst; }
    bool& getUsePGA() { return _settings.useP3GA; }

    void UpdateView()
    {
        currentSample = 0;
        _rayOrigin = _camera.getCameraToWorld() * glm::vec4(0, 0, 0, 1);
        _needToUpdateRays = true;
    }

    void UpdateScene()
    {
        currentSample = 0;
        _needToUpdateRays = false;
    }


private:
    float GetShapeDistance(const Shape& shape, const Ray& eye);

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

    std::vector<std::vector<Ray> > _rays;

    int currentSample = 0;
    const int maxSamples = 1;

    bool _needToUpdateRays = true; // If camera move, we must compute new rays
};

