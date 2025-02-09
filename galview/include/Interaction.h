#pragma once

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// IMPORTANT: This needs to be included before the other ones.
#include <GLUtil.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>

namespace gal {
namespace view {

/**
 * @brief Starts a new imgui frame.
 */
void imGuiNewFrame();
/**
 * @brief Gets the viewer instance for the viewer.
 */
spdlog::logger& logger();
/**
 * @brief Load an interactive demo from the given python file.
 *
 * @param demoPath Path to the python demo file.
 * @return int 0 if all goes well, non-zero values otherwise.
 */
int runPythonDemoFile(const std::filesystem::path& demoPath);
/**
 * @brief Set the visibility of a panel with the given name.
 *
 * @param name Name of the panel.
 * @param visible If true, panel will be made visible. If false, it will be made
 * invisible.
 */
void setPanelVisibility(const std::string& name, bool visible);

class Widget
{
public:
  Widget()                         = default;
  virtual ~Widget()                = default;
  Widget(Widget const&)            = delete;
  Widget(Widget&&)                 = delete;
  Widget& operator=(Widget const&) = delete;
  Widget& operator=(Widget&&)      = delete;

  virtual void draw() = 0;
};

class Panel
{
public:
  using DrawCBType = void (*)();

  explicit Panel(const std::string& title);
  Panel(const std::string& title, bool visible);

  void draw() const;

private:
  std::vector<Widget*>    mWidgets;
  std::vector<DrawCBType> mCallbacks;
  std::string             mTitle;
  mutable bool            mIsVisible = true;

public:
  void addWidget(Widget* widget);

  void addCallback(DrawCBType cb);

  /**
   * @brief Clears all widgets / contents inside this panel.
   * Expect the panel to be emptied.
   */
  void clear();

  const std::string& title() const;
  bool               visibility() const;
  void               visibility(bool visible);
};

Panel& panelByName(const std::string& name);

class Text : public Widget
{
public:
  explicit Text(const std::string& text);
  virtual ~Text() = default;

  Text(Text const&)            = delete;
  Text(Text&&)                 = delete;
  Text& operator=(Text const&) = delete;
  Text& operator=(Text&&)      = delete;

  void draw() override;

  const std::string& value() const;
  std::string&       value();

private:
  std::string mValue;
};

class Button : public Widget
{
  std::string           mLabel;
  std::function<void()> mOnClick;

public:
  Button(const std::string& label, const std::function<void()>& onClick);
  virtual ~Button() = default;

  Button(Button const&)            = delete;
  Button(Button&&)                 = delete;
  Button& operator=(Button const&) = delete;
  Button& operator=(Button&&)      = delete;

  void draw() override;
};

template<typename T>
class InputWidget : public Widget
{
private:
  std::string mLabel;
  T           mValue;
  bool        mEdited = false;

public:
  explicit InputWidget(const std::string& label)
      : mLabel(label)
      , mValue(T())
  {}

  InputWidget(const std::string& label, const T& value)
      : mLabel(label)
      , mValue(value)
  {}

  virtual ~InputWidget() = default;

  InputWidget(InputWidget const&)            = delete;
  InputWidget(InputWidget&&)                 = delete;
  InputWidget& operator=(InputWidget const&) = delete;
  InputWidget& operator=(InputWidget&&)      = delete;

  T const&           value() const { return mValue; }
  T&                 value() { return mValue; }
  std::string const& label() const { return mLabel; }
  std::string&       label() { return mLabel; }

protected:
  /**
   * @brief IMPORTANT: THis can only be called right after drawing this widget!
   * This is because the ImGui works in "immediate" mode.
   */
  void checkEdited()
  {
    if (ImGui::IsItemEdited())
      mEdited = true;
  }

  bool isEdited() const { return mEdited; }

  void clearEdited() { mEdited = false; }

  void setEdited() { mEdited = true; }

  virtual void handleChanges() = 0;
};

template<typename T, int N = 1>
inline void drawSlider(const char* label, T* value, T min, T max)
{
  static_assert((N > 0 && N < 5) && (std::is_same_v<T, int> || std::is_same_v<T, float>),
                "Slider type not supported by ImGui.");

  if constexpr (std::is_same_v<T, float>) {
    if constexpr (N == 1) {
      ImGui::SliderFloat(label, value, min, max);
    }
    else if constexpr (N == 2) {
      ImGui::SliderFloat2(label, value, min, max);
    }
    else if constexpr (N == 3) {
      ImGui::SliderFloat3(label, value, min, max);
    }
    else if constexpr (N == 4) {
      ImGui::SliderFloat4(label, value, min, max);
    }
  }
  else if constexpr (std::is_same_v<T, int>) {
    if constexpr (N == 1) {
      ImGui::SliderInt(label, value, min, max);
    }
    else if constexpr (N == 2) {
      ImGui::SliderInt2(label, value, min, max);
    }
    else if constexpr (N == 3) {
      ImGui::SliderInt3(label, value, min, max);
    }
    else if constexpr (N == 4) {
      ImGui::SliderInt4(label, value, min, max);
    }
  }
}

template<typename T>
class Slider : public InputWidget<T>
{
  static_assert(std::is_fundamental_v<T>, "Must be a fundamental type");
  static constexpr T ZERO = T(0);

public:
  Slider(const std::string& label, T min, T max)
      : InputWidget<T>(label, std::clamp(ZERO, min, max))
      , mRange {min, max} {};
  Slider(const std::string& label, T min, T max, T value)
      : InputWidget<T>(label, std::clamp(value, min, max))
      , mRange {min, max} {};

  virtual ~Slider() = default;

  Slider(Slider const&)            = delete;
  Slider(Slider&&)                 = delete;
  Slider& operator=(Slider const&) = delete;
  Slider& operator=(Slider&&)      = delete;

  void draw()
  {
    drawSlider<T>(
      this->label().c_str(), &(this->value()), this->mRange[0], this->mRange[1]);
    this->checkEdited();
    this->handleChanges();
  };

private:
  std::array<T, 2> mRange {};
};

/**
 * @brief This specialization is for sliders that control glm vectors.
 * @tparam N The length of the vector.
 * @tparam T The type of the vector.
 * @tparam Q Precision.
 */
template<int N, typename T, glm::qualifier Q>
class Slider<glm::vec<N, T, Q>> : public InputWidget<glm::vec<N, T, Q>>
{
  using VecType = glm::vec<N, T, Q>;

  static constexpr T ZERO = T(0);

public:
  /**
   * @brief Construct a new Slider object.
   * This constructor expects min, max, and val args of type T instead of glm vectors.
   * This is necessary to conform to ImGui's functions for drawing multi-sliders.
   * @param label
   * @param min
   * @param max
   * @param val
   */
  Slider(const std::string& label, const T& min, const T& max, const T& val)
      : InputWidget<VecType>(label)
      , mRange {min, max}
  {
    this->value() = VecType(val);
  };

  /**
   * @brief Construct a new Slider object
   * This constructor expects min, and max args of type T instead of glm vectors.
   * This is necessary to conform to ImGui's functions for drawing multi-sliders.
   * @param label
   * @param min
   * @param max
   */
  Slider(const std::string& label, const T& min, const T& max)
      : Slider(label, min, max, std::clamp(ZERO, min, max)) {};

  virtual ~Slider() = default;

  Slider(Slider const&)            = delete;
  Slider(Slider&&)                 = delete;
  Slider& operator=(Slider const&) = delete;
  Slider& operator=(Slider&&)      = delete;

  void draw()
  {
    static_assert(N < 5, "Unsupported vector length");
    drawSlider<T, N>(
      this->label().c_str(), &(this->value()[0]), this->mRange[0], this->mRange[1]);
    this->checkEdited();
    this->handleChanges();
  };

private:
  std::array<T, 2> mRange {};
};

class TextInput : public InputWidget<std::string>
{
public:
  TextInput(const std::string& label, const std::string& value);
  virtual ~TextInput() = default;

  TextInput(TextInput const&)            = delete;
  TextInput(TextInput&&)                 = delete;
  TextInput& operator=(TextInput const&) = delete;
  TextInput& operator=(TextInput&&)      = delete;

  void draw() override;
};

class CheckBox : public InputWidget<bool>
{
public:
  CheckBox(const std::string& label, bool value);
  virtual ~CheckBox() = default;

  CheckBox(CheckBox const&)            = delete;
  CheckBox(CheckBox&&)                 = delete;
  CheckBox& operator=(CheckBox const&) = delete;
  CheckBox& operator=(CheckBox&&)      = delete;

  void        draw() override;
  const bool* checkedPtr() const;
  void        handleChanges() override;
};

void init(GLFWwindow* window, const char* glslVersion);
void draw(GLFWwindow* window);
void destroy(GLFWwindow* window);

}  // namespace view
}  // namespace gal
