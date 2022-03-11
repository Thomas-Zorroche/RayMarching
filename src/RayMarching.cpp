#include "RayMarching.hpp"

#include "glm/gtx/transform.hpp"
#include "glm/gtx/compatibility.hpp"

#include <omp.h>
#include <iostream>


// Clamp between [0.0, 1.0]
float saturate(float x)
{
    return x > 1.f ? 1.f : (x < 0.f ? 0.f : x);
}

float lerp(float start, float end, float t)
{
    return start * (1 - t) + end * t;
}

glm::vec3 lerp(const glm::vec3& start, const glm::vec3& end, float t)
{
    return start * (1 - t) + end * t;
}

float SphereDistance(const glm::vec3& eye, const glm::vec3& centre, float radius) 
{
    return glm::distance(eye, centre) - radius;
}

float GetShapeDistance(const Shape& shape, const glm::vec3& eye)
{
    return SphereDistance(eye, shape.position, shape.size.x);

    //if (shape.shapeType == 0) {
    //    return SphereDistance(eye, shape.position, shape.size.x);
    //}
    //else if (shape.shapeType == 1) {
    //    return CubeDistance(eye, shape.position, shape.size);
    //}
    //else if (shape.shapeType == 2) {
    //    return TorusDistance(eye, shape.position, shape.size.x, shape.size.y);
    //}

    //return maxDst;
}

// polynomial smooth min (k = 0.1);
// from https://www.iquilezles.org/www/articles/smin/smin.htm
glm::vec4 Blend(float a, float b, const glm::vec3& colA, const glm::vec3& colB, float k)
{
    float h = saturate(0.5f + 0.5f * (b - a) / k);
    float blendDst = lerp(b, a, h) - k * h * (1.0f - h);
    glm::vec3 blendCol = glm::lerp(colB, colA, h);
    return glm::vec4(blendCol, blendDst);
}

glm::vec4 Combine(float dstA, float dstB, const glm::vec3& colorA, const glm::vec3& colorB, EOperation operation, float blendStrength)
{
    switch (operation)
    {
    case EOperation::DEFAULT:
        if (dstB < dstA)
        {
            return glm::vec4(colorB, dstB);
        }
    case EOperation::BLEND:
        return(Blend(dstA, dstB, colorA, colorB, blendStrength));
    }

    //// Cut
    //else if (operation == 2) {
    //    // max(a,-b)
    //    if (-dstB > dst) {
    //        dst = -dstB;
    //        colour = colourB;
    //    }
    //}
    //// Mask
    //else if (operation == 3) {
    //    // max(a,b)
    //    if (dstB > dst) {
    //        dst = dstB;
    //        colour = colourB;
    //    }
    //}

    return glm::vec4(colorA, dstA);
}

RayMarchingManager::RayMarchingManager(int width, int height)
    : _camera(Camera(width, height)),
    _width(width),
    _height(height),
    _nbpixels(_width * _height),
    _bufferSize(_nbpixels * 3 /* RGB */),
    _buffer(std::vector<unsigned char>(_bufferSize)),
    _fbo(Framebuffer(width, height, _buffer))
{
    int value = 125;
    for (int i = 0; i < _bufferSize; ++i)
    {
        _buffer[i] = (unsigned char)(value);
    }

    _settings.shapes = std::vector<Shape>({
        //    position   size       color
        Shape({0, 0, 0}, { 1, 1, 1 }, {255, 150, 0}, "Orange Sphere"),
        Shape({1, 1, 0}, { .75, .75, .75}, {0, 150, 0}, "Green Sphere")
    });

    _rayOrigin = _camera.getCameraToWorld() * glm::vec4(0, 0, 0, 1); 

    for (size_t i = 0; i < 20; i++)
    {
        _rays.push_back(std::vector<Ray>(_nbpixels));
    }
}


Ray RayMarchingManager::createCameraRay(const glm::vec2& uv)
{
    glm::vec3 direction = _camera.getCameraInverseProjection() * glm::vec4(uv, 0, 1);
    direction = _camera.getCameraToWorld() * glm::vec4(direction, 0);
    direction = normalize(direction);
    return Ray(_rayOrigin, direction);
}

glm::vec4 RayMarchingManager::getSceneInfo(const glm::vec3& eye)
{
    float globalDst = _settings.maxDst;
    glm::vec3 globalColour = glm::vec3(1);

    for (int i = 0; i < _settings.numShapes; i++) {
        Shape& shape = _settings.shapes[i];
        //int numChildren = shape.numChildren;

        float localDst = GetShapeDistance(shape, eye);
        const glm::vec3& localColour = shape.color;


        //for (int j = 0; j < numChildren; j++) {
        //    Shape childShape = shapes[i + j + 1];
        //    float childDst = GetShapeDistance(childShape, eye);

        //    glm::vec4 combined = Combine(localDst, childDst, localColour, childShape.colour, childShape.operation, childShape.blendStrength);
        //    localColour = combined;
        //    localDst = combined.w;
        //}
        //i += numChildren; // skip over children in outer loop

        glm::vec4 globalCombined = Combine(globalDst, localDst, globalColour, localColour, shape.operation, shape.blendStrength);
        globalColour = globalCombined;
        globalDst = globalCombined.w;

    }

    return glm::vec4(globalColour, globalDst);
}

glm::vec3 RayMarchingManager::estimateNormal(const glm::vec3& p)
{
    float x = getSceneInfo(glm::vec3(p.x + _settings.epsilon, p.y, p.z)).w - getSceneInfo(glm::vec3(p.x - _settings.epsilon, p.y, p.z)).w;
    float y = getSceneInfo(glm::vec3(p.x, p.y + _settings.epsilon, p.z)).w - getSceneInfo(glm::vec3(p.x, p.y - _settings.epsilon, p.z)).w;
    float z = getSceneInfo(glm::vec3(p.x, p.y, p.z + _settings.epsilon)).w - getSceneInfo(glm::vec3(p.x, p.y, p.z - _settings.epsilon)).w;
    return normalize(glm::vec3(x, y, z));
}

void RayMarchingManager::updateRays()
{

}


void RayMarchingManager::update()
{
    if (currentSample == maxSamples)
    {
        return;
    }

    if (_needToUpdateRays)
    {
        _rays.empty();
    }

    glm::vec4 sceneInfo;
    int pixelID = 0;
    int rayID = 0;
    
    int step = maxSamples - currentSample;

    int idThread;
    int NUM_THREADS = omp_get_max_threads();
    int size_per_thread = _bufferSize / NUM_THREADS;

    #pragma omp parallel shared(step) private(idThread, pixelID, rayID, sceneInfo) num_threads(NUM_THREADS)
    {
        idThread = omp_get_thread_num(); // Get current thread number
        pixelID = idThread * size_per_thread / 3;
        rayID = idThread * size_per_thread / (step * 3);

        #pragma omp parallel for num_threads(NUM_THREADS)
        for (int bufferID = size_per_thread * idThread; bufferID < _bufferSize; bufferID += step * 3) 
        {
            bool hit = false;
            float rayDst = 1;
            int marchSteps = 0;

            Ray ray;
            if (_needToUpdateRays)
            {
                int idPixelX = pixelID % _width;
                int idPixelY = pixelID / _width;
                glm::vec2 uv = glm::vec2(idPixelX / (float)_width, idPixelY / (float)_height) * glm::vec2(2.f, 2.f) - glm::vec2(1.f, 1.f);
                ray = createCameraRay(uv);
                _rays[currentSample][rayID] = ray;
                //if (bufferID % 1000 == 0)
                //    std::cout << "CREATE RAY" << std::endl;
            }
            else
            {
                //if (bufferID % 1000 == 0)
                //    std::cout << "USE RAY" << std::endl;
                ray = _rays[currentSample][rayID];
            }

            while (rayDst < _settings.maxDst)
            {
                marchSteps++;
                sceneInfo = getSceneInfo(ray.origin);

                float dst = sceneInfo.w;
                if (dst < _settings.epsilon)
                {
                    //std::cout << i << " - " << marchSteps << std::endl;

                    glm::vec3 pointOnSurface = ray.origin + ray.direction * dst;
                    glm::vec3 normal = estimateNormal(pointOnSurface - ray.direction * _settings.epsilon);
                    glm::vec3 lightDir = (_settings.positionLight) ? normalize(_settings.Light - ray.origin) : -_settings.Light;
                    float lighting = saturate(saturate(dot(normal, lightDir)));
                    //float lighting = 1.0f;
                    glm::vec3 col = sceneInfo;

                    // Shadow
                    //float3 offsetPos = pointOnSurface + normal * shadowBias;
                    //float3 dirToLight = (positionLight) ? normalize(_Light - offsetPos) : -_Light;

                    //ray.origin = offsetPos;
                    //ray.direction = dirToLight;

                    //float dstToLight = (positionLight) ? distance(offsetPos, _Light) : maxDst;
                    //float shadow = CalculateShadow(ray, dstToLight);

                    _buffer[bufferID] = (unsigned char)(col.r * lighting);
                    _buffer[bufferID + 1] = (unsigned char)(col.g * lighting);
                    _buffer[bufferID + 2] = (unsigned char)(col.b * lighting);
                    hit = true;

                    break;
                }

                ray.origin += ray.direction * dst;
                rayDst += dst;
            }

            if (!hit)
            {
                _buffer[bufferID] =    (unsigned char)(120);
                _buffer[bufferID + 1] = (unsigned char)(120);
                _buffer[bufferID + 2] = (unsigned char)(120);
            }

            pixelID += step;
            rayID++;
        }
    }

    if (currentSample < maxSamples)
    {
        currentSample++;
    }
    else if (_needToUpdateRays)
    {
        _needToUpdateRays = false;
    }

    _fbo.update(_buffer);
}


void RayMarchingManager::free()
{
    _fbo.free();
}
