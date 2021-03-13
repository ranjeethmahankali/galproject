#include <galview/Context.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sstream>

namespace gal {
namespace view {

static bool       sRightDown = false;
static bool       sShiftDown = false;
static bool       sLeftDown  = false;
static glm::dvec2 sMousePos  = {0.0f, 0.0f};

static glm::mat4 sTrans    = glm::identity<glm::mat4>();
static glm::mat4 sInvTrans = glm::identity<glm::mat4>();

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
  if (shaderId < mShaders.size()) {
    mShaders[shaderId].use();
    mShaderIndex = shaderId;
  }
  else {
    std::cerr << "Invalid shader id\n";
  }
}

Context& Context::get()
{
  // Singleton instance.
  static Context sInstance;
  return sInstance;
};

Context::Context() : mShaders(1)
{
  mShaders.reserve(16);
  mShaders[0].loadFromName("simple");
  useShader(0);

  useCamera(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));

  setWireframeMode(sEdgeMode);
};

void Context::useCamera(const glm::vec3& eye,
                        const glm::vec3& target,
                        const glm::vec3& up)
{
  mView = glm::lookAt(eye, target, up);
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
  glfwSetKeyCallback(window, Context::onKeyEvent);
};

void Context::onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (!sShiftDown)
    return;
  static constexpr float     sRotSpeed = 0.002f;
  static constexpr glm::vec3 sZAxis    = {0.0f, 0.0f, 1.0f};
  static constexpr glm::vec4 sXAxis    = {1.0f, 0.0f, 0.0f, 0.0};
  if (sRightDown) {
    get().mView *=
      sInvTrans *
      glm::rotate(glm::rotate(float((xpos - sMousePos[0]) * sRotSpeed), sZAxis),
                  float(ypos - sMousePos[1]) * sRotSpeed,
                  glm::vec3(glm::inverse(get().mView) * sXAxis)) *
      sTrans;

    get().cameraChanged();
  }

  static constexpr float sTransScale = 500.0f;
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

void Context::onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
    if (action == GLFW_PRESS)
      sShiftDown = true;
    if (action == GLFW_RELEASE)
      sShiftDown = false;
  }
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
void Context::setUniformInternal<bool>(int location, const bool& flag)
{
  GL_CALL(glUniform1i(location, flag));
};

void Context::setWireframeMode(bool flag)
{
  sEdgeMode = flag;
}

void Context::setWireframeUniform(bool flag)
{
  get().setUniform<bool>("edgeMode", flag && sEdgeMode);
}

bool Context::wireframeMode()
{
  return sEdgeMode;
};

void Context::setPerspective(float fovy, float aspect, float near, float far)
{
  get().mProj = glm::perspective(fovy, aspect, near, far);
  cameraChanged();
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
};

static void checkCompilation(uint32_t id, uint32_t type)
{
  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    std::cerr << "Failed to compile ";
    switch (type) {
    case GL_VERTEX_SHADER:
      std::cerr << "vertex";
      break;
    case GL_FRAGMENT_SHADER:
      std::cerr << "fragment";
      break;
    case GL_COMPUTE_SHADER:
      std::cerr << "compute";
      break;
    default:
      break;
    }
    std::cerr << " shader" << std::endl;

    std::string message(length + 1, '\0');
    glGetShaderInfoLog(id, length, &length, message.data());
    std::cerr << message << std::endl;

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
    std::cout << "Error Linking Shader Program:\n" << infoLog << std::endl;
  }
};

static std::string readfile(const std::string& filepath)
{
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::stringstream file_stream;
  try {
    file.open(filepath);
    file_stream << file.rdbuf();
    file.close();
  }
  catch (std::ifstream::failure e) {
    std::cout << "Error reading shader source file!" << std::endl;
  }
  return file_stream.str();
};

void Context::Shader::loadFromSources(const std::string& vsrc,
                                                 const std::string& fsrc)
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

void Context::Shader::loadFromFiles(const std::string& vpath,
                                               const std::string& fpath)
{
  loadFromSources(readfile(vpath), readfile(fpath));
};

void Context::Shader::loadFromName(const std::string& name)
{
  loadFromFiles(name + "_v.glsl", name + "_f.glsl");
  mName = name;
};

void Context::Shader::use() const
{
  GL_CALL(glUseProgram(mId));
}

Context::Shader::~Shader()
{
  GL_CALL(glDeleteProgram(mId));
}

}  // namespace view
}  // namespace gal