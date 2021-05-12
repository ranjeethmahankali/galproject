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
  bool                          orthoMode     = false;
  std::pair<uint32_t, uint32_t> polygonMode   = {uint32_t(GL_FRONT_AND_BACK),
                                               uint32_t(GL_FILL)};

  void apply() const;

  size_t opacityScore() const;
};

class Drawable
{
public:
  Drawable()          = default;
  virtual ~Drawable() = default;

  virtual void draw() const = 0;
  virtual bool opaque() const;
  const Box3&  bounds() const;

protected:
  void setBounds(const Box3& bounds);

private:
  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;
  Drawable(Drawable&&)                       = default;

  Box3 mBounds;
};

// Template specialization needed.
template<typename T>
struct MakeDrawable : public std::false_type
{
  static std::shared_ptr<Drawable> get(const T&                     geom,
                                       std::vector<RenderSettings>& renderSettings);
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
  struct RenderData
  {
    std::shared_ptr<Drawable> drawable;
    RenderSettings            settings;
    /**
     * @brief Before drawing the drawable, the boolean value stored at this pointer will
     * be checked. The drawable will be drawn only if it is set to true.
     * Who ever created this drawable is responsible for the lifetime of this boolean
     * pointer.
     */
    const bool* visibilityFlag;
  };

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
                       float near   = -5.f,
                       float far    = 100.0f);

  void set2dMode(bool flag);

  size_t shaderId(const std::string& name) const;

  void useShader(size_t shaderId);

  void render() const;

private:
  Context();

  std::vector<Shader> mShaders;

  std::vector<RenderData> mDrawables;
  std::vector<size_t>     mRenderOrder;

  glm::mat4 mProj;
  glm::mat4 mView;

  int32_t mShaderIndex = -1;

  static void onMouseMove(GLFWwindow* window, double xpos, double ypos);
  static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
  static void onMouseScroll(GLFWwindow* window, double xOffset, double yOffset);
  static void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

  void cameraChanged();

  void zoomExtents();

  template<typename T>
  void setUniformInternal(int location, const T& val);

  void insertKeyInPlace(size_t key, const std::shared_ptr<Drawable>& drawable);

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
  size_t addDrawable(const T& val, const bool* visibility)
  {
    static_assert(MakeDrawable<T>::value, "This is not a drawable type");
    std::vector<RenderSettings> settings;
    auto                        drawable = MakeDrawable<T>::get(val, settings);
    for (const auto& s : settings) {
      mDrawables.push_back({drawable, s, visibility});
    }
    std::sort(
      mDrawables.begin(), mDrawables.end(), [](const RenderData& a, const RenderData& b) {
        return a.settings.opacityScore() > b.settings.opacityScore();
      });
    return size_t(drawable.get());
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
  size_t replaceDrawable(size_t id, const T& val, const bool* visibility)
  {
    removeDrawable(id);
    return addDrawable<T>(val, visibility);
  };

  void clearDrawables();
};

}  // namespace view
}  // namespace gal