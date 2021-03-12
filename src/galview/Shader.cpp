#include <galview/GLUtil.h>
#include <galview/Shader.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sstream>

namespace gal {

namespace view {

static Shader*    sCurrent   = nullptr;
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

void Shader::cameraChanged()
{
  if (!sCurrent)
    return;
  sCurrent->setUniform("mvpMat", sCurrent->mvpMatrix());
}

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

Shader Shader::loadFromSources(const std::string& vsrc, const std::string& fsrc)
{
  return Shader(vsrc, fsrc);
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

Shader Shader::loadFromFiles(const std::string& vpath, const std::string& fpath)
{
  return Shader(readfile(vpath), readfile(fpath));
};

Shader Shader::loadFromName(const std::string& name)
{
  return loadFromFiles(name + "_v.glsl", name + "_f.glsl");
};

Shader::Shader(const std::string& vertSrc, const std::string& fragSrc)
{
  // Compile vertex shader.
  mVertId          = glCreateShader(GL_VERTEX_SHADER);
  const char* cstr = vertSrc.c_str();
  GL_CALL(glShaderSource(mVertId, 1, &cstr, nullptr));
  GL_CALL(glCompileShader(mVertId));
  checkCompilation(mVertId, GL_VERTEX_SHADER);

  // Compile fragment shader.
  mFragId = glCreateShader(GL_FRAGMENT_SHADER);
  cstr    = fragSrc.c_str();
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

  useCamera(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
  cameraChanged();
};

void Shader::use() const
{
  GL_CALL(glUseProgram(mId));
  sCurrent = const_cast<Shader*>(this);
  setWireframeMode(sEdgeMode);
}

void Shader::useCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
{
  mView = glm::lookAt(eye, target, up);
  cameraChanged();
};

glm::mat4 Shader::mvpMatrix() const
{
  return mProj * mView;
}

Shader::~Shader()
{
  GL_CALL(glDeleteProgram(mId));
}

void Shader::registerCallbacks(GLFWwindow* window)
{
  glfwSetCursorPosCallback(window, Shader::onMouseMove);
  glfwSetMouseButtonCallback(window, Shader::onMouseButton);
  glfwSetScrollCallback(window, Shader::onMouseScroll);
  glfwSetKeyCallback(window, Shader::onKeyEvent);
};

void Shader::onMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (!sCurrent)
    return;
  if (!sShiftDown)
    return;
  static constexpr float     sRotSpeed = 0.002f;
  static constexpr glm::vec3 sZAxis    = {0.0f, 0.0f, 1.0f};
  static constexpr glm::vec4 sXAxis    = {1.0f, 0.0f, 0.0f, 0.0};
  if (sRightDown) {
    sCurrent->mView *=
      sInvTrans *
      glm::rotate(glm::rotate(float((xpos - sMousePos[0]) * sRotSpeed), sZAxis),
                  float(ypos - sMousePos[1]) * sRotSpeed,
                  glm::vec3(glm::inverse(sCurrent->mView) * sXAxis)) *
      sTrans;

    cameraChanged();
  }

  static constexpr float sTransScale = 500.0f;
  if (sLeftDown) {
    auto trans = glm::translate(glm::vec3(
      glm::inverse(sCurrent->mView) * glm::vec4 {float(xpos - sMousePos[0]) / sTransScale,
                                                 float(sMousePos[1] - ypos) / sTransScale,
                                                 0.0f,
                                                 0.0f}));
    sTrans *= trans;
    sInvTrans = glm::inverse(sTrans);
    sCurrent->mView *= trans;
    cameraChanged();
  }
  captureMousePos(xpos, ypos);
}

void Shader::onMouseButton(GLFWwindow* window, int button, int action, int mods)
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

void Shader::onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
    if (action == GLFW_PRESS)
      sShiftDown = true;
    if (action == GLFW_RELEASE)
      sShiftDown = false;
  }
}

void Shader::onMouseScroll(GLFWwindow* window, double xOffset, double yOffset)
{
  if (!sCurrent)
    return;

  static constexpr float zoomUp   = 1.2f;
  static constexpr float zoomDown = 1.0f / zoomUp;

  static constexpr glm::vec3 zUp = {zoomUp, zoomUp, zoomUp};
  static constexpr glm::vec3 zDn = {zoomDown, zoomDown, zoomDown};

  if (yOffset > 0.0)
    sCurrent->mView *= sInvTrans * glm::scale(zUp) * sTrans;
  else
    sCurrent->mView *= sInvTrans * glm::scale(zDn) * sTrans;

  cameraChanged();
}

template<>
void Shader::setUniformInternal<float>(int location, const float& val)
{
  GL_CALL(glUniform1f(location, val));
};

template<>
void Shader::setUniformInternal<glm::mat4>(int location, const glm::mat4& mat)
{
  GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]));
}

template<>
void Shader::setUniformInternal<bool>(int location, const bool& flag)
{
  GL_CALL(glUniform1i(location, flag));
}

void Shader::setPerspective(float fovy, float aspect, float near, float far)
{
  sCurrent->mProj = glm::perspective(fovy, aspect, near, far);
  cameraChanged();
};

void Shader::setOrthographic(float left,
                             float right,
                             float top,
                             float bottom,
                             float near,
                             float far)
{
  sCurrent->mProj = glm::ortho(left, right, bottom, top, near, far);
  cameraChanged();
}

void Shader::setWireframeMode(bool flag)
{
  sEdgeMode = flag;
}

void Shader::setWireframeUniform(bool flag) {
  if (!sCurrent)
    return;
  sCurrent->setUniform<bool>("edgeMode", flag && sEdgeMode);
}

bool Shader::wireframeMode() {
  return sEdgeMode;
}

}  // namespace view

}  // namespace gal