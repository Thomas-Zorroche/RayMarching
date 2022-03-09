// Display a Triangle in OpenGL with IBO

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Editor.hpp"
#include "Framebuffer.hpp"
#include "RayMarching.hpp"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "Ray Marching", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Initialize glad: load all OpenGL function pointers */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    int viewer3DWidth = 600;
    int viewer3DHeight = 400;
    Framebuffer fbo = Framebuffer(viewer3DWidth, viewer3DHeight);
    RayMarchingManager rayMarching(viewer3DWidth, viewer3DHeight);

    // Initialize ImGui
    initEditor(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.1, 0.15f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        rayMarching.update();

        fbo.update(rayMarching.getBuffer());

        drawEditor(fbo);
        renderEditor();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    fbo.free();
    glfwTerminate();
    return 0;
}