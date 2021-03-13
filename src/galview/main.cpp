#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <galview/MeshView.h>
#include <galview/PlaneView.h>

#include <galcore/ObjLoader.h>
#include <galcore/Plane.h>

using namespace gal;

static Mesh createBoxMesh()
{
  // clang-format off
  static constexpr std::array<float, 24> sCoords = {
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
  };

  static constexpr std::array<size_t, 36> sIndices = {
    0, 3, 2, 0, 2, 1, // bottom
    0, 4, 7, 0, 7, 3, // left
    3, 7, 6, 3, 6, 2, // front
    5, 6, 7, 5, 7, 4, // top
    1, 2, 6, 1, 6, 5, // right
    0, 1, 5, 0, 5, 4, // back
  };
  // clang-format on

  return Mesh(sCoords.data(), sCoords.size() / 3, sIndices.data(), sIndices.size() / 3);
}

static Mesh loadBunny()
{
  return io::ObjMeshData("../assets/bunny_large.obj", true).toMesh();
  // return io::ObjMeshData("../assets/bunny.obj", true).toMesh();
}

static void glfw_error_cb(int error, const char* desc)
{
  std::cerr << "Glfw Error " << error << ": " << desc << std::endl;
}

int main(int argc, char** argv)
{
  glfwSetErrorCallback(glfw_error_cb);
  if (!glfwInit())
    return 1;

  constexpr char glsl_version[] = "#version 330 core";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1600, 900, "First Attempt", nullptr, nullptr);
  if (window == nullptr)
    return 1;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize OpenGLL loader!";
    return 1;
  }

  // auto view = view::MeshView::create(createBoxMesh());
  // auto view = view::MeshView::create(loadBunny());
  view::Context::get().addDrawable(loadBunny());
  view::Context::get().addDrawable(gal::Plane {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});

  // Init shader.
  view::Context& ctx      = view::Context::get();
  size_t         shaderId = ctx.shaderId("simple");
  ctx.useShader(shaderId);

  view::Context::registerCallbacks(window);
  view::Context::get().setPerspective();

  // Setup IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  ImGui::StyleColorsDark();  // Dark Mode

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  int W, H;
  glfwGetFramebufferSize(window, &W, &H);
  glViewport(0, 0, W, H);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  // view::Shader::setWireframeMode(true);
  glLineWidth(1.5f);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool demoWindow = true;
    ImGui::ShowDemoWindow(&demoWindow);

    {  // Populate the ImGui window.
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

    view::Context::get().render();
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