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

using namespace gal;

static void createBoxMeshDemo()
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

  auto mesh =
    Mesh(sCoords.data(), sCoords.size() / 3, sIndices.data(), sIndices.size() / 3);

  view::Context::get().addDrawable(mesh);
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
  using namespace std::string_literals;
  auto& panel       = view::newPanel("Mesh-Plane-Clipping"s);
  auto  ptSlider    = panel.newWidget<view::SliderF3>("Point"s, 0.0f, 1.0f);
  float initNorm[3] = {0.0f, 0.0f, 1.0f};
  auto  normSlider  = panel.newWidget<view::SliderF3>("Normal"s, 0.0f, 1.0f, initNorm);

  static auto   plane   = gal::Plane {{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}};
  static size_t meshId  = 0;
  static size_t planeId = 0;

  static auto meshUpdater = []() {
    auto mesh = loadBunnySmall();
    mesh.clipWithPlane(plane);
    meshId  = view::Context::get().replaceDrawable(meshId, mesh);
    planeId = view::Context::get().replaceDrawable(planeId, plane);
  };

  auto ptUpdater = [](const float(&coords)[3]) {
    plane.setOrigin({coords[0], coords[1], coords[2]});
    meshUpdater();
  };

  auto normUpdater = [](const float(&coords)[3]) {
    plane.setNormal({coords[0], coords[1], coords[2]});
    meshUpdater();
  };

  meshUpdater();  // First time.
  ptSlider->addHandler(ptUpdater);
  normSlider->addHandler(normUpdater);
};

static void convexHullDemo()
{
  using namespace std::string_literals;
  auto& panel  = view::newPanel("Convex Hull Demo"s);
  auto  slider = panel.newWidget<view::SliderI>("Number of Points"s, 10, 1000, 10);

  static const Box3 box(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
  static std::vector<glm::vec3> points;
  static size_t                 id = 0;

  auto updater = [](const int& n) {
    points.clear();
    points.reserve(n);
    box.randomPoints(size_t(n), std::back_inserter(points));
    ConvexHull hull(points.begin(), points.end());
    id = view::Context::get().replaceDrawable(id, hull.toMesh());
  };

  // For the first time.
  updater(10);

  slider->addHandler(updater);
}

static void boxPointsDemo()
{
  using namespace std::string_literals;
  auto& panel  = view::newPanel("Random Points Demo"s);
  auto  slider = panel.newWidget<view::SliderI>("Number of Points"s, 10, 1000, 10);

  static Box3 box(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
  view::Context::get().addDrawable(box);
  static gal::PointCloud cloud;
  static size_t          id = 0;

  auto numUpdater = [](const int& n) {
    cloud.clear();
    cloud.reserve(n);
    box.randomPoints(size_t(n), std::back_inserter(cloud));
    id = view::Context::get().replaceDrawable(id, cloud);
  };

  // For the first time;
  numUpdater(1000);
  slider->addHandler(numUpdater);
}

static void sphereQueryDemo()
{
  using namespace std::string_literals;
  static auto   mesh   = loadBunnySmall();
  static auto   box    = mesh.bounds();
  static auto   ball   = gal::Sphere {{0.0f, 1.0f, 0.0f}, 0.8f};
  static size_t idMesh = 0;
  static size_t idBall = 0;

  auto& panel        = view::newPanel("Sphere Query");
  auto  radiusSlider = panel.newWidget<view::SliderF>("Radius"s, 0.0f, 1.0f, 0.5f);
  auto  centerSlider = panel.newWidget<view::SliderF3>("Center"s, 0.0f, 1.0f);

  static auto meshUpdater = []() {
    std::vector<size_t> queryFaces;
    mesh.querySphere(ball, std::back_inserter(queryFaces), gal::eMeshElement::face);
    auto querymesh = mesh.extractFaces(queryFaces);
    idMesh         = view::Context::get().replaceDrawable(idMesh, querymesh);
    idBall         = view::Context::get().replaceDrawable(idBall, ball);
  };

  auto radiusUpdater = [](const float& radius) {
    ball.radius = radius;
    meshUpdater();
  };

  auto centerUpdater = [](const float(&coords)[3]) {
    ball.center = {coords[0], coords[1], coords[2]};
    meshUpdater();
  };

  // First time.
  ball.radius = 0.5f;
  meshUpdater();

  radiusSlider->addHandler(radiusUpdater);
  centerSlider->addHandler(centerUpdater);
};

static void stupidImGuiDemo()
{
  auto& panel = view::newPanel("window title");
  panel.newWidget<view::Text>("This is some useful text at the start.");
  auto slider3 = panel.newWidget<view::SliderF3>(std::string("Coords"), 0.0f, 1.0f);

  // Add handler to check the handlers are working.
  slider3->addHandler([](const float(&value)[3]) {
    std::cout << "Coords: (" << value[0] << ", " << value[1] << ", " << value[2] << ")\n";
  });

  auto slider = panel.newWidget<view::SliderF>(std::string("Slider"), 0.0f, 1.0f);
  slider->addHandler(
    [](const float& value) { std::cout << "Slider: " << value << std::endl; });

  panel.newWidget<view::Text>("This is some other stupid text at the end.");
};

static void closestPointDemo()
{
  using namespace std::string_literals;
  auto& panel  = view::newPanel("window title");
  auto  slider = panel.newWidget<view::SliderI>("Number of Points"s, 10, 5000, 1000);

  static auto   mesh   = loadBunnySmall();
  static Box3   box    = mesh.bounds();
  static size_t ptsId  = 0;
  static size_t meshId = 0;
  view::Context::get().addDrawable(box);
  view::Context::get().addDrawable(mesh);

  auto sliderUpdater = [](const int& n) {
    gal::PointCloud cloud, cloud2;
    cloud.reserve(n);
    box.randomPoints(n, std::back_inserter(cloud));
    cloud2.reserve(cloud.size());
    for (const auto& pt : cloud) {
      cloud2.push_back(mesh.closestPoint(pt, FLT_MAX));
    }
    ptsId = view::Context::get().replaceDrawable(ptsId, cloud2);
  };

  // First time.
  sliderUpdater(1000);

  slider->addHandler(sliderUpdater);
}

static void circumCircleDemo()
{
  view::Context::get().set2dMode(true);
  //   view::Context::get().addDrawable(gal::Circle2d(glm::vec3 {0.f, 0.f, 0.f}, 0.5f));
  //   view::Context::get().addDrawable(gal::Circle2d(glm::vec3 {0.5f, 0.f, 0.f}, 0.5f));
  glm::vec2 verts[3] = {
    glm::vec2 {0.0f, 0.2f}, glm::vec2 {1.0f, 0.3f}, glm::vec2 {0.4f, 1.f}};

  gal::PointCloud cloud;
  cloud.reserve(3);
  for (const auto& pt : verts) {
    cloud.emplace_back(pt.x, pt.y, 0.f);
  }
  view::Context::get().addDrawable(cloud);
  view::Context::get().addDrawable(
    gal::Circle2d::createCircumcircle(verts[0], verts[1], verts[2]));
};

static void boundingCircleDemo()
{
  view::Context::get().set2dMode(true);

  static size_t nPts = 20;
  static Box2   b(glm::vec2 {-1.f, -1.f}, glm::vec2 {1.f, 1.f});

  std::vector<glm::vec2> pts;
  pts.reserve(nPts);
  b.randomPoints(nPts, std::back_inserter(pts));

  view::Context::get().addDrawable(gal::PointCloud(pts));
  view::Context::get().addDrawable(
    gal::Circle2d::minBoundingCircle(pts.data(), pts.size()));
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

  view::Context::get().setWireframeMode(true);

  //   meshPlaneClippingDemo();
  //   boxPointsDemo();
  //   convexHullDemo();
  //   sphereQueryDemo();
  //   createBoxMeshDemo();
  // closestPointDemo();
  //   circumCircleDemo();
  boundingCircleDemo();
  //   stupidImGuiDemo();  // Demo using my own imgui integration.

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
    }

    view::imGuiRender();

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