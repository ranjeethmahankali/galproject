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

class Widget
{
public:
  virtual void draw() = 0;
};

class Panel : public Widget
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
};

Panel& newPanel(const std::string& title);

void drawAllPanels();

class Text : public Widget
{
public:
  Text(const std::string& text);

  void draw();

private:
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

  std::vector<HandlerFn> mHandlers;
  std::string            mLabel;
  T                      mValue;

protected:
  void handleChanges()
  {
    if (!ImGui::IsItemActive()) {
      return;
    }
    for (auto handler : mHandlers) {
      handler(mValue);
    }
  };
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

  void draw()
  {
    ImGui::SliderFloat(
      this->mLabel.c_str(), &(this->mValue), this->mRange[0], this->mRange[1]);
    this->handleChanges();
  };

private:
  T mRange[2];
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

  void draw()
  {
    ImGui::SliderFloat3(
      this->mLabel.c_str(), this->mValue, this->mRange[0], this->mRange[1]);
    this->handleChanges();
  };

private:
  T mRange[2];
};

using SliderF  = Slider<float>;
using SliderF3 = Slider3<float>;

}  // namespace view
}  // namespace gal