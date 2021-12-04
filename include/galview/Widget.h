#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>

#include <galview/GLUtil.h>

namespace gal {
namespace view {

void initializeImGui(GLFWwindow* window, const char* glslVersion);

void imGuiNewFrame();

void imGuiRender();

class Widget
{
public:
  virtual ~Widget()   = default;
  virtual void draw() = 0;
};

class Panel final : public Widget
{
public:
  Panel(const std::string& title);
  virtual ~Panel() = default;

  void draw();

private:
  std::vector<std::shared_ptr<Widget>> mWidgets;
  std::string                          mTitle;

public:
  void addWidget(const std::shared_ptr<Widget>& widget);

  /**
   * @brief Clears all widgets / contents inside this panel.
   * Expect the panel to be emptied.
   */
  void clear();

  template<typename T, typename... TArgs>
  std::shared_ptr<T> newWidget(const TArgs&... args)
  {
    static_assert(std::is_base_of_v<Widget, T>, "Type must inherit from Widget.");
    auto w = std::make_shared<T>(args...);
    addWidget(w);
    return w;
  };
};

Panel& newPanel(const std::string& title);

void drawAllPanels();

class Text : public Widget
{
public:
  Text(const std::string& text);
  virtual ~Text() = default;

  void set(const std::string& text);

  void draw();

protected:
  std::string mValue;
};

class Button : public Widget
{
  std::string           mLabel;
  std::function<void()> mOnClick;

public:
  Button(const std::string& label, const std::function<void()>& onClick);
  virtual ~Button() = default;

  void draw();
};

template<typename T>
class InputWidget : public Widget
{
public:
  using HandlerFn = void (*)(const T&);

  virtual void addHandler(const HandlerFn& fn) final { mHandlers.push_back(fn); };

protected:
  InputWidget(const std::string& label)
      : mLabel(label) {};
  InputWidget(const std::string& label, const T& value)
      : mLabel(label)
      , mValue(value) {};
  virtual ~InputWidget() = default;

  std::vector<HandlerFn> mHandlers;
  std::string            mLabel;
  T                      mValue;
  bool                   mEdited = false;

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

  virtual void handleChanges()
  {
    if (!isEdited())
      return;

    for (auto handler : mHandlers) {
      handler(mValue);
    }
    clearEdited();
  };

public:
  const T& value() const { return mValue; };
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

  void draw()
  {
    drawSlider<T>(
      this->mLabel.c_str(), &(this->mValue), this->mRange[0], this->mRange[1]);
    this->checkEdited();
    this->handleChanges();
  };

private:
  T mRange[2];
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
    this->mValue = VecType(val);
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

  void draw()
  {
    static_assert(N < 5, "Unsupported vector length");
    drawSlider<T, N>(
      this->mLabel.c_str(), &(this->mValue[0]), this->mRange[0], this->mRange[1]);
    this->checkEdited();
    this->handleChanges();
  };

private:
  T mRange[2];
};

class TextInput : public InputWidget<std::string>
{
public:
  TextInput(const std::string& label, const std::string& value);
  virtual ~TextInput() = default;

  void draw();
};

class TextInputBox : public InputWidget<std::string>
{
public:
  TextInputBox(const std::string& label);
  virtual ~TextInputBox() = default;

  void draw();
};

class CheckBox : public InputWidget<bool>
{
public:
  CheckBox(const std::string& label, bool value);
  virtual ~CheckBox() = default;

  void        draw();
  const bool* checkedPtr() const;
};

}  // namespace view
}  // namespace gal
