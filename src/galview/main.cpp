#include <iostream>

#include <glm/gtx/transform.hpp>

#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <galview/Widget.h>

#include <galcore/Circle2d.h>
#include <galcore/ConvexHull.h>
#include <galcore/ObjLoader.h>
#include <galcore/Plane.h>
#include <galcore/PointCloud.h>
#include <galview/GuiFunctions.h>

#include <fstream>
#include <sstream>

using namespace gal;
namespace fs = std::filesystem;

static void initPythonEnvironment()
{
  PyImport_AppendInittab("pygalfunc", &PyInit_pygalfunc);
  PyImport_AppendInittab("pygalview", &PyInit_pygalview);
  Py_Initialize();

  using namespace std::string_literals;
  gal::viewfunc::initPanels(view::newPanel("Inputs"s), view::newPanel("Outputs"s));
};

static void loadDemo(const fs::path& path)
{
  try {
    boost::python::exec_file(path.c_str());
  }
  catch (boost::python::error_already_set) {
    PyErr_Print();
  }
}

static void glfw_error_cb(int error, const char* desc)
{
  std::cerr << "Glfw Error " << error << ": " << desc << std::endl;
}

int main(int argc, char** argv)
{
  fs::path demoPath;
  if (argc < 2) {
    std::cout << "Please supply the filepath to the demo file as an argument.\n";
    // demoPath = "/home/rnjth94/dev/GeomAlgoLib/demos/staticBoundinCircle.py";
    return 1;
  }
  else {
    demoPath = fs::absolute(fs::path(argv[1]));
  }

  glfwSetErrorCallback(glfw_error_cb);
  std::cout << "Initializign GLFW...\n";
  if (!glfwInit())
    return 1;

  constexpr char glslVersion[] = "#version 330 core";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  std::cout << "...Opening the Window...\n";
  GLFWwindow* window = glfwCreateWindow(1600, 900, "First Attempt", nullptr, nullptr);
  if (window == nullptr)
    return 1;

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  std::cout << "...OpenGL bindings...\n";
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize OpenGLL loader!";
    return 1;
  }

  // Init shader.
  view::Context& ctx      = view::Context::get();
  size_t         shaderId = ctx.shaderId("default");
  ctx.useShader(shaderId);

  std::cout << "Setting up mouse event callbacks...\n";
  view::Context::registerCallbacks(window);
  view::Context::get().setPerspective();

  // Setup IMGUI
  view::initializeImGui(window, glslVersion);

  // Initialize Embedded Python and the demo
  initPythonEnvironment();
  loadDemo(demoPath.string());

  int W, H;
  glfwGetFramebufferSize(window, &W, &H);
  glViewport(0, 0, W, H);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //   glEnable(GL_LINE_SMOOTH);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(3.0f);
  glLineWidth(1.5f);

  std::cout << "Starting render loop...\n";

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    view::imGuiNewFrame();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      view::drawAllPanels();
      viewfunc::evalOutputs();
      view::imGuiRender();
      view::Context::get().render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}