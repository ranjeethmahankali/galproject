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

class Slider : public InputWidget<float>
{
public:
  Slider(const std::string& label, float min, float max);
  Slider(const std::string& label, float min, float max, float value);

  void draw();

private:
  float mRange[2];
};

class Slider3 : public InputWidget<float[3]>
{
public:
  Slider3(const std::string& label, float min, float max);
  Slider3(const std::string& label, float min, float max, const float (&value)[3]);

  void draw();

private:
  float mRange[2];
};

}  // namespace view
}  // namespace gal