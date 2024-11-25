// ==========================================================-GLAD_setup-==============================================================================
// https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.6&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=core&extensions=GL_ARB_arrays_of_arrays&extensions=GL_ARB_buffer_storage&extensions=GL_ARB_clear_buffer_object&extensions=GL_ARB_clear_texture&extensions=GL_ARB_compute_shader&extensions=GL_ARB_copy_buffer&extensions=GL_ARB_copy_image&extensions=GL_ARB_direct_state_access&extensions=GL_ARB_shader_objects&extensions=GL_ARB_shader_storage_buffer_object&extensions=GL_ARB_texture_buffer_object&extensions=GL_ARB_texture_buffer_object_rgb32&loader=on
// ====================================================================================================================================================
#include "waves.h"

#include "../imgui-src/imgui.h"
#include "../imgui-src/imgui_impl_glfw.h"

#include "../opengl-objects/computeShader.h"
#include "../opengl-objects/framebuffer.h"
#include "../opengl-objects/shader.h"
#include "../opengl-objects/shaderStorageBuffer.h"
#include "../opengl-objects/texture.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int TEXTURE_WIDTH = 1920, TEXTURE_HEIGHT = 1080;

GLuint VAO, VBO, FBO, RBO;

int WINDOW_WIDTH, WINDOW_HEIGHT;

int main() {
  GLFWwindow *window;
  if (glfwSetup(window) == -1)
    return -1;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGuiWindowFlags wFlags;
  // wFlags |= ImGuiWindowFlags_NoTitleBar;
  // wFlags |= ImGuiWindowFlags_NoResize;
  // wFlags |= ImGuiWindowFlags_NoScrollbar;
  // wFlags |= ImGuiWindowFlags_NoScrollWithMouse;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.5f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  std::string project = "/waves";

  // Shader creation
  Shader shader(project, "vertexShader.vs.glsl", "fragmentShader.fs.glsl");
  ComputeShader computeShader(project, "computeShaderLinear");
  ComputeShader dataSyncShader(project, "dataSyncLinear");

  // Texture creation
  Texture texFB(0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
  Texture texCS(1, TEXTURE_WIDTH, TEXTURE_HEIGHT);

  Framebuffer fbo(&texFB);

  std::vector<Wave> data((TEXTURE_WIDTH + 2) * (TEXTURE_HEIGHT + 2));
  int simSpeed = 100;

  for (size_t j = 1; j < TEXTURE_HEIGHT + 1; j++) {
    for (size_t i = 1; i < TEXTURE_WIDTH + 1; i++) {
      if (j > TEXTURE_HEIGHT / 2 - 10 && j < TEXTURE_HEIGHT / 2 + 10 &&
          i > TEXTURE_WIDTH / 2 - 10 && i < TEXTURE_WIDTH / 2 + 10) {
        data[j * TEXTURE_WIDTH + i] = {10, 10, 10, 1.0};
      } else {
        data[j * TEXTURE_WIDTH + i] = {0.0, 0.0, 0.0, 1.0};
      }
    }
  }

  ShaderStorageBuffer ssbo(3);

  ssbo.storeData((const void *)&data[0], sizeof(Wave) * data.size());

  GLuint vbo, vao;
  std::array<int, 4> screenPosArray{};
  int windowPadding = 1;
  glfwGetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
  std::array<int, 2> buttonFrameSize = {200, WINDOW_WIDTH};

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 440");

  const char *fps = "FPS";
  bool runIt = true;
  bool pause = false;
  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    // processInput(window);

    auto startTime = glfwGetTime();
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    fbo.bindFramebuffer();
    ImGui::NewFrame();
    {
      ImGui::SetNextWindowSize(ImVec2(buttonFrameSize[0], WINDOW_HEIGHT));
      // ImGui::SetCursorPos(ImVec2(0, 0));
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      // Create Buttons window
      ImGui::Begin("Parameters");
      ImVec2 initPos = ImGui::GetCursorPos();
      ImGui::PushID(99);
      if (ImGui::Button(fps)) {
        std::cout << "True" << std::endl;
        glfwSetWindowShouldClose(window, 1);
      }
      static char shaderFile[32] = "computeShader";
      ImGui::PopID();
      ImGui::SliderInt("Speed", &simSpeed, 100, 0);
      if (ImGui::Button("||")) {
        pause = true;
        runIt = false;
      }
      ImGui::SameLine();
      if (ImGui::Button(">"))
        runIt = true;
      ImGui::SameLine();
      if (ImGui::Button(">>")) {
        runIt = true;
        pause = false;
      }

      int padding = ImGui::GetStyle().FramePadding.y + 1;
      ImGui::SetCursorPos(ImVec2(8, WINDOW_HEIGHT - initPos.y - padding -
                                        ImGui::GetFrameHeight()));
      if (ImGui::Button("Recompile Shader")) {
        if (sizeof(shaderFile) / sizeof(shaderFile[0]) < 40) {
          std::cout << "Recompiling script: " << shaderFile << std::endl;
          shaderInputCallback(computeShader, shaderFile);
        }
      }
      ImGui::InputText("Shader", shaderFile, IM_ARRAYSIZE(shaderFile));
      ImGui::End();
    }

    {
      screenPosArray[2] = screenPosArray[0] + buttonFrameSize[0];
      ImGui::SetNextWindowPos(ImVec2(screenPosArray[2], screenPosArray[3]));
      ImGui::SetNextWindowSize(
          ImVec2(WINDOW_WIDTH - screenPosArray[2], WINDOW_HEIGHT));
      ImGui::Begin("Scene");
      const float windowWidth = ImGui::GetContentRegionAvail().x;
      const float windowHeight = ImGui::GetContentRegionAvail().y;

      fbo.rescaleFramebuffer(windowWidth, windowHeight);
      glViewport(0, 0, windowWidth, windowHeight);

      ImVec2 pos = ImGui::GetCursorScreenPos();

      ImGui::Image((ImTextureID)(intptr_t)texCS.texture,
                   ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
      ImGui::End();
    }

    ImGui::Render();
    fbo.unbindFramebuffer();

    if (runIt) {
      texCS.bindTexture();

      computeShader.use();
      computeShader.setUIvec2("texSize", TEXTURE_WIDTH, TEXTURE_HEIGHT);
      computeShader.setInt("imgOutput", texCS.texNum);
      glDispatchCompute((unsigned int)TEXTURE_WIDTH * TEXTURE_HEIGHT / 1024, 1,
                        1);
      // glDispatchCompute((unsigned int)TEXTURE_WIDTH / 128,
      //                   (unsigned int)TEXTURE_HEIGHT / 8, 1);

      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // This shader transfers the data from the previous
      // iteration to the current
      //
      // uPrev = u
      // u = uNext
      //
      // to prevent data races conditions
      dataSyncShader.use();
      dataSyncShader.setUIvec2("texSize", TEXTURE_WIDTH, TEXTURE_HEIGHT);
      dataSyncShader.setInt("imgOutput", texCS.texNum);
      glDispatchCompute((unsigned int)TEXTURE_WIDTH * TEXTURE_HEIGHT / 1024, 1,
                        1);
      // glDispatchCompute((unsigned int)TEXTURE_WIDTH / 128,
      //                   (unsigned int)TEXTURE_HEIGHT / 8, 1);

      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      texCS.unbindTexture();
      runIt = pause ? false : true;
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backupCurrentContext = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backupCurrentContext);
    }

    glfwSwapBuffers(window);

    std::this_thread::sleep_for(
        std::chrono::microseconds(100 * (100 - simSpeed)));
    fps = std::to_string((glfwGetTime() - startTime)).c_str();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void createBox(GLuint &vbo, GLuint &vao) {
  float quadVertices[] = {
      // positions        // texture Coords
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
  };
  // setup plane vao
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}

void renderQuad(GLuint &vao) {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void shaderInputCallback(ComputeShader &shader, const char *file) {
  shader.fileName = file;
  shader.update();
}

// glfw: whenever the window size changed (by OS or user resize) this
// callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
  WINDOW_WIDTH = width;
  WINDOW_HEIGHT = height;
  glViewport(0, 0, width, height);
}

int glfwSetup(GLFWwindow *&window) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // glfw window creation
  // --------------------

  window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Test Window",
                            glfwGetPrimaryMonitor(), NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSwapInterval(0);
  return 0;
}
