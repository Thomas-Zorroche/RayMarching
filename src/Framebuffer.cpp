#include "Framebuffer.hpp"

#include <glad/glad.h>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

Framebuffer::Framebuffer(float width, float height, const std::vector<unsigned char>& buffer)
{
    _width = width;
    _height = height;

    glGenFramebuffers(1, &_id);
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    {
        // Create Texture
        glGenTextures(1, &_textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
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


void Framebuffer::update(const std::vector<unsigned char>& buffer)
{
    glBindTexture(GL_TEXTURE_2D, _textureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
}


