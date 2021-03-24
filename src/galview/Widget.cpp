
#include <galview/Widget.h>
#include <iostream>

namespace gal {
namespace view {

static ImFont* sFont = nullptr;

void initializeImGui(GLFWwindow* window, const char* glslVersion)
{
  std::cout << "Setting up ImGui...\n";
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  if (!sFont)
    sFont = io.Fonts->AddFontFromFileTTF("CascadiaMono.ttf", 17.0f);
  ImGui::StyleColorsDark();  // Dark Mode

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion);
};

Panel::Panel(const std::string& title)
    : mTitle(title) {};

void Panel::draw()
{
  ImGui::Begin(mTitle.c_str());
  if (sFont)
    ImGui::PushFont(sFont);
  for (auto& w : mWidgets) {
    w->draw();
  }
  if (sFont)
    ImGui::PopFont();
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

}  // namespace view
}  // namespace gal