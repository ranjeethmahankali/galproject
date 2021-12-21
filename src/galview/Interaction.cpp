#include <galfunc/Functions.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <imgui.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <galcore/Util.h>
#include <galview/GLUtil.h>
#include <galview/GuiFunctions.h>
#include <galview/Interaction.h>
#include <galview/Views.h>

namespace gal {
namespace view {

static ImFont* sFont      = nullptr;
static ImFont* sFontLarge = nullptr;

static std::stringstream sResponseStream;
static auto              sResponseSink =
  std::make_shared<spdlog::sinks::ostream_sink_mt>(sResponseStream);
static auto     sLogger = std::make_shared<spdlog::logger>("galview", sResponseSink);
static fs::path sCurrentDemoPath = "";

using CmdFnType = void (*)(int, char**);
static std::unordered_map<std::string, CmdFnType> sCommandFnMap;

static std::vector<Panel> sPanels;

static void initializeImGui(GLFWwindow* window, const char* glslVersion)
{
  glutil::logger().info("Setting up ImGui...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO&    io      = ImGui::GetIO();
  std::string absPath = utils::absPath("CascadiaMono.ttf");
  if (!sFont) {
    sFont = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 17.f);
  }
  if (!sFontLarge) {
    sFontLarge = io.Fonts->AddFontFromFileTTF(absPath.c_str(), 19.f);
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
  ImGui::SetNextWindowBgAlpha(0.8f);
  ImGui::Begin(mTitle.c_str(), &mIsVisible, ImGuiWindowFlags_HorizontalScrollbar);
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

static void initPanels()
{
  sPanels.clear();
  sPanels.emplace_back("inputs");
  sPanels.emplace_back("outputs");
  sPanels.emplace_back("canvas", false);
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

static void setPanelVisibility(const std::string& name, bool visible)
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

void setDemoFilepath(const fs::path& path)
{
  sCurrentDemoPath = path;
}

void autocompleteCommand(const std::string& cmd, std::string& charsToInsert)
{
  static std::string sSuggestions = "";
  charsToInsert.clear();
  sSuggestions.clear();
  for (const auto& pair : sCommandFnMap) {
    const std::string& match   = std::get<0>(pair);
    auto               cmdend  = std::find(cmd.begin(), cmd.end(), '\0');
    size_t             cmdsize = std::distance(cmd.begin(), cmdend);
    if (cmdsize > match.size()) {
      continue;
    }
    if (std::equal(cmd.begin(), cmdend, match.begin())) {
      auto diffbegin = match.begin() + cmdsize;
      sSuggestions += " " + match;
      if (charsToInsert.empty()) {
        std::copy(diffbegin, match.end(), std::back_inserter(charsToInsert));
      }
      else {
        for (auto left = charsToInsert.begin();
             left != charsToInsert.end() && diffbegin != match.end();
             left++, diffbegin++) {
          if (*left != *diffbegin) {
            charsToInsert.erase(left, charsToInsert.end());
            break;
          }
        }
      }
    }
  }

  if (!sSuggestions.empty()) {
    logger().info("Possible completions:{}", sSuggestions);
  }
}

static void updateCanvas()
{
  auto   gdata = func::store::getGraphData();
  size_t count = gdata.size();
  // TODO: Incomplete.
}

int runPythonDemoFile(const fs::path& demoPath)
{
  try {
    view::setDemoFilepath(demoPath);
    boost::python::dict global;
    global["__file__"] = demoPath.string();
    global["__name__"] = "__main__";
    boost::python::exec_file(demoPath.c_str(), global);

    // Update the canvas panel.
    updateCanvas();

    logger().info("Loaded demo file: {}", demoPath.string());
    return 0;
  }
  catch (boost::python::error_already_set) {
    PyErr_Print();
    logger().error("Unable to load the demo... aborting...\n");
    return 1;
  }
}

void runCommand(const std::string& cmd)
{
  static std::string         sParsed;
  static std::vector<size_t> sIndices;
  static std::vector<char*>  sArgV;

  sParsed = cmd;
  sIndices.clear();
  for (size_t i = 0; i < sParsed.size(); i++) {
    char& c = sParsed[i];
    if (c == ' ') {
      c = '\0';
    }
    else if (i == 0 || (sParsed[i - 1] == '\0')) {
      sIndices.push_back(i);
    }
  }
  sArgV.resize(sIndices.size());
  std::transform(sIndices.begin(), sIndices.end(), sArgV.begin(), [](size_t i) {
    return i + sParsed.data();
  });

  if (sArgV.empty()) {
    // No command received.
    return;
  }
  std::string command = sArgV[0];
  auto        match   = sCommandFnMap.find(command);
  if (match == sCommandFnMap.end()) {
    logger().error("Unrecognized command {}", command);
    return;
  }

  view::logger().info(">>> {}", cmd);
  match->second(int(sArgV.size()), sArgV.data());
}

namespace cmdfuncs {

void reload(int, char**)
{
  gal::viewfunc::unloadAllOutputs();
  gal::func::store::unloadAllFunctions();
  gal::view::Views::clear();
  int err = runPythonDemoFile(sCurrentDemoPath);
  if (err != 0) {
    logger().error("Unable to run the demo file. Aborting...\n");
    std::exit(err);
  }
}

void twoDMode(int, char**)
{
  view::logger().info("Enabling 2d viewer mode...");
  view::Context::get().set2dMode(true);
}

void threeDMode(int, char**)
{
  view::logger().info("Enabling 3d viewer mode...");
  view::Context::get().set2dMode(false);
}

void zoomExtents(int, char**)
{
  view::logger().info("Zooming to extents...");
  view::Context::get().zoomExtents();
}

void show(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("show commands expects the name of the panel as an argument.");
    return;
  }
  setPanelVisibility(argv[1], true);
}

void hide(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("hide commands expects the name of the panel as an argument.");
    return;
  }
  setPanelVisibility(argv[1], false);
}

void wireframe(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("wireframe command expacts a on/off argument.");
    return;
  }
  bool flag = false;
  if (0 == std::strcmp(argv[1], "on")) {
    flag = true;
  }
  else if (0 == std::strcmp(argv[1], "off")) {
    flag = false;
  }
  else {
    logger().error(
      "Unrecognized option {} for the wireframe command. Expected either on or off.",
      argv[1]);
    return;
  }
  Context::get().setWireframeMode(flag);
  logger().info("Wireframe mode flag set to {}.", flag);
}

void meshEdges(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("meshedges command expacts a on/off argument.");
    return;
  }
  bool flag = false;
  if (0 == std::strcmp(argv[1], "on")) {
    flag = true;
  }
  else if (0 == std::strcmp(argv[1], "off")) {
    flag = false;
  }
  else {
    logger().error(
      "Unrecognized option {} for the meshedges command. Expected either on or off.",
      argv[1]);
    return;
  }
  Context::get().setMeshEdgeMode(flag);
  logger().info("Mesh edge visibility flag set to {}.", flag);
}

void perspective(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("perspective command expects an on/off argument.");
    return;
  }

  if (0 == std::strcmp(argv[1], "on")) {
    Context::get().setPerspective();
    logger().info("Now using a perspective camera.");
  }
  else if (0 == std::strcmp(argv[1], "off")) {
    Context::get().setOrthographic();
    logger().info("Now using an orthographic camera.");
  }
  else {
    logger().error(
      "Unrecorgnized option {} for the perspective command. Expected either on or off.",
      argv[1]);
    return;
  }
}

}  // namespace cmdfuncs

static std::string sCmdline  = "";
static std::string sResponse = "";

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

static std::string* sHistoryPtr = nullptr;

void init(GLFWwindow* window, const char* glslVersion)
{
  initializeImGui(window, glslVersion);
  initPanels();
  sResponseSink->set_pattern("[%l] %v");
  Panel& historyPanel = panelByName("history");
  auto   history      = historyPanel.newWidget<Text>("");
  sHistoryPtr         = &(history->value());

  // Initialize the command map.
  sCommandFnMap.emplace("reload", cmdfuncs::reload);
  sCommandFnMap.emplace("2d", cmdfuncs::twoDMode);
  sCommandFnMap.emplace("3d", cmdfuncs::threeDMode);
  sCommandFnMap.emplace("ze", cmdfuncs::zoomExtents);
  sCommandFnMap.emplace("zoomextents", cmdfuncs::zoomExtents);
  sCommandFnMap.emplace("show", cmdfuncs::show);
  sCommandFnMap.emplace("hide", cmdfuncs::hide);
  sCommandFnMap.emplace("wireframe", cmdfuncs::wireframe);
  sCommandFnMap.emplace("meshedges", cmdfuncs::meshEdges);
  sCommandFnMap.emplace("perspective", cmdfuncs::perspective);
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
  float cmdHeight = sCmdHeight + float(nLines) * 17.f;

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
    runCommand(sCmdline);
    sCmdline.clear();
    sResponse.clear();
  }
  if (sResponseStream.rdbuf()->in_avail() > 0) {
    sResponse = sResponseStream.str();
    *(sHistoryPtr) += sResponse;
    sResponseStream.str("");
  }
  if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
    ImGui::SetKeyboardFocusHere(0);
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

}  // namespace view
}  // namespace gal
