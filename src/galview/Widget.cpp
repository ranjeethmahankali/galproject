#include <galview/Widget.h>
#include <iostream>

namespace gal {
namespace view {

void initializeImGui(GLFWwindow* window, const char* glslVersion)
{
  std::cout << "Setting up ImGui...\n";
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  ImGui::StyleColorsDark();  // Dark Mode

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion);
};

Panel::Panel(const std::string& title)
    : mTitle(title) {};

void Panel::draw()
{
  ImGui::Begin(mTitle.c_str());
  for (auto& w : mWidgets) {
    w->draw();
  }
  ImGui::End();
};

static std::vector<Panel> sPanels;

Panel& newPanel(const std::string& title)
{
  sPanels.emplace_back(title);
  return sPanels.back();
};

void drawAllPanels()
{
  for (auto& p : sPanels) {
    p.draw();
  }
};

Text::Text(const std::string& value)
    : mValue(value) {};

void Text::draw()
{
  ImGui::Text(mValue.c_str());
};

Slider::Slider(const std::string& label, float min, float max)
    : InputWidget<float>(label, std::clamp(0.0f, min, max))
    , mRange {min, max} {};

Slider::Slider(const std::string& label, float min, float max, float value)
    : InputWidget<float>(label, std::clamp(value, min, max))
    , mRange {min, max} {};

void Slider::draw()
{
  ImGui::SliderFloat(mLabel.c_str(), &mValue, mRange[0], mRange[1]);
  handleChanges();
};

Slider3::Slider3(const std::string& label, float min, float max)
    : InputWidget<float[3]>(label)
    , mRange {min, max}
{
  std::fill_n(mValue, 3, std::clamp(0.0f, mRange[0], mRange[1]));
};

Slider3::Slider3(const std::string& label, float min, float max, const float (&value)[3])
    : InputWidget<float[3]>(label)
    , mRange {min, max}
{
  std::copy_n(value, 3, mValue);
};

void Slider3::draw()
{
  ImGui::SliderFloat3(mLabel.c_str(), mValue, mRange[0], mRange[1]);
  handleChanges();
};

}  // namespace view
}  // namespace gal