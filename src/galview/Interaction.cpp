#include <assert.h>
#include <algorithm>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <imgui.h>
#include <imnodes.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/adaptors.hpp>

#include <galcore/Util.h>
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
static func::graph::Graph sGraph;

struct NodeProps
{
  const func::Function* func = nullptr;
  glm::vec2             pos;
  int                   depth      = -1;
  int                   height     = -1;
  int                   col        = -1;
  int                   row        = -1;
  float                 innerWidth = 0.f;

  /**
   * @brief Gets the span (i.e. sum of depth and height) of the node.
   *
   * @return int Span if height and depth are assigned, -1 otherwise.
   */
  int span() const { return (depth == -1 || height == -1) ? -1 : (depth + height); }
};

struct PinProps
{
  glm::vec2 pos;
};

func::Property<NodeProps>& nodeProps()
{
  static func::Property<NodeProps> sProp = sGraph.addNodeProperty<NodeProps>();
  return sProp;
}

func::Property<int>& funcNodeIndices()
{
  static func::Property<int> sFuncNodeIndices = func::store::addProperty<int>();
  return sFuncNodeIndices;
}

func::Property<PinProps>& pinProps()
{
  static func::Property<PinProps> sPinProps = sGraph.addPinProperty<PinProps>();
  return sPinProps;
}

static constexpr int imNodeId(int i)
{
  return 0xfe783b1e ^ i;
}

static constexpr int imPinId(int i)
{
  return 0x14b907bf ^ i;
}

static constexpr int imLinkId(int i)
{
  return 0x876836b5 ^ i;
}

static void initializeImGui(GLFWwindow* window, const char* glslVersion)
{
  glutil::logger().info("Setting up ImGui...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImNodes::CreateContext();
  ImGuiIO&    io      = ImGui::GetIO();
  std::string absPath = utils::absPath("CascadiaMono.ttf");
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

  // ImNodes customizations.
  ImNodesStyle& nstyle = ImNodes::GetStyle();
  nstyle.Flags &= ~ImNodesStyleFlags_GridLines;
  nstyle.Flags &= ~ImNodesStyleFlags_NodeOutline;
  nstyle.Colors[ImNodesCol_GridBackground] = IM_COL32(0, 0, 0, 0);
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

template<bool UpdatePass = false>
void drawCanvas()
{
  ImNodes::BeginNodeEditor();
  int         count  = int(sGraph.numNodes());
  const auto& nprops = nodeProps();
  auto&       pprops = pinProps();
  for (int ni = 0; ni < count; ni++) {
    const auto& ndata = nprops[ni];
    const auto& info  = ndata.func->info();
    ImNodes::BeginNode(imNodeId(ni));
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(info.mName.data());
    ImNodes::EndNodeTitleBar();

    int i = 0;
    for (int pi : sGraph.nodeInputs(ni)) {
      ImNodes::BeginInputAttribute(imPinId(pi));
      ImGui::Text("%s", info.mInputNames[i].data());
      ImNodes::EndInputAttribute();
      if constexpr (UpdatePass) {
        pprops[pi].pos =
          0.5f * (toGlm(ImGui::GetItemRectMin()) + toGlm(ImGui::GetItemRectMax()));
      }
      i++;
    }

    i = 0;
    for (int pi : sGraph.nodeOutputs(ni)) {
      ImNodes::BeginOutputAttribute(imPinId(pi));
      ImGui::Indent(ndata.innerWidth -
                    ImGui::CalcTextSize(info.mOutputNames[i].data()).x);
      ImGui::Text("%s", info.mOutputNames[i].data());
      ImNodes::EndOutputAttribute();
      if constexpr (UpdatePass) {
        pprops[pi].pos =
          0.5f * (toGlm(ImGui::GetItemRectMin()) + toGlm(ImGui::GetItemRectMax()));
      }
      i++;
    }
    ImNodes::EndNode();
  }
  count = int(sGraph.numLinks());
  for (int i = 0; i < count; i++) {
    ImNodes::Link(imLinkId(i), imPinId(sGraph.linkStart(i)), imPinId(sGraph.linkEnd(i)));
  }
  ImNodes::EndNodeEditor();

  if constexpr (UpdatePass) {
    // TODO: Update positions with iterative optimization.
  }
}

static void initPanels()
{
  sPanels.clear();
  sPanels.emplace_back("inputs");
  sPanels.emplace_back("outputs");

  sPanels.emplace_back("canvas", false);
  Panel& canvas = sPanels.back();
  canvas.addCallback(&drawCanvas<false>);

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

static void calcDepthsAndHeights()
{
  auto& g      = sGraph;
  auto& nprops = nodeProps();
  struct Candidate
  {
    int node;
    int value;
  };
  std::vector<Candidate> q;
  q.reserve(g.numNodes());
  int nNodes = int(g.numNodes());

  // Heights.
  for (int i = 0; i < nNodes; i++) {
    if (!g.nodeHasOutputs(i)) {
      q.push_back(Candidate {i, 0});
    }
  }
  while (!q.empty()) {
    Candidate c = q.back();
    q.pop_back();
    int& height = nprops[c.node].height;
    height      = std::max(c.value, height);
    int h2      = height + 1;
    for (int pi : g.nodeInputs(c.node)) {
      auto plinks = g.pinLinks(pi);
      std::transform(plinks.begin(), plinks.end(), std::back_inserter(q), [&](int li) {
        return Candidate {g.pinNode(g.linkStart(li)), h2};
      });
    }
  }

  // Depths.
  for (int i = 0; i < nNodes; i++) {
    if (!g.nodeHasInputs(i)) {
      q.push_back(Candidate {i, 0});
    }
  }
  while (!q.empty()) {
    Candidate c = q.back();
    q.pop_back();
    int& depth = nprops[c.node].depth;
    depth      = std::max(c.value, depth);
    int d2     = depth + 1;
    for (int pi : g.nodeOutputs(c.node)) {
      auto plinks = g.pinLinks(pi);
      std::transform(plinks.begin(), plinks.end(), std::back_inserter(q), [&](int li) {
        return Candidate {g.pinNode(g.linkEnd(li)), d2};
      });
    }
  }
}

static void buildGraph()
{
  auto& g         = sGraph;
  auto& fnNodeIdx = funcNodeIndices();
  auto& nprops    = nodeProps();
  g.clear();
  // Reserve memory.
  size_t nfunc    = func::store::numFunctions();
  size_t nInputs  = 0;
  size_t nOutputs = 0;
  for (size_t i = 0; i < nfunc; i++) {
    const auto& f = func::store::function(i);
    nInputs += f.numInputs();
    nOutputs = f.numOutputs();
  }
  g.reserve(nfunc, nInputs + nOutputs, nInputs);
  // Add nodes.
  std::vector<func::InputInfo> inputs;
  for (size_t i = 0; i < nfunc; i++) {
    const auto& f   = func::store::function(i);
    int         ni  = g.addNode(f.numInputs(), f.numOutputs());
    fnNodeIdx[f]    = ni;
    nprops[ni].func = &f;
  }
  // Add links.
  for (size_t i = 0; i < nfunc; i++) {
    const auto& f = func::store::function(i);
    f.getInputs(inputs);
    int fni = fnNodeIdx[f];
    int end = g.nodeInput(fni);
    for (const auto& input : inputs) {
      int start = g.nodeOutput(fnNodeIdx[*(input.mFunc)], input.mOutputIdx);
      g.addLink(start, end);
      end = g.pinNext(end);
    }
  }
}

static void calcColumns()
{
  namespace ba = boost::adaptors;
  using namespace gal::utils;
  using IntIter = boost::counting_iterator<int>;

  struct Candidate
  {
    int node;
    int value;
  };

  std::vector<Candidate> nodes;  // Queue for processing all nodes.

  auto&       nprops = nodeProps();
  const auto& g      = sGraph;
  assert(g.numNodes() == nprops.size());
  int nNodes = int(g.numNodes());

  int maxt = 0;
  for (int ni = 0; ni < nNodes; ni++) {
    maxt = std::max(maxt, nprops[ni].span());
  }
  auto pushUpstream = [&](int ni, int val) {
    for (int pi : g.nodeInputs(ni)) {
      auto plinks = g.pinLinks(pi);
      std::transform(
        plinks.begin(), plinks.end(), std::back_inserter(nodes), [&](int li) {
          return Candidate {g.pinNode(g.linkStart(li)), val};
        });
    }
  };
  auto pushDownstream = [&](int ni, int val) {
    for (int pi : g.nodeOutputs(ni)) {
      auto plinks = g.pinLinks(pi);
      std::transform(
        plinks.begin(), plinks.end(), std::back_inserter(nodes), [&](int li) {
          return Candidate {g.pinNode(g.linkEnd(li)), val};
        });
    }
  };
  auto assignCol = [&](Candidate c) -> int {
    int& col = nprops[c.node].col;
    if (c.value < 0) {
      col = col == -1 ? maxt + c.value : std::min(maxt + c.value, col);
    }
    else {
      col = std::max(c.value, col);
    }
    return col;
  };

  std::vector<int> seeds = utils::makeVector<int>(
    Span<IntIter>(IntIter(0), IntIter(nNodes)) |
    ba::filtered([&](int ni) { return nprops[ni].span() == maxt; }));

  for (int s : seeds) {
    int& col = nprops[s].col;
    col      = nprops[s].depth;
    pushUpstream(s, col - maxt - 1);
  }
  while (!nodes.empty()) {
    Candidate c = nodes.back();
    nodes.pop_back();
    int col = assignCol(c);
    // Push upstream candidates with negative offsets measured from right.
    pushUpstream(c.node, col - maxt - 1);
  }

  for (int s : seeds) {
    pushDownstream(s, nprops[s].col + 1);
  }
  while (!nodes.empty()) {
    Candidate c = nodes.back();
    nodes.pop_back();
    int col = assignCol(c);
    // Push downstream candidaets with positive offsets measured from left.
    pushDownstream(c.node, col + 1);
  }
}

static void calcRows()
{
  auto&       nprops = nodeProps();
  const auto& g      = sGraph;
  assert(g.numNodes() == nprops.size());
  int nNodes = int(g.numNodes());

  std::vector<int> nodes(boost::counting_iterator<int>(0),
                         boost::counting_iterator<int>(nNodes));
  std::sort(std::execution::par, nodes.begin(), nodes.end(), [&](int a, int b) {
    return nprops[a].col < nprops[b].col;
  });
  std::vector<float> scores(nodes.size());
  int                col      = 0;
  auto               colbegin = std::find_if(
    nodes.begin(), nodes.end(), [&](int ni) { return nprops[ni].col > col; });
  for (auto it = nodes.begin(); it != colbegin; it++) {
    nprops[*it].row = int(std::distance(nodes.begin(), it));
  }
  while (colbegin != nodes.end()) {
    col++;
    auto colend = std::find_if(
      nodes.begin(), nodes.end(), [&](int ni) { return nprops[ni].col > col; });
    for (auto it = colbegin; it != colend; it++) {
      int      ni     = *it;
      auto&    score  = scores[ni];
      uint32_t nlinks = 0;
      for (int pi : g.nodeInputs(ni)) {
        for (int li : g.pinLinks(pi)) {
          int uni = g.pinNode(g.linkStart(li));
          score += float(nprops[uni].row);
          nlinks++;
        }
      }
      score /= float(nlinks);
    }
    std::sort(colbegin, colend, [&](int a, int b) { return scores[a] < scores[b]; });
    for (auto it = colbegin; it != colend; it++) {
      nprops[*it].row = std::distance(colbegin, it);
    }
    colbegin = colend;
  }
}

static void updateNodePositions()
{
  static constexpr float yOffset = 25.f;
  static constexpr float xOffset = 25.f;
  const auto&            nprops  = nodeProps();
  for (size_t i = 0; i < nprops.size(); i++) {
    ImNodes::SetNodeScreenSpacePos(
      imNodeId(i), ImVec2(xOffset + nprops[i].pos.x, yOffset + nprops[i].pos.y));
  }
}

static void updateCanvas()
{
  using namespace gal::func::graph;
  // Build graph and compute layout.
  buildGraph();
  calcDepthsAndHeights();
  calcColumns();
  calcRows();

  imGuiNewFrame();
  ImGui::PushFont(sFont);

  auto& nprops = nodeProps();
  for (int ni = 0; ni < sGraph.numNodes(); ni++) {
    auto&       ndata  = nprops[ni];
    int         nodeId = imNodeId(ni);
    const auto& info   = ndata.func->info();

    ndata.innerWidth = ImGui::CalcTextSize(info.mName.data()).x;
    for (size_t ii = 0; ii < info.mNumInputs; ii++) {
      ndata.innerWidth =
        std::max(ndata.innerWidth, ImGui::CalcTextSize(info.mInputNames[ii].data()).x);
    }
    for (size_t oi = 0; oi < info.mNumOutputs; oi++) {
      ndata.innerWidth =
        std::max(ndata.innerWidth, ImGui::CalcTextSize(info.mOutputNames[oi].data()).x);
    }
    nprops[ni].pos = {200.f * float(ndata.col), 100.f * float(ndata.row)};
  }
  updateNodePositions();

  drawCanvas<true>();
  ImGui::PopFont();
  ImGui::EndFrame();
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

static std::string sCmdline  = "";
static std::string sResponse = "";

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
    queueCommand(sCmdline);
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

void destroy(GLFWwindow* window)
{
  glutil::logger().info("Cleaning up...\n");
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImNodes::DestroyContext();
  ImGui::DestroyContext();
  if (window) {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
}

}  // namespace view
}  // namespace gal
