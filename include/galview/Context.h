#pragma once
#include <galview/Shader.h>
#include <vector>

namespace gal {
namespace view {

class Context
{
  class Shader
  {
  public:
    void loadFromName(const std::string& name);
    void loadFromSources(const std::string& vertSrc, const std::string& fragSrc);
    void loadFromFiles(const std::string& vertFilePath,
                                const std::string& fragFilePath);

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

  static void setWireframeMode(bool flag);
  static void setWireframeUniform(bool flag);
  static bool wireframeMode();

  template<typename T>
  void setUniform(const std::string& name, const T& val)
  {
    int loc = glGetUniformLocation(mShaders[mShaderIndex].mId, name.c_str());
    if (loc == -1) {
      std::cerr << "Uniform " << name << " not found.\n";
      return;
    }
    setUniformInternal<T>(loc, val);
  };

  void      useCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
  glm::mat4 mvpMatrix() const;

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

  size_t shaderId(const std::string& name) const;

  void useShader(size_t shaderId);

private:
  Context();

  std::vector<Shader> mShaders;
  glm::mat4           mProj;
  glm::mat4           mView;
  int32_t             mShaderIndex = -1;

  static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
  static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
  static void onMouseScroll(GLFWwindow* window, double xOffset, double yOffset);
  static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

  void cameraChanged();
  static void updateViewMatrix();

  template<typename T>
  void setUniformInternal(int location, const T& val);
};

}  // namespace view
}  // namespace gal