#include <fstream>
#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <galcore/Mesh.h>
#include <galcore/Util.h>
#include <galview/Command.h>
#include <galview/Context.h>
#include <galview/Views.h>

namespace gal {
namespace view {

static bool sOrthoMode = false;

static void setOrthoModeUniform()
{
  Context::get().setUniform<bool>("orthoMode", sOrthoMode);
}

void RenderSettings::apply() const
{
  Context& ctx = Context::get();
  ctx.useShader(shaderId);
  ctx.setUniform<glm::vec4>("faceColor", faceColor);
  ctx.setUniform<glm::vec4>("edgeColor", edgeColor);
  ctx.setUniform<glm::vec4>("pointColor", pointColor);
  ctx.setUniform<float>("shadingFactor", shadingFactor);
  ctx.setUniform<bool>("edgeMode", edgeMode);
  ctx.setUniform<bool>("pointMode", pointMode);

  GL_CALL(glPolygonMode(polygonMode.first, polygonMode.second));
};

static bool       sRightDown  = false;
static bool       sLeftDown   = false;
static glm::dvec2 sMousePos   = {0.0f, 0.0f};
static float      sTransScale = 500.f;
static glm::mat4  sTrans      = glm::identity<glm::mat4>();
static glm::mat4  sInvTrans   = glm::identity<glm::mat4>();

static bool s2dMode   = false;
static bool sEdgeMode = false;

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
    view::logger().error("{} is an invalid shader id.", shaderId);
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
    setOrthographic();
  }
  else {
    useCamera({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    setPerspective();
  }
}

Context::Context()
    : mShaders(4)
{
  mShaders[0].loadFromName("default");
  mShaders[1].loadFromName("mesh");
  mShaders[2].loadFromName("text");
  mShaders[3].loadFromName("glyph");

  useCamera(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));

  setWireframeMode(sEdgeMode);
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
};

void Context::onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (ImGui::GetIO().WantCaptureMouse)
    return;
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
  static constexpr float zoomUp   = 1.2f;
  static constexpr float zoomDown = 1.0f / zoomUp;

  static constexpr glm::vec3 zUp = {zoomUp, zoomUp, zoomUp};
  static constexpr glm::vec3 zDn = {zoomDown, zoomDown, zoomDown};

  if (yOffset > 0.0)
    get().mView *= sInvTrans * glm::scale(zUp) * sTrans;
  else
    get().mView *= sInvTrans * glm::scale(zDn) * sTrans;

  get().cameraChanged();
}

template<>
void Context::setUniformInternal<float>(int location, const float& val)
{
  GL_CALL(glUniform1f(location, val));
};

template<>
void Context::setUniformInternal<glm::mat4>(int location, const glm::mat4& mat)
{
  GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
};

template<>
void Context::setUniformInternal<glm::vec4>(int location, const glm::vec4& v)
{
  GL_CALL(glUniform4fv(location, 1, &v[0]));
};

template<>
void Context::setUniformInternal<glm::vec3>(int location, const glm::vec3& v)
{
  GL_CALL(glUniform3fv(location, 1, &v[0]));
};

template<>
void Context::setUniformInternal<bool>(int location, const bool& flag)
{
  GL_CALL(glUniform1i(location, flag));
};

void Context::setWireframeMode(bool flag)
{
  sEdgeMode = flag;
}

bool Context::wireframeMode()
{
  return sEdgeMode;
};

void Context::setPerspective(float fovy, float aspect, float near, float far)
{
  get().mProj = glm::perspective(fovy, aspect, near, far);
  cameraChanged();
  sOrthoMode = false;
  setOrthoModeUniform();
};

void Context::setOrthographic(float left,
                              float right,
                              float top,
                              float bottom,
                              float near,
                              float far)
{
  get().mProj = glm::ortho(left, right, bottom, top, near, far);
  cameraChanged();
  sOrthoMode = true;
  setOrthoModeUniform();
};

static void checkCompilation(uint32_t id, uint32_t type)
{
  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int length;
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
    view::logger().error("Failed to compile {} shader:\n{}", shaderType, message);

    GL_CALL(glDeleteShader(id));
  }
};

static void checkLinking(uint32_t progId)
{
  int  success;
  char infoLog[1024];
  glGetProgramiv(progId, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(progId, 1024, NULL, infoLog);
    view::logger().error("Error linking shader program:\n{}", infoLog);
  }
};

static std::string readfile(const std::string& filepath)
{
  view::logger().debug("Reading shader source: {}", filepath);
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::stringstream file_stream;
  try {
    file.open(filepath);
    file_stream << file.rdbuf();
    file.close();
  }
  catch (std::ifstream::failure e) {
    view::logger().error("Error reading shader source file:\n{}", e.what());
  }
  return file_stream.str();
};

void Context::Shader::loadFromSources(const std::string& vsrc, const std::string& fsrc)
{
  // Compile vertex shader.
  mVertId          = glCreateShader(GL_VERTEX_SHADER);
  const char* cstr = vsrc.c_str();
  GL_CALL(glShaderSource(mVertId, 1, &cstr, nullptr));
  GL_CALL(glCompileShader(mVertId));
  checkCompilation(mVertId, GL_VERTEX_SHADER);

  // Compile fragment shader.
  mFragId = glCreateShader(GL_FRAGMENT_SHADER);
  cstr    = fsrc.c_str();
  GL_CALL(glShaderSource(mFragId, 1, &cstr, nullptr));
  GL_CALL(glCompileShader(mFragId));
  checkCompilation(mFragId, GL_FRAGMENT_SHADER);

  // Link
  mId = glCreateProgram();
  GL_CALL(glAttachShader(mId, mVertId));
  GL_CALL(glAttachShader(mId, mFragId));
  GL_CALL(glLinkProgram(mId));
  checkLinking(mId);
  GL_CALL(glDeleteShader(mVertId));
  GL_CALL(glDeleteShader(mFragId));
};

void Context::Shader::loadFromFiles(const std::string& vpath, const std::string& fpath)
{
  loadFromSources(readfile(vpath), readfile(fpath));
};

void Context::Shader::loadFromName(const std::string& name)
{
  loadFromFiles(utils::absPath(name + "_v.glsl"), utils::absPath(name + "_f.glsl"));
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
