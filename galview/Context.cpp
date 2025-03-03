#include <Context.h>

#include <GLUtil.h>
#include <Mesh.h>
#include <Util.h>
#include <Views.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sstream>

namespace gal {
namespace view {

static bool sOrthoMode = false;  // NOLINT

static void setOrthoModeUniform()
{
  Context::get().setUniform("orthoMode", sOrthoMode);
}

void RenderSettings::apply() const
{
  Context& ctx = Context::get();
  ctx.useShader(shaderId);
  ctx.setUniform("faceColor", faceColor);
  ctx.setUniform("edgeColor", edgeColor);
  ctx.setUniform("pointColor", pointColor);
  ctx.setUniform("shadingFactor", shadingFactor);
  ctx.setUniform("edgeMode", edgeMode);
  ctx.setUniform("pointMode", pointMode);

  GL_CALL(glPolygonMode(polygonMode.first, polygonMode.second));
};

static bool       sRightDown  = false;                       // NOLINT
static bool       sLeftDown   = false;                       // NOLINT
static glm::dvec2 sMousePos   = {0.0f, 0.0f};                // NOLINT
static float      sTransScale = 500.f;                       // NOLINT
static glm::mat4  sTrans      = glm::identity<glm::mat4>();  // NOLINT
static glm::mat4  sInvTrans   = glm::identity<glm::mat4>();  // NOLINT

static bool s2dMode        = false;  // NOLINT
static bool sWireFrameMode = false;  // NOLINT
static bool sMeshEdgeMode  = false;  // NOLINT

static void captureMousePos(double x, double y)
{
  sMousePos.x = x;
  sMousePos.y = y;
};

void Context::cameraChanged()
{
  setUniform("mvpMat", mvpMatrix());
};

void Context::zoomExtents()
{
  auto bounds = Views::visibleBounds();
  if (!bounds.valid())
    return;
  glm::vec3 target = bounds.center();
  glm::vec3 diag   = bounds.diagonal();
  if (s2dMode) {
    useCamera(target + glm::vec3 {0.f, 0.f, 2.0f}, target, {0.0f, 1.0f, 0.0f});
  }
  else {
    useCamera(target + diag, target, vec3_zunit);
  }
  sTrans    = glm::translate(-target);
  sInvTrans = glm::inverse(sTrans);
}

size_t Context::shaderId(const std::string& name) const
{
  size_t i = 0;
  for (const auto& shader : mShaders) {
    if (shader.mName == name) {
      return i;
    }
    i++;
  }
  return SIZE_MAX;
};

void Context::useShader(size_t shaderId)
{
  if (shaderId == mShaderIndex) {
    return;  // Already using the shader.
  }
  else if (shaderId < mShaders.size()) {
    mShaders[shaderId].use();
    mShaderIndex = shaderId;
    // Set all the uniforms again.
    cameraChanged();
    setOrthoModeUniform();
  }
  else {
    glutil::logger().error("{} is an invalid shader id.", shaderId);
  }
}

Context& Context::get()
{
  // Singleton instance.
  static Context sInstance;
  return sInstance;
};

void Context::set2dMode(bool flag)
{
  s2dMode = flag;
  if (s2dMode) {
    useCamera({0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    setProjectionMode(Projection::PARALLEL);
  }
  else {
    useCamera({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    setProjectionMode(Projection::PERSPECTIVE);
  }
}

Context::Context()
    : mShaders(4)
    , mProj {}
    , mView {}
    , mWindowSize {}
{
  mShaders[0].loadFromName("default");
  mShaders[1].loadFromName("mesh");
  mShaders[2].loadFromName("text");
  mShaders[3].loadFromName("glyph");
};

void Context::useCamera(const glm::vec3& eye,
                        const glm::vec3& target,
                        const glm::vec3& up)
{
  mView       = glm::lookAt(eye, target, up);
  sTransScale = 700.f / glm::distance(eye, target);
  cameraChanged();
};

glm::mat4 Context::mvpMatrix() const
{
  return mProj * mView;
};

void Context::registerCallbacks(GLFWwindow* window)
{
  glfwSetCursorPosCallback(window, Context::onMouseMove);
  glfwSetMouseButtonCallback(window, Context::onMouseButton);
  glfwSetScrollCallback(window, Context::onMouseScroll);
  glfwSetFramebufferSizeCallback(window, Context::onWindowResize);
};

void Context::onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (ImGui::GetIO().WantCaptureMouse) {
    // Mouse is interacting with ImGui elements, so the camera should not react.
    return;
  }
  static constexpr float     sRotSpeed = 0.002f;
  static constexpr glm::vec3 sZAxis    = {0.0f, 0.0f, 1.0f};
  static constexpr glm::vec4 sXAxis    = {1.0f, 0.0f, 0.0f, 0.0};
  if (sRightDown) {
    if (s2dMode) {
      return;  // cannot orbit in 2d mode.
    }
    get().mView *=
      sInvTrans *
      glm::rotate(glm::rotate(float((xpos - sMousePos[0]) * sRotSpeed), sZAxis),
                  float(ypos - sMousePos[1]) * sRotSpeed,
                  glm::vec3(glm::inverse(get().mView) * sXAxis)) *
      sTrans;
    get().cameraChanged();
  }
  if (sLeftDown) {
    auto trans = glm::translate(glm::vec3(
      glm::inverse(get().mView) * glm::vec4 {float(xpos - sMousePos[0]) / sTransScale,
                                             float(sMousePos[1] - ypos) / sTransScale,
                                             0.0f,
                                             0.0f}));
    sTrans *= trans;
    sInvTrans = glm::inverse(sTrans);
    get().mView *= trans;
    get().cameraChanged();
  }
  captureMousePos(xpos, ypos);
}

void Context::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    if (action == GLFW_PRESS)
      sRightDown = true;
    if (action == GLFW_RELEASE)
      sRightDown = false;
  }
  else if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS)
      sLeftDown = true;
    if (action == GLFW_RELEASE)
      sLeftDown = false;
  }
  GL_CALL(glfwGetCursorPos(window, &sMousePos.x, &sMousePos.y));
}

void Context::onMouseScroll(GLFWwindow* window, double xOffset, double yOffset)
{
  if (ImGui::GetIO().WantCaptureMouse) {
    // Mouse is interacting with ImGui elements, so the camera should not react.
    return;
  }
  static constexpr float     zoomUp   = 1.2f;
  static constexpr float     zoomDown = 1.0f / zoomUp;
  static constexpr glm::vec3 zUp      = {zoomUp, zoomUp, zoomUp};
  static constexpr glm::vec3 zDn      = {zoomDown, zoomDown, zoomDown};
  // Update the matrices.
  if (yOffset > 0.0)
    get().mView *= sInvTrans * glm::scale(zUp) * sTrans;
  else
    get().mView *= sInvTrans * glm::scale(zDn) * sTrans;
  // Push changes to the shader.
  get().cameraChanged();
}

void Context::onWindowResize(GLFWwindow* window, int w, int h)
{
  GL_CALL(glViewport(0, 0, w, h));
  get().mWindowSize = {w, h};
  get().setProjectionMode(get().mProjectionMode);
}

static GLint uniformLocation(const std::string& name)
{
  GLint prog = 0;
  GL_CALL(glGetIntegerv(GL_CURRENT_PROGRAM, &prog));
  GLint loc = -1;
  GL_CALL(loc = glGetUniformLocation(prog, name.c_str()));
  return loc;
}

void Context::setUniform(const std::string& name, float val)
{
  GLint location = uniformLocation(name);
  if (location != -1) {
    GL_CALL(glUniform1f(location, val));
  }
}

void Context::setUniform(const std::string& name, const glm::mat4& mat)
{
  GLint location = uniformLocation(name);
  if (location != -1) {
    GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
  }
}

void Context::setUniform(const std::string& name, const glm::vec4& v)
{
  GLint location = uniformLocation(name);
  if (location != -1) {
    GL_CALL(glUniform4fv(location, 1, &v[0]));
  }
}

void Context::setUniform(const std::string& name, const glm::vec3& v)
{
  GLint location = uniformLocation(name);
  if (location != -1) {
    GL_CALL(glUniform3fv(location, 1, &v[0]));
  }
}

void Context::setUniform(const std::string& name, bool flag)
{
  GLint location = uniformLocation(name);
  if (location != -1) {
    GL_CALL(glUniform1i(location, flag));
  }
}

void Context::setWireframeMode(bool flag)
{
  sWireFrameMode = flag;
}

void Context::setMeshEdgeMode(bool flag)
{
  sMeshEdgeMode = flag;
}

bool Context::wireframeMode()
{
  return sWireFrameMode;
};

bool Context::meshEdgeMode()
{
  return sMeshEdgeMode;
}

void Context::init(GLFWwindow* window)
{
  glfwGetWindowSize(window, &mWindowSize[0], &mWindowSize[1]);
  size_t id = shaderId("default");
  useShader(id);
  useCamera(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
  setWireframeMode(sWireFrameMode);
  setProjectionMode(mProjectionMode);
}

void Context::setProjectionMode(Projection mode)
{
  switch (mode) {
  case Projection::PARALLEL: {
    static constexpr float LEFT   = -2.0f;
    static constexpr float RIGHT  = 2.0f;
    static constexpr float TOP    = 1.1f;
    static constexpr float BOTTOM = -1.1f;
    // near and far are defined as macros on Windows.
    static constexpr float NEAR_S = -5.f;
    static constexpr float FAR_S  = 100.0f;
    get().mProj                   = glm::ortho(LEFT, RIGHT, BOTTOM, TOP, NEAR_S, FAR_S);
    cameraChanged();
    sOrthoMode = true;
    setOrthoModeUniform();
    break;
  }
  case Projection::PERSPECTIVE: {
    static constexpr float FOVY = 0.9f;
    // near and far as defined as macros in windows.
    static constexpr float NEAR_S = 0.01f;
    static constexpr float FAR_S  = 100.0f;
    float                  ASPECT = float(mWindowSize.x) / float(mWindowSize.y);
    // glutil::logger().warn("Setting perspective mode with aspect ratio: {}", aspect);
    get().mProj = glm::perspective(FOVY, ASPECT, NEAR_S, FAR_S);
    cameraChanged();
    sOrthoMode = false;
    setOrthoModeUniform();
    break;
  }
  default:
    break;
  }
  mProjectionMode = mode;
};

static void checkCompilation(uint32_t id, uint32_t type)
{
  int result = 0;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    std::string shaderType = "unknown";
    switch (type) {
    case GL_VERTEX_SHADER:
      shaderType = "vertex";
      break;
    case GL_FRAGMENT_SHADER:
      shaderType = "fragment";
      break;
    case GL_COMPUTE_SHADER:
      shaderType = "compute";
      break;
    default:
      break;
    }

    std::string message(length + 1, '\0');
    glGetShaderInfoLog(id, length, &length, message.data());
    glutil::logger().error("Failed to compile {} shader:\n{}", shaderType, message);

    GL_CALL(glDeleteShader(id));
  }
};

static void checkLinking(uint32_t progId)
{
  int success = 0;
  glGetProgramiv(progId, GL_LINK_STATUS, &success);
  if (!success) {
    std::array<char, 1024> infolog {};
    glGetProgramInfoLog(progId, 1024, NULL, infolog.data());
    glutil::logger().error("Error linking shader program:\n{}", infolog.data());
  }
};

static std::string readfile(const std::string& filepath)
{
  glutil::logger().debug("Reading shader source: {}", filepath);
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::stringstream file_stream;
  try {
    file.open(filepath);
    file_stream << file.rdbuf();
    file.close();
  }
  catch (const std::ifstream::failure& e) {
    glutil::logger().error(
      "Error reading shader source file:\n{}\n{}", filepath, e.what());
  }
  return file_stream.str();
};

void Context::Shader::loadFromSources(const std::string& vsrc, const std::string& fsrc)
{
  // Compile vertex shader.
  uint32_t    vertId = glCreateShader(GL_VERTEX_SHADER);
  const char* cstr   = vsrc.c_str();
  GL_CALL(glShaderSource(vertId, 1, &cstr, nullptr));
  GL_CALL(glCompileShader(vertId));
  checkCompilation(vertId, GL_VERTEX_SHADER);
  // glutil::logger().info("Compiled vertex shader: {}", vertId);

  // Compile fragment shader.
  uint32_t fragId = glCreateShader(GL_FRAGMENT_SHADER);
  cstr            = fsrc.c_str();
  GL_CALL(glShaderSource(fragId, 1, &cstr, nullptr));
  GL_CALL(glCompileShader(fragId));
  checkCompilation(fragId, GL_FRAGMENT_SHADER);
  // glutil::logger().info("Compiled vertex shader: {}", fragId);

  // Link
  mId = glCreateProgram();
  GL_CALL(glAttachShader(mId, vertId));
  GL_CALL(glAttachShader(mId, fragId));
  GL_CALL(glLinkProgram(mId));
  checkLinking(mId);
  GL_CALL(glDeleteShader(vertId));
  GL_CALL(glDeleteShader(fragId));
  // glutil::logger().info("Linked program: {}", mId);
};

void Context::Shader::loadFromFiles(const std::string& vpath, const std::string& fpath)
{
  loadFromSources(readfile(vpath), readfile(fpath));
};

void Context::Shader::loadFromName(const std::string& name)
{
  loadFromFiles(utils::absPath(std::filesystem::path(name + "_v.glsl")).string(),
                utils::absPath(std::filesystem::path(name + "_f.glsl")).string());
  mName = name;
};

void Context::Shader::use() const
{
  GL_CALL(glUseProgram(mId));
}

Context::Shader::~Shader()
{
  GL_CALL(glDeleteProgram(mId));
};
}  // namespace view
}  // namespace gal
