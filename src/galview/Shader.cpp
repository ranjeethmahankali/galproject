#include <galview/GLUtil.h>
#include <galview/Shader.h>
#include <fstream>
#include <glm/glm.hpp>
#include <sstream>

namespace gal {

namespace view {

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
};

void Shader::use() const
{
  GL_CALL(glUseProgram(mId));
}

void Shader::useCamera(const Camera& cam)
{
  setUniform("mView", cam.viewMatrix());
  setUniform("mProj", cam.projMatrix());
}

Shader::~Shader()
{
  GL_CALL(glDeleteProgram(mId));
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

}  // namespace view

}  // namespace gal