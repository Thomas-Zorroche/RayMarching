#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Framebuffer.hpp"

void initEditor(GLFWwindow* window);

void drawEditor(Framebuffer& fbo);

void renderEditor();