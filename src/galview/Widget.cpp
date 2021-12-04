
#include <iostream>

#include <imgui.h>

#include <galcore/Util.h>
#include <galview/GLUtil.h>
#include <galview/Widget.h>

namespace gal {
namespace view {

static ImFont* sFont      = nullptr;
static ImFont* sFontLarge = nullptr;

void initializeImGui(GLFWwindow* window, const char* glslVersion)
{
  std::cout << "Setting up ImGui...\n";
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO&    io      = ImGui::GetIO();
  std::string absPath = utils::absPath("CascadiaMono.ttf");
  if (!sFont) {
    sFont = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 17.f);
  }
  if (!sFontLarge) {
    sFontLarge = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 20.f);
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

void Panel::clear()
{
  std::cout << "Clearing all widgets...\n";
  mWidgets.clear();
}

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

void Text::set(const std::string& text)
{
  mValue = text;
}

void Text::draw()
{
  ImGui::Text("%s", mValue.c_str());
};

Button::Button(const std::string& label, const std::function<void()>& onClick)
    : mLabel(label)
    , mOnClick(onClick) {};

void Button::draw()
{
  auto size = ImGui::CalcTextSize(mLabel.c_str());
  ImGui::Button(mLabel.c_str(), ImVec2(size.x + 10.f, size.y + 10.f));
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    mOnClick();
  }
}

TextInput::TextInput(const std::string& label, const std::string& value)
    : InputWidget<std::string>(label, value)
{
  mValue.reserve(1024);  // avoids reallocation for each frame.
};

static int TextInputCallBack(ImGuiInputTextCallbackData* data)
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
                   TextInputCallBack,
                   (void*)(&mValue));

  checkEdited();
  if (!ImGui::IsItemActive()) {
    handleChanges();
  }
}

TextInputBox::TextInputBox(const std::string& label)
    : InputWidget<std::string>(label, "")
{
  mValue.reserve(1024);
}

void TextInputBox::draw()
{
  ImGui::InputTextMultiline(mLabel.c_str(),
                            mValue.data(),
                            mValue.size(),
                            ImVec2(200, ImGui::GetTextLineHeight() * 4),
                            ImGuiInputTextFlags_CallbackResize,
                            TextInputCallBack,
                            (void*)(&mValue));
  checkEdited();
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
  checkEdited();
  handleChanges();
}

const bool* CheckBox::checkedPtr() const
{
  return &mValue;
}

namespace cmdinterface {

static int cmdLineCallback(ImGuiInputTextCallbackData* data)
{
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    std::string* str = (std::string*)data->UserData;
    str->resize(data->BufSize);
    data->Buf = str->data();
  }
  else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
    // TODO: Implement proper auto completion here.
    data->InsertChars(data->CursorPos, "..");
  }
  return 0;
}

void show(GLFWwindow* window)
{
  static std::string cmdline    = "";
  static bool        isVisible  = true;
  static float       sCmdHeight = 50.f;

  // Get the parent window size.
  int width = 0, height = 0;
  glfwGetWindowSize(window, &width, &height);
  float fwidth = float(width), fheight = float(height);

  ImGuiWindowFlags wflags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
  // ImGuiWindowFlags wflags = 0;
  ImGui::SetNextWindowPos(ImVec2(0.f, fheight - sCmdHeight));
  ImGui::SetNextWindowSize(ImVec2(fwidth, sCmdHeight));
  ImGui::Begin("command-window", &isVisible, wflags);
  if (sFontLarge) {
    ImGui::PushFont(sFontLarge);
  }

  ImGuiInputTextFlags tflags = ImGuiInputTextFlags_CallbackResize |
                               ImGuiInputTextFlags_EnterReturnsTrue |
                               ImGuiInputTextFlags_CallbackCompletion;
  ImGui::Text("Command: ");
  ImGui::SameLine();
  ImGui::PushItemWidth(fwidth - 120.f);
  if (ImGui::InputText(
        "", cmdline.data(), cmdline.size(), tflags, cmdLineCallback, (void*)(&cmdline))) {
    cmdline.erase(std::remove(cmdline.begin(), cmdline.end(), '\0'), cmdline.end());
    std::cout << "Received command: " << cmdline << std::endl;
    cmdline.clear();
  }
  if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
    ImGui::SetKeyboardFocusHere(0);
  }
  ImGui::PopItemWidth();

  if (sFontLarge) {
    ImGui::PopFont();
  }
  ImGui::End();
}

}  // namespace cmdinterface

}  // namespace view
}  // namespace gal
