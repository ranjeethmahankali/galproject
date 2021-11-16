#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/program_options.hpp>
#include <glm/gtx/transform.hpp>

#include <galcore/Circle2d.h>
#include <galcore/ConvexHull.h>
#include <galcore/ObjLoader.h>
#include <galcore/Plane.h>
#include <galcore/PointCloud.h>
#include <galcore/Util.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <galview/GuiFunctions.h>
#include <galview/Views.h>
#include <galview/Widget.h>

using namespace gal;
namespace fs  = std::filesystem;
namespace bpo = boost::program_options;

static constexpr char glslVersion[]    = "#version 330 core";
static fs::path       sCurrentDemoPath = "";

void initPythonEnvironment()
{
  PyImport_AppendInittab("pygalfunc", &PyInit_pygalfunc);
  PyImport_AppendInittab("pygalview", &PyInit_pygalview);
  Py_Initialize();

  using namespace std::string_literals;
  gal::viewfunc::initPanels(view::newPanel("Inputs"s), view::newPanel("Outputs"s));
};

int runPythonDemoFile(const fs::path& demoPath)
{
  try {
    std::cout << "Running demo file: " << demoPath << std::endl;
    sCurrentDemoPath = demoPath;
    boost::python::dict global;
    global["__file__"] = demoPath.string();
    global["__name__"] = "__main__";
    boost::python::exec_file(demoPath.c_str(), global);
    return 0;
  }
  catch (boost::python::error_already_set) {
    PyErr_Print();
    std::cerr << "Unable to load the demo... aborting...\n";
    return 1;
  }
}

void glfw_error_cb(int error, const char* desc)
{
  std::cerr << "Glfw Error " << error << ": " << desc << std::endl;
}

int initViewer(GLFWwindow*& window)
{
  glfwSetErrorCallback(glfw_error_cb);
  std::cout << "Initializign GLFW...\n";
  if (!glfwInit())
    return 1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  std::cout << "...Opening the Window...\n";
  window = glfwCreateWindow(1920, 1080, "galview", nullptr, nullptr);
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
  return 0;
}

void showSettingsPanel()
{
  static view::Panel& panel = view::newPanel("Settings");
  panel.newWidget<view::Button>("Toggle 2d Mode",
                                []() { view::Context::get().toggle2dMode(); });
  panel.newWidget<view::Button>("Reload demo", []() {
    gal::viewfunc::unloadAllOutputs();
    gal::func::store::unloadAllFunctions();
    gal::view::Views::clear();
    int err = runPythonDemoFile(sCurrentDemoPath);
    if (err != 0) {
      std::cerr << "Unable to run the demo file. Aborting...\n";
      std::exit(err);
    }
  });
}

void wrapUp(GLFWwindow* window)
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
}

int loadDemo(const fs::path& demoPath)
{
  int         err    = 0;
  GLFWwindow* window = nullptr;
  if ((err = initViewer(window))) {
    std::cerr << "Failed to initialize the viewer\n";
    return err;
  }

  // Setup IMGUI
  view::initializeImGui(window, glslVersion);

  // Initialize Embedded Python and the demo
  initPythonEnvironment();
  err = runPythonDemoFile(demoPath);
  if (err != 0) {
    std::cerr << "Unable to run the demo file. Aborting...\n";
    return err;
  }

  std::cout << "Starting render loop...\n";
  showSettingsPanel();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    view::imGuiNewFrame();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
      view::drawAllPanels();
      view::imGuiRender();
      viewfunc::evalOutputs();
      view::Views::render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    glfwSwapBuffers(window);
  }

  wrapUp(window);
  return 0;
}

int main(int argc, char** argv)
{
  static constexpr char    pathKey[] = "path";
  bpo::options_description desc("galview options");
  desc.add_options()("help", "produce help message");
  bpo::options_description hidden("hidden options");
  hidden.add_options()(pathKey, "Path to run the program with.");
  bpo::options_description allOptions;
  allOptions.add(desc).add(hidden);

  bpo::positional_options_description posn;
  posn.add(pathKey, 1);

  bpo::variables_map vmap;
  try {
    bpo::store(
      bpo::command_line_parser(argc, argv).options(allOptions).positional(posn).run(),
      vmap);
    bpo::notify(vmap);
  }
  catch (const bpo::error& err) {
    std::cerr << "Couldn't parse command line arguments properly:\n";
    std::cerr << err.what() << '\n' << '\n';
    std::cerr << desc << '\n';
    return 1;
  }

  if (vmap.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  if (!vmap.count(pathKey)) {
    std::cerr << "Please provide a path to a demo file, or a directory for debugging or "
                 "postmortem";
    std::cerr << desc << '\n';
    return 1;
  }

  fs::path path = fs::absolute(fs::path(vmap[pathKey].as<std::string>()));

  if (fs::exists(path)) {
    return loadDemo(path);
  }
  else {
    std::cerr << "The given path does not exist!\n";
    return 1;
  }
}
