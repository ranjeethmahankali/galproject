#include <assert.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include <imgui.h>
#include <pybind11/embed.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/adaptors.hpp>

#include <galfunc/Functions.h>
#include <galfunc/Graph.h>
#include <galfunc/Property.h>
#include <galview/Command.h>
#include <galview/GLUtil.h>
#include <galview/GuiFunctions.h>
#include <galview/Interaction.h>
#include <galview/Views.h>

namespace gal {
namespace view {

static ImFont*           sFont      = nullptr;
static ImFont*           sFontLarge = nullptr;
static std::stringstream sResponseStream;
static auto              sResponseSink =
  std::make_shared<spdlog::sinks::ostream_sink_mt>(sResponseStream);
static auto sLogger = std::make_shared<spdlog::logger>("galview", sResponseSink);
static std::vector<Panel> sPanels;

static void initializeImGui(GLFWwindow* window, const char* glslVersion)
{
  glutil::logger().info("Setting up ImGui...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO&    io      = ImGui::GetIO();
  std::string absPath = utils::absPath("CascadiaMono.ttf");
  glutil::logger().info("Loading font {}", absPath);
  if (!sFont) {
    sFont = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 17.f);
  }
  if (!sFontLarge) {
    sFontLarge = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 19.f);
  }
  ImGui::StyleColorsDark();  // Dark Mode
  ImGuiStyle& style    = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glslVersion);
};

void imGuiNewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
};

Panel::Panel(const std::string& title)
    : mTitle(title) {};

Panel::Panel(const std::string& title, bool visible)
    : mTitle(title)
    , mIsVisible(visible)
{}

void Panel::draw() const
{
  if (!mIsVisible) {
    return;
  }
  ImGui::SetNextWindowBgAlpha(0.9f);
  ImGui::Begin(mTitle.c_str(), &mIsVisible, ImGuiWindowFlags_HorizontalScrollbar);
  if (sFont) {
    ImGui::PushFont(sFont);
  }
  for (auto& w : mWidgets) {
    w->draw();
  }
  for (DrawCBType cb : mCallbacks) {
    cb();
  }
  if (sFont) {
    ImGui::PopFont();
  }
  ImGui::End();
};

void Panel::addWidget(Widget* widget)
{
  mWidgets.push_back(widget);
};

void Panel::addCallback(DrawCBType cb)
{
  mCallbacks.push_back(cb);
}

void Panel::clear()
{
  mWidgets.clear();
}

const std::string& Panel::title() const
{
  return mTitle;
}

bool Panel::visibility() const
{
  return mIsVisible;
}

void Panel::visibility(bool visible)
{
  mIsVisible = visible;
}

Text::Text(const std::string& value)
    : mValue(value) {};

void Text::draw()
{
  ImGui::Text("%s", mValue.c_str());
};

const std::string& Text::value() const
{
  return mValue;
}

std::string& Text::value()
{
  return mValue;
}

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

spdlog::logger& logger()
{
  return *sLogger;
}

inline glm::vec2 toGlm(ImVec2 v)
{
  return {v.x, v.y};
}

static void initPanels()
{
  sPanels.clear();
  sPanels.emplace_back("inputs");
  sPanels.emplace_back("outputs");
  sPanels.emplace_back("history", false);
}

static auto panelIterByName(const std::string& name)
{
  return std::find_if(sPanels.begin(), sPanels.end(), [&name](const Panel& panel) {
    return panel.title() == name;
  });
}

Panel& panelByName(const std::string& name)
{
  auto match = panelIterByName(name);
  if (match == sPanels.end()) {
    throw std::runtime_error("Cannot find panel with the given name");
  }
  return *match;
}

static void drawPanels()
{
  for (const auto& panel : sPanels) {
    panel.draw();
  }
}

void setPanelVisibility(const std::string& name, bool visible)
{
  auto match = panelIterByName(name);
  if (match != sPanels.end()) {
    match->visibility(visible);
    view::logger().info("Visibility of {} panel set to {}.", name, visible);
  }
  else {
    view::logger().error("No panel named {} was found.", name);
  }
}

int runPythonDemoFile(const fs::path& demoPath)
{
  try {
    view::demoFilePath() = demoPath;
    py::dict global;
    global["__file__"] = demoPath.string();
    global["__name__"] = "__main__";
    py::eval_file(demoPath.c_str(), global);
    logger().info("Loaded demo file: {}", demoPath.string());
    return 0;
  }
  catch (std::exception& e) {
    PyErr_Print();
    logger().error(e.what());
    return 1;
  }
}

static std::string  sCmdline    = "";
static std::string  sResponse   = "";
static std::string* sHistoryPtr = nullptr;

void init(GLFWwindow* window, const char* glslVersion)
{
  static std::unique_ptr<Text> sHistoryWidget;
  initializeImGui(window, glslVersion);
  initPanels();
  sResponseSink->set_pattern("[%l] %v");
  Panel& historyPanel = panelByName("history");
  sHistoryWidget      = std::make_unique<Text>("");
  historyPanel.addWidget(sHistoryWidget.get());
  sHistoryPtr = &(sHistoryWidget->value());
  initCommands();
}

static int cmdLineCallback(ImGuiInputTextCallbackData* data)
{
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    std::string* str = (std::string*)data->UserData;
    str->resize(data->BufSize);
    data->Buf = str->data();
  }
  else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
    static std::string sCharsToInsert = "";
    std::string*       cmd            = (std::string*)data->UserData;
    autocompleteCommand(*cmd, sCharsToInsert);
    data->InsertChars(data->CursorPos, sCharsToInsert.c_str());
  }
  return 0;
}

void draw(GLFWwindow* window)
{
  static float sCmdHeight = 50.f;
  // Get the parent window size.
  int width = 0, height = 0;
  glfwGetWindowSize(window, &width, &height);
  float  fwidth = float(width), fheight = float(height);
  float  cmdWidth = std::min(fwidth, 960.f);
  size_t nLines =
    std::max(size_t(1), size_t(std::count(sResponse.begin(), sResponse.end(), '\n')));
  float            cmdHeight = sCmdHeight + float(nLines) * 17.f;
  ImGuiWindowFlags wflags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
  // ImGuiWindowFlags wflags = 0;
  ImGui::SetNextWindowPos(ImVec2(0.5f * (fwidth - cmdWidth), fheight - cmdHeight));
  ImGui::SetNextWindowSize(ImVec2(cmdWidth, cmdHeight));
  ImGui::SetNextWindowBgAlpha(0.8f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::Begin("command-window", nullptr, wflags);
  ImGui::PushStyleColor(ImGuiCol_Text, 0xff999999);
  ImGui::Text("%s", sResponse.c_str());
  ImGui::PopStyleColor();
  if (sFontLarge) {
    ImGui::PushFont(sFontLarge);
  }
  ImGui::PushItemWidth(cmdWidth - 70.f);
  ImGuiInputTextFlags tflags = ImGuiInputTextFlags_CallbackResize |
                               ImGuiInputTextFlags_EnterReturnsTrue |
                               ImGuiInputTextFlags_CallbackCompletion;
  ImGui::Text(">>> ");
  ImGui::SameLine();
  if (ImGui::InputText("",
                       sCmdline.data(),
                       sCmdline.size(),
                       tflags,
                       cmdLineCallback,
                       (void*)(&sCmdline))) {
    sCmdline.erase(std::remove(sCmdline.begin(), sCmdline.end(), '\0'), sCmdline.end());
    queueCommands(sCmdline);
    sCmdline.clear();
    sResponse.clear();
  }
  if (sResponseStream.rdbuf()->in_avail() > 0) {
    sResponse = sResponseStream.str();
    *(sHistoryPtr) += sResponse;
    sResponseStream.str("");
  }
  if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
    ImGui::SetKeyboardFocusHere(-1);
  }
  ImGui::PopItemWidth();
  ImGui::PopStyleVar();
  if (sFontLarge) {
    ImGui::PopFont();
  }
  ImGui::End();
  // Draw all panels.
  drawPanels();
}

void destroy(GLFWwindow* window)
{
  glutil::logger().info("Cleaning up...\n");
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  if (window) {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
}

}  // namespace view
}  // namespace gal
