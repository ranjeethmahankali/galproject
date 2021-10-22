#pragma once

#include <galcore/Box.h>
#include <galview/GLUtil.h>
#include <stdint.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace gal {
namespace view {

struct RenderSettings
{
  glm::vec4                     faceColor     = {1.f, 1.f, 1.f, 1.f};
  glm::vec4                     edgeColor     = {1.f, 1.f, 1.f, 1.f};
  glm::vec4                     pointColor    = {1.f, 0.f, 0.f, 1.f};
  float                         shadingFactor = 1.0f;
  bool                          edgeMode      = false;
  bool                          pointMode     = false;
  std::pair<uint32_t, uint32_t> polygonMode   = {uint32_t(GL_FRONT_AND_BACK),
                                               uint32_t(GL_FILL)};
  size_t                        shaderId      = 0;  // default shader

  void apply() const;
};

template<typename T>
struct Drawable : public std::false_type
{
};

class Context
{
  class Shader
  {
  public:
    void loadFromName(const std::string& name);
    void loadFromSources(const std::string& vertSrc, const std::string& fragSrc);
    void loadFromFiles(const std::string& vertFilePath, const std::string& fragFilePath);

    Shader() = default;

    void use() const;

    ~Shader();

    std::string mName;
    uint32_t    mVertId;
    uint32_t    mFragId;
    uint32_t    mId = 0;
  };

public:
  static Context& get();

  static void registerCallbacks(GLFWwindow* window);

  void setWireframeMode(bool flag);
  bool wireframeMode();

  template<typename T>
  void setUniform(const std::string& name, const T& val)
  {
    int loc = glGetUniformLocation(mShaders[mShaderIndex].mId, name.c_str());
    if (loc == -1) {
      //   std::cerr << "Uniform " << name << " not found.\n";
      return;
    }
    setUniformInternal<T>(loc, val);
  };

  void useCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

  glm::mat4 mvpMatrix() const;

  void setPerspective(float fovy   = 0.9f,
                      float aspect = 1.8f,
                      float near   = 0.01f,
                      float far    = 100.0f);

  void setOrthographic(float left   = -2.0f,
                       float right  = 2.0f,
                       float top    = 1.1f,
                       float bottom = -1.1f,
                       float near   = -5.f,
                       float far    = 100.0f);

  void set2dMode(bool flag);

  void toggle2dMode();

  size_t shaderId(const std::string& name) const;

  void useShader(size_t shaderId);

private:
  Context();

  std::vector<Shader> mShaders;

  glm::mat4 mProj;
  glm::mat4 mView;

  size_t mShaderIndex = SIZE_MAX;

  static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
  static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
  static void onMouseScroll(GLFWwindow* window, double xOffset, double yOffset);
  static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

  void cameraChanged();

  void zoomExtents();

  template<typename T>
  void setUniformInternal(int location, const T& val);
};

}  // namespace view
}  // namespace gal
