#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

namespace gal {
namespace view {

class Shader
{
public:
  static Shader loadFromSources(const std::string& vertSrc, const std::string& fragSrc);
  static Shader loadFromFiles(const std::string& vertFilePath,
                              const std::string& fragFilePath);
  static Shader loadFromName(const std::string& name);

  void use() const;
  void useCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

  glm::mat4 mvpMatrix() const;

  ~Shader();

  void setPerspective(float fovy   = 0.9f,
                      float aspect = 1.8f,
                      float near   = 0.01f,
                      float far    = 100.0f);

  void setOrthographic(float left   = -2.0f,
                       float right  = 2.0f,
                       float top    = 1.1f,
                       float bottom = -1.1f,
                       float near   = 0.01f,
                       float far    = 100.0f);

  static void registerCallbacks(GLFWwindow* window);

private:
  glm::mat4 mProj;
  glm::mat4 mView;
  uint32_t  mVertId;
  uint32_t  mFragId;
  uint32_t  mId;

  Shader(const std::string& vertSrc, const std::string& fragSrc);

  template<typename T>
  void setUniformInternal(int location, const T& val);

  static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
  static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
  static void onMouseScroll(GLFWwindow* window, double xOffset, double yOffset);

  static void cameraChanged();
  static void updateViewMatrix();

public:
  template<typename T>
  void setUniform(const std::string& name, const T& val)
  {
    int loc = glGetUniformLocation(mId, name.c_str());
    if (loc == -1) {
      std::cerr << "Uniform " << name << " not found.\n";
      return;
    }
    setUniformInternal<T>(loc, val);
  };
};

}  // namespace view
}  // namespace gal