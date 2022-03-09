#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "RayMarching.hpp"

void initEditor(GLFWwindow* window);

void drawEditor(RayMarchingManager& rayMarching);

void renderEditor();