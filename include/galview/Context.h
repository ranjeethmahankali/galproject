#pragma once

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

class Drawable
{
public:
  Drawable()          = default;
  virtual ~Drawable() = default;

  virtual void draw() const = 0;
  virtual bool opaque() const;

private:
  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;
  Drawable(Drawable&&)                       = default;
};

// Template specialization needed.
template<typename T>
struct MakeDrawable
{
  static std::shared_ptr<Drawable> get(const T& geom);
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
      std::cerr << "Uniform " << name << " not found.\n";
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
                       float near   = 0.01f,
                       float far    = 100.0f);

  size_t shaderId(const std::string& name) const;

  void useShader(size_t shaderId);

  void render() const;

private:
  Context();

  std::vector<Shader>                         mShaders;
  std::map<size_t, std::shared_ptr<Drawable>> mDrawables;
  glm::mat4                                   mProj;
  glm::mat4                                   mView;
  int32_t                                     mShaderIndex = -1;

  static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
  static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
  static void onMouseScroll(GLFWwindow* window, double xOffset, double yOffset);
  static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

  void cameraChanged();

  template<typename T>
  void setUniformInternal(int location, const T& val);

public:
  /**
   * @brief Adds a drawable object to the scene.
   * The render data (tessellations) are generated for the object and added to the scene.
   * MakeDrawable template is used to generate the render data so the object must
   * specialize that template.
   * @tparam T The type of the object.
   * @param val The object.
   * @return size_t The id of the render data added. This can be used later to
   * replace the object, or to delete the object from the scene.
   */
  template<typename T>
  size_t addDrawable(const T& val)
  {
    auto   drawable = MakeDrawable<T>::get(val);
    size_t id       = size_t(drawable.get());
    mDrawables.emplace(id, drawable);
    return id;
  };

  void removeDrawable(size_t id);

  /**
   * @brief Replaces the render data with the given id with the new data.
   * @tparam T The type of the new object.
   * @param id The old id.
   * @param val The new object.
   * @return size_t The id of the new render data.
   */
  template<typename T>
  size_t replaceDrawable(size_t id, const T& val)
  {
    removeDrawable(id);
    return addDrawable<T>(val);
  };
};

}  // namespace view
}  // namespace gal