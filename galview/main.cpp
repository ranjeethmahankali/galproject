#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <pybind11/embed.h>
#include <cxxopts.hpp>
#include <glm/gtx/transform.hpp>

#include <Circle2d.h>
#include <Command.h>
#include <Context.h>
#include <ConvexHull.h>
#include <GLUtil.h>
#include <GuiFunctions.h>
#include <Interaction.h>
#include <Plane.h>
#include <PointCloud.h>
#include <Util.h>
#include <Views.h>

using namespace gal;
namespace fs = std::filesystem;

static constexpr char glslVersion[] = "#version 330 core";

void initPythonEnvironment()
{
  PyImport_AppendInittab("pygalfunc", &PyInit_pygalfunc);
  PyImport_AppendInittab("pygalview", &PyInit_pygalview);
};

void glfw_error_cb(int error, const char* desc)
{
  glutil::logger().error("GLFW Error {}: {}", error, desc);
}

int initViewer(GLFWwindow*& window, const std::string& filename)
{
  static constexpr int WIDTH  = 1920;
  static constexpr int HEIGHT = 1080;
  glfwSetErrorCallback(glfw_error_cb);
  if (!glfwInit()) {
    glutil::logger().error("Failed to initialize GLFW.");
    return 1;
  }
  glutil::logger().info("Initialized GLFW.");
  // Window setup
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  std::string title = "galview - " + filename;
  window            = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);
  // OpenGL bindings
  if (glewInit() != GLEW_OK) {
    glutil::logger().error("Failed to initialize OpenGL bindings.");
    return 1;
  }
  glutil::logger().info("OpenGL bindings are ready.");
  // Init shader.
  view::Context& ctx = view::Context::get();
  ctx.init(window);
  size_t shaderId = ctx.shaderId("default");
  ctx.useShader(shaderId);
  // Mouse support
  view::Context::registerCallbacks(window);
  glutil::logger().debug(
    "Registered mouse event callbacks to allow viewer interactions.");
  view::Context::get().setProjectionMode(view::Context::Projection::PERSPECTIVE);
  int W, H;
  glfwGetFramebufferSize(window, &W, &H);
  GL_CALL(glViewport(0, 0, W, H));
  GL_CALL(glEnable(GL_DEPTH_TEST));
  GL_CALL(glEnable(GL_BLEND));
  GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL_CALL(glEnable(GL_LINE_SMOOTH));
  GL_CALL(glEnable(GL_PROGRAM_POINT_SIZE));
  GL_CALL(glPointSize(3.0f));
  GL_CALL(glLineWidth(1.0f));
  return 0;
}

int loadDemo(const fs::path& demoPath)
{
  int         err    = 0;
  GLFWwindow* window = nullptr;
  try {
    if ((err = initViewer(window, demoPath.filename().string()))) {
      glutil::logger().error("Failed to initialize the viewer. Error code {}.", err);
      return err;
    }
    // Initialize all the user interface elements.
    view::init(window, glslVersion);
    // Initialize Embedded Python and the demo
    initPythonEnvironment();
    py::scoped_interpreter guard {};
    err = view::runPythonDemoFile(demoPath);
    if (err != 0) {
      glutil::logger().error("Unable to run the demo file. Error code {}.", err);
      return err;
    }
    // Render loop.
    glutil::logger().info("Starting the render loop...");
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      view::imGuiNewFrame();
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      {
        view::draw(window);
        ImGui::Render();
        viewfunc::evalOutputs();
        view::Views::render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      }
      glfwSwapBuffers(window);
      gal::view::runQueuedCommands();
    }
  }
  catch (const std::exception& e) {
    glutil::logger().critical("Fatal error: {}", e.what());
    err = -1;
  }
  view::destroy(window);
  return err;
}

int main(int argc, char** argv)
{
  cxxopts::Options opts("galview", "Visualize the gal demos written in python");
  fs::path         path;
  // clang-format off
  opts
    .allow_unrecognised_options()
    .add_options()
    ("help", "Print help")
    ("filepath", "Path to the demo file", cxxopts::value<fs::path>(path), "<filepath>");
  // clang-format on
  opts.positional_help("<path/to/demo/file>");
  opts.parse_positional({"filepath"});
  auto parsed = opts.parse(argc, argv);
  if (parsed.count("help")) {
    std::cout << opts.help() << std::endl;
    return 0;
  }
  if (parsed.count("filepath") == 0) {
    std::cerr << "Please provide the path to the demo file.\n";
    return 1;
  }
  if (!fs::is_regular_file(path) || !fs::exists(path)) {
    std::cerr << "The given path does not point to a an existing file.\n";
    return 1;
  }
  return loadDemo(path);
}
