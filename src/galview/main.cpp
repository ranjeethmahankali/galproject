#include <iostream>

#include <glm/gtx/transform.hpp>

#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/GLUtil.h>
#include <galview/Widget.h>

#include <galcore/ConvexHull.h>
#include <galcore/ObjLoader.h>
#include <galcore/Plane.h>
#include <galcore/PointCloud.h>

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

static Mesh loadBunnyLarge()
{
  return io::ObjMeshData("../assets/bunny_large.obj", true).toMesh();
}

static Mesh loadBunnySmall()
{
  auto mesh = io::ObjMeshData("../assets/bunny.obj", true).toMesh();
  mesh.transform(glm::scale(glm::vec3(10.0f, 10.0f, 10.0f)));
  return mesh;
}

static void glfw_error_cb(int error, const char* desc)
{
  std::cerr << "Glfw Error " << error << ": " << desc << std::endl;
}

static void meshPlaneClippingDemo()
{
  auto mesh  = loadBunnySmall();
  auto plane = gal::Plane {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}};
  mesh.clipWithPlane(plane);
  view::Context::get().addDrawable(mesh);
  view::Context::get().addDrawable(plane);
}

static void convexHullDemo()
{
  auto       mesh = loadBunnySmall();
  ConvexHull hull(mesh.vertexCBegin(), mesh.vertexCEnd());
  view::Context::get().addDrawable(hull.toMesh());
}

static void boxPointsDemo()
{
  Box3 box(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
  view::Context::get().addDrawable(box);
  gal::PointCloud cloud;
  cloud.reserve(1000);
  box.randomPoints(1000, std::back_inserter(cloud));
  view::Context::get().addDrawable(cloud);
}

static void sphereQueryDemo()
{
  auto                mesh = loadBunnySmall();
  auto                box  = mesh.bounds();
  auto                ball = gal::Sphere {box.min, 0.8f};
  std::vector<size_t> queryFaces;
  mesh.querySphere(ball, std::back_inserter(queryFaces), gal::eMeshElement::face);
  auto querymesh = mesh.extractFaces(queryFaces);
  view::Context::get().addDrawable(querymesh);
  view::Context::get().addDrawable(ball);
};

void stupidImGuiDemo()
{
  auto& panel = view::newPanel("window title");
  panel.newWidget<view::Text>("This is some useful text at the start.");
  auto slider3 = panel.newWidget<view::Slider3>(std::string("Coords"), 0.0f, 1.0f);

  // Add handler to check the handlers are working.
  slider3->addHandler([](const float(&value)[3]) {
    std::cout << "Coords: (" << value[0] << ", " << value[1] << ", " << value[2] << ")\n";
  });

  auto slider = panel.newWidget<view::Slider>(std::string("Slider"), 0.0f, 1.0f);
  slider->addHandler([](const float& value) {
      std::cout << "Slider: " << value << std::endl;
  });

  panel.newWidget<view::Text>("This is some other stupid text at the end.");
};

static void closestPointDemo()
{
  auto mesh = loadBunnySmall();
  Box3 box  = mesh.bounds();
  view::Context::get().addDrawable(box);
  gal::PointCloud cloud, cloud2;
  cloud.reserve(1000);
  box.randomPoints(1000, std::back_inserter(cloud));
  cloud2.reserve(cloud.size());
  for (const auto& pt : cloud) {
    cloud2.push_back(mesh.closestPoint(pt, FLT_MAX));
  }
  view::Context::get().addDrawable(mesh);
  view::Context::get().addDrawable(cloud2);
  //   view::Context::get().addDrawable(cloud);
}

int main(int argc, char** argv)
{
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
  glfwSwapInterval(1);

  std::cout << "...OpenGL bindings...\n";
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize OpenGLL loader!";
    return 1;
  }

  //   meshPlaneClippingDemo();
  //   boxPointsDemo();
  //   convexHullDemo();
  //   sphereQueryDemo();
  closestPointDemo();

  // Init shader.
  view::Context& ctx      = view::Context::get();
  size_t         shaderId = ctx.shaderId("default");
  ctx.useShader(shaderId);

  std::cout << "Setting up mouse event callbacks...\n";
  view::Context::registerCallbacks(window);
  view::Context::get().setPerspective();

  // Setup IMGUI
  view::initializeImGui(window, glslVersion);

  stupidImGuiDemo();  // Demo using my own imgui integration.

  int W, H;
  glfwGetFramebufferSize(window, &W, &H);
  glViewport(0, 0, W, H);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(3.0f);
  // view::Context::get().setWireframeMode(true);
  glLineWidth(1.5f);

  std::cout << "Starting render loop...\n";

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      view::drawAllPanels();
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