
#include <galcore/Util.h>
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
  if (!sFont) {
    std::string absPath = utils::absPath("CascadiaMono.ttf");
    sFont               = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 17.0f);
  }
  ImGui::StyleColorsDark();  // Dark Mode

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion);
};

void imGuiNewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
};

void imGuiRender()
{
  ImGui::Render();
}

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

void Panel::addWidget(const std::shared_ptr<Widget>& widget)
{
  mWidgets.push_back(widget);
};

static std::vector<std::shared_ptr<Panel>> sPanels;

Panel& newPanel(const std::string& title)
{
  sPanels.push_back(std::make_shared<Panel>(title));
  return *sPanels.back();
};

void drawAllPanels()
{
  for (auto& p : sPanels) {
    p->draw();
  }
};

Text::Text(const std::string& value)
    : mValue(value) {};

void Text::draw()
{
  ImGui::Text(mValue.c_str());
};

TextInput::TextInput(const std::string& label, const std::string& value)
    : InputWidget<std::string>(label, value)
{
  mValue.reserve(1024);  // avoids reallocation for each frame.
};

static int TextInputResizeCallback(ImGuiInputTextCallbackData* data)
{
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    std::string* str = (std::string*)data->UserData;
    str->resize(data->BufSize);
    data->Buf = str->data();
  }
  return 0;
}

void TextInput::draw()
{
  ImGui::InputText(mLabel.c_str(),
                   mValue.data(),
                   mValue.size(),
                   ImGuiInputTextFlags_CallbackResize,
                   TextInputResizeCallback,
                   (void*)(&mValue));

  if (!ImGui::IsItemActive()) {
    handleChanges();
  }
}

CheckBox::CheckBox(const std::string& label, bool value)
    : InputWidget<bool>(label, value)
{}

void CheckBox::draw()
{
  ImGui::Checkbox(mLabel.c_str(), &mValue);
}

const bool* CheckBox::checkedPtr() const
{
  return &mValue;
}
}  // namespace view
}  // namespace gal