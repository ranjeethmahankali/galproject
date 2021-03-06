#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static void glfw_error_cb(int error, const char* desc)
{
  std::cerr << "Glfw Error " << error << ": " << desc << std::endl;
}

int main(int argc, char** argv)
{
  glfwSetErrorCallback(glfw_error_cb);
  if (!glfwInit())
    return 1;

  constexpr char glsl_version[] = "#version 430";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow* window = glfwCreateWindow(1280, 720, "First Attempt", nullptr, nullptr);
  if (window == nullptr)
    return 1;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize OpenGLL loader!";
    return 1;
  }

  // Setup IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  ImGui::StyleColorsDark();  // Dark Mode

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool demoWindow = true;
    ImGui::ShowDemoWindow(&demoWindow);

    {  // Populate the ImGui window.
      static float f       = 0.0f;
      static int   counter = 0;
      ImGui::Begin("Hello, world!");

      ImGui::Text("This is some useful text.");
      float                  pt[3];
      static constexpr float min = 0.0f;
      static constexpr float max = 1.0f;
      ImGui::SliderFloat3("Test coords", pt, min, max);
      ImGui::Text("This is just some text");
      ImGui::End();
    }

    ImGui::Render();
    int W, H;
    glfwGetFramebufferSize(window, &W, &H);
    glViewport(0, 0, W, H);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}