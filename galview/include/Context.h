#pragma once

#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <Box.h>
#include <GLUtil.h>

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
  enum class Projection
  {
    PERSPECTIVE,
    PARALLEL,
  };

  static Context& get();

  static void registerCallbacks(GLFWwindow* window);

  void setWireframeMode(bool flag);
  void setMeshEdgeMode(bool flag);
  bool wireframeMode();
  bool meshEdgeMode();
  void init(GLFWwindow* window);

  template<typename T>
  void setUniform(const std::string& name, const T& val)
  {
    int loc = glGetUniformLocation(mShaders[mShaderIndex].mId, name.c_str());
    if (loc == -1) {
      // glutil::logger().error("OpenGL uniform '{}' not found.", name);
      return;
    }
    setUniformInternal<T>(loc, val);
  };

  void      useCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
  glm::mat4 mvpMatrix() const;
  void      setProjectionMode(Projection mode);
  void      set2dMode(bool flag);
  size_t    shaderId(const std::string& name) const;
  void      useShader(size_t shaderId);
  void      zoomExtents();

private:
  Context();

  static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
  static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
  static void onMouseScroll(GLFWwindow* window, double xOffset, double yOffset);
  static void onWindowResize(GLFWwindow* window, int width, int height);
  void        cameraChanged();

  template<typename T>
  void setUniformInternal(int location, const T& val);

  std::vector<Shader> mShaders;
  glm::mat4           mProj;
  glm::mat4           mView;
  glm::ivec2          mWindowSize;
  size_t              mShaderIndex    = SIZE_MAX;
  Projection          mProjectionMode = Projection::PERSPECTIVE;
};

}  // namespace view
}  // namespace gal
