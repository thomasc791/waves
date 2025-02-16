#ifndef WINDOW_H
#define WINDOW_H

#include "../opengl/imgui-src/imgui.h"
#include "../opengl/imgui-src/imgui_impl_glfw.h"
#include "../opengl/imgui-src/imgui_impl_opengl3.h"
#include "../opengl/opengl-objects/computeShader.h"
#include "../opengl/opengl-objects/shader.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>

// #define RECORD_VIDEO

struct Wave {
  float uPrev;
  float u;
  float uNext;
  float _padding;
};

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
std::string readShader(std::string fileName);
void checkCompileErrors(unsigned int shader, std::string shaderType);
void renderQuad(GLuint &vao);
void createBox(GLuint &vbo, GLuint &vao);
void createTexture(unsigned int &tex, unsigned int texNum);
void createFramebuffer();
void shaderInputCallback(ComputeShader &shader, const char *file);
void rescaleFramebuffer(float width, float height);
int glfwSetup(GLFWwindow *&window);

#endif
