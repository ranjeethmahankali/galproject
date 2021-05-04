#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <galview/GLUtil.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

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

  void draw();

private:
  std::vector<std::shared_ptr<Widget>> mWidgets;
  std::string                          mTitle;

public:
  template<typename T, typename... TArgs>
  std::shared_ptr<T> newWidget(const TArgs&... args)
  {
    static_assert(std::is_base_of_v<Widget, T>, "Type must inherit from Widget.");
    auto w = std::make_shared<T>(args...);
    mWidgets.push_back(w);
    return w;
  };

  void addWidget(const std::shared_ptr<Widget>& widget);

  void clearWidgets();
};

Panel& newPanel(const std::string& title);

void drawAllPanels();

class Text : public Widget
{
public:
  Text(const std::string& text);
  virtual ~Text() = default;

  void draw();

protected:
  std::string mValue;
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

protected:
  virtual void handleChanges()
  {
    if (!ImGui::IsItemEdited()) {
      return;
    }
    for (auto handler : mHandlers) {
      handler(mValue);
    }
  };

public:
  const T& value() const { return mValue; };
};

template<typename T>
inline void drawSlider(const char* label, T* value, T min, T max);

template<>
inline void drawSlider<float>(const char* label, float* value, float min, float max)
{
  ImGui::SliderFloat(label, value, min, max);
};

template<>
inline void drawSlider<int>(const char* label, int* value, int min, int max)
{
  ImGui::SliderInt(label, value, min, max);
};

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
    this->handleChanges();
  };

private:
  T mRange[2];
};

template<typename T>
inline void drawSlider3(const char* label, T (&value)[3], T min, T max);

template<>
inline void drawSlider3<float>(const char* label, float (&value)[3], float min, float max)
{
  ImGui::SliderFloat3(label, value, min, max);
};

template<typename T>
class Slider3 : public InputWidget<T[3]>
{
  static_assert(std::is_fundamental_v<T>, "Must be a fundamental type");
  static constexpr T ZERO = T(0);

public:
  Slider3(const std::string& label, T min, T max)
      : InputWidget<T[3]>(label)
      , mRange {min, max}
  {
    std::fill_n(this->mValue, 3, std::clamp(ZERO, this->mRange[0], this->mRange[1]));
  };

  Slider3(const std::string& label, T min, T max, const T (&value)[3])
      : InputWidget<T[3]>(label)
      , mRange {min, max}
  {
    std::copy_n(value, 3, this->mValue);
  };

  virtual ~Slider3() = default;

  void draw()
  {
    drawSlider3<T>(this->mLabel.c_str(), this->mValue, this->mRange[0], this->mRange[1]);
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

using SliderF  = Slider<float>;
using SliderI  = Slider<int>;
using SliderF3 = Slider3<float>;

}  // namespace view
}  // namespace gal