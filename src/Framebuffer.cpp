#include "Framebuffer.hpp"

#include <glad/glad.h>
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

Framebuffer::Framebuffer(float width, float height)
{
    _width = width;
    _height = height;

    glGenFramebuffers(1, &_id);
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    {
        // Create Texture
        glGenTextures(1, &_textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Attach Texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);

        glGenRenderbuffers(1, &_rboID);
        glBindRenderbuffer(GL_RENDERBUFFER, _rboID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rboID);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(float width, float height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    {
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, _rboID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind(float viewportWidth, float viewportHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::free()
{
    glDeleteFramebuffers(1, &_id);
    glDeleteTextures(1, &_textureID);
    glDeleteRenderbuffers(1, &_rboID);
}

// RAY MARCHING ALGORITHM
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

static const float maxDst = 10.0f;
static const float epsilon = 0.1f;
static bool positionLight = false;
static glm::vec3 Light = { 0.9, 0.9, 0.9 };
static const int numShapes = 1;
static std::vector<Shape> shapes;

static glm::mat4 CameraToWorld;
static glm::mat4 CameraInverseProjection;

float saturate(float x)
{
    return x > 1.f ? 1.f : (x < 0.f ? 0.f : x);
}

Ray createCameraRay(glm::vec2 uv) {
    glm::vec3 origin = CameraToWorld * glm::vec4(0, 0, 0, 1);
    glm::vec3 direction = CameraInverseProjection * glm::vec4(uv, 0, 1);
    direction = CameraToWorld * glm::vec4(direction, 0);
    direction = normalize(direction);
    return Ray(origin, direction);
}

float SphereDistance(glm::vec3 eye, glm::vec3 centre, float radius) {
    return glm::distance(eye, centre) - radius;
}

float GetShapeDistance(Shape shape, glm::vec3 eye) 
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

glm::vec4 Combine(float dstA, float dstB, glm::vec3 colourA, glm::vec3 colourB, int operation, float blendStrength) {
    float dst = dstA;
    glm::vec3 colour = colourA;

    if (operation == 0) {
        if (dstB < dstA) {
            dst = dstB;
            colour = colourB;
        }
    }
    // Blend
    //else if (operation == 1) {
    //    glm::vec4 blend = Blend(dstA, dstB, colourA, colourB, blendStrength);
    //    dst = blend.w;
    //    colour = blend.xyz;
    //}
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

    return glm::vec4(colour, dst);
}

glm::vec4 getSceneInfo(glm::vec3 eye) {
    float globalDst = maxDst;
    glm::vec3 globalColour = glm::vec3(1);

    for (int i = 0; i < numShapes; i++) {
        Shape shape = shapes[i];
        //int numChildren = shape.numChildren;

        float localDst = GetShapeDistance(shape, eye);
        glm::vec3 localColour = shape.color;


        //for (int j = 0; j < numChildren; j++) {
        //    Shape childShape = shapes[i + j + 1];
        //    float childDst = GetShapeDistance(childShape, eye);

        //    glm::vec4 combined = Combine(localDst, childDst, localColour, childShape.colour, childShape.operation, childShape.blendStrength);
        //    localColour = combined;
        //    localDst = combined.w;
        //}
        //i += numChildren; // skip over children in outer loop

        glm::vec4 globalCombined = Combine(globalDst, localDst, globalColour, localColour, /*shape.operation*/0, /*shape.blendStrength*/0);
        globalColour = globalCombined;
        globalDst = globalCombined.w;
    }

    return glm::vec4(globalColour, globalDst);
}

glm::vec3 estimateNormal(glm::vec3 p) {
    float x = getSceneInfo(glm::vec3(p.x + epsilon, p.y, p.z)).w - getSceneInfo(glm::vec3(p.x - epsilon, p.y, p.z)).w;
    float y = getSceneInfo(glm::vec3(p.x, p.y + epsilon, p.z)).w - getSceneInfo(glm::vec3(p.x, p.y - epsilon, p.z)).w;
    float z = getSceneInfo(glm::vec3(p.x, p.y, p.z + epsilon)).w - getSceneInfo(glm::vec3(p.x, p.y, p.z - epsilon)).w;
    return normalize(glm::vec3(x, y, z));
}

void Framebuffer::update()
{
    int nbpixels = (int)_width * (int)_height * 3;
    std::vector<unsigned char> buffer = std::vector<unsigned char>(nbpixels);
    int value = 125;
    for (int i = 0; i < nbpixels; ++i)
    {
        buffer[i] = (unsigned char)(value);
    }

    shapes.push_back({
        //    position   size       color
        Shape({0, 0, 0}, {2, 2, 2 }, {255, 150, 0})
    });

    glm::vec3 eye = { 0, 0, -5 };
    glm::vec3 center = { 0, 0, 0 };
    glm::vec3 up = { 0, 1, 0 };

    CameraToWorld = glm::lookAt(eye, center, up);
    const auto projMatrix = glm::perspective(70.f, float(_width) / _height, 0.001f, 1000.0f);
    CameraInverseProjection = glm::inverse(projMatrix);

    glm::vec4 sceneInfo;
    int pixelID = 0;
    for (int bufferID = 0; bufferID < nbpixels; bufferID +=3)
    {
        float rayDst = 0;
        int marchSteps = 0;

        int idPixelX = pixelID % _width;
        int idPixelY = pixelID / _width;
        glm::vec2 uv = glm::vec2(idPixelX / (float)_width, idPixelY / (float)_height) * glm::vec2(2.f, 2.f) - glm::vec2(1.f, 1.f);
        Ray ray = createCameraRay(uv);
        while (rayDst < maxDst)
        {
            marchSteps++;
            sceneInfo = getSceneInfo(ray.origin);

            float dst = sceneInfo.w;
            if (dst < epsilon)
            {
                //std::cout << i << " - " << marchSteps << std::endl;

                glm::vec3 pointOnSurface = ray.origin + ray.direction * dst;
                glm::vec3 normal = estimateNormal(pointOnSurface - ray.direction * epsilon);
                glm::vec3 lightDir = (positionLight) ? normalize(Light - ray.origin) : -Light;
                float lighting = saturate(saturate(dot(normal, lightDir)));
                glm::vec3 col = sceneInfo;

                // Shadow
                //float3 offsetPos = pointOnSurface + normal * shadowBias;
                //float3 dirToLight = (positionLight) ? normalize(_Light - offsetPos) : -_Light;

                //ray.origin = offsetPos;
                //ray.direction = dirToLight;

                //float dstToLight = (positionLight) ? distance(offsetPos, _Light) : maxDst;
                //float shadow = CalculateShadow(ray, dstToLight);

                buffer[bufferID]     = (unsigned char)(col.r * lighting);
                buffer[bufferID + 1] = (unsigned char)(col.g * lighting);
                buffer[bufferID + 2] = (unsigned char)(col.b * lighting);

                break;
            }

            ray.origin += ray.direction * dst;
            rayDst += dst;
        }
        pixelID++;
    }


    glGenFramebuffers(1, &_id);
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    {
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Attach Texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureID, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


