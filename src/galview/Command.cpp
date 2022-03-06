#include <spdlog/logger.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <galview/Command.h>
#include <galview/Context.h>
#include <galview/GuiFunctions.h>
#include <galview/Views.h>
#include <algorithm>
#include <cxxopts.hpp>
#include <initializer_list>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace gal {
namespace view {

using CmdFnPtr = void (*)(int, char**);

std::vector<std::string>& cmdQueue()
{
  static std::vector<std::string> sQueue;
  return sQueue;
}

std::unordered_map<std::string, CmdFnPtr>& cmdFuncMap()
{
  static std::unordered_map<std::string, CmdFnPtr> sCommandFnMap;
  return sCommandFnMap;
}

fs::path& currentDemoPath()
{
  static fs::path sCurrentDemoPath = "";
  return sCurrentDemoPath;
}

static void runCommand(const std::string& cmd)
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
  auto        match   = cmdFuncMap().find(command);
  if (match == cmdFuncMap().end()) {
    logger().error("Unrecognized command {}", command);
    return;
  }
  view::logger().info(">>> {}", cmd);
  match->second(int(sArgV.size()), sArgV.data());
}

void runQueuedCommands()
{
  for (const auto& cmd : cmdQueue()) {
    runCommand(cmd);
  }
  cmdQueue().clear();
}

void autocompleteCommand(const std::string& cmd, std::string& charsToInsert)
{
  static std::string sSuggestions = "";
  charsToInsert.clear();
  sSuggestions.clear();
  for (const auto& pair : cmdFuncMap()) {
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

void queueCommand(const std::string& cmd)
{
  cmdQueue().push_back(cmd);
}

struct ArgDesc
{
  std::string_view name;
  std::string_view desc;
};

template<size_t I, typename... Ts>
void addOptions(cxxopts::OptionAdder& adder, const std::initializer_list<ArgDesc>& args)
{
  using Type = std::tuple_element_t<I, std::tuple<Ts...>>;

  if constexpr (I < sizeof...(Ts)) {
    const auto& arg = *(args.begin() + I);
    adder(arg.name.data(), arg.desc.data(), cxxopts::value<Type>(), arg.name.data());
  }

  if constexpr (I + 1 < sizeof...(Ts)) {
    addOptions<I + 1, Ts...>(adder, args);
  }
}

cxxopts::Options optionsWithoutArgs(const std::string_view& name,
                                    const std::string_view& desc)
{
  cxxopts::Options opts(name.data(), desc.data());
  opts.allow_unrecognised_options().add_options()("help", "Print help");
  return opts;
}

template<typename... Ts>
cxxopts::Options optionsWithPosnArgs(const std::string_view&               name,
                                     const std::string_view&               desc,
                                     const std::initializer_list<ArgDesc>& args)
{
  if (!(sizeof...(Ts) == 1 || args.size() == sizeof...(Ts))) {
    logger().error("Incorrect number of argument types for the options parser.");
  }
  auto opts  = optionsWithoutArgs(name, desc);
  auto adder = opts.add_options();
  addOptions<0, Ts...>(adder, args);
  std::vector<std::string> argNames(args.size());
  std::transform(args.begin(), args.end(), argNames.begin(), [](const auto& pair) {
    return pair.name;
  });
  opts.parse_positional(argNames);
  return opts;
}

std::optional<cxxopts::ParseResult> parseOptions(int               argc,
                                                 char**            argv,
                                                 cxxopts::Options& opts)
{
  auto parsed = opts.parse(argc, argv);
  if (parsed.count("help")) {
    logger().info(opts.help());
    return std::nullopt;
  }
  return parsed;
}

namespace cmdfuncs {

void reload(int argc, char** argv)
{
  static auto opts   = optionsWithoutArgs("reload", "Reloads the current demo file.");
  auto        parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }

  gal::viewfunc::unloadAllOutputs();
  gal::func::store::unloadAllFunctions();
  gal::view::Views::clear();
  int err = runPythonDemoFile(demoFilePath());
  if (err != 0) {
    logger().error("Unable to run the demo file. Aborting...\n");
    std::exit(err);
  }
}

void twoDMode(int argc, char** argv)
{
  static auto opts = optionsWithoutArgs("twoDMode", "Switches to a 2d top-down viewer.");
  auto        parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }
  view::logger().info("Enabling 2d viewer mode...");
  view::Context::get().set2dMode(true);
}

void threeDMode(int argc, char** argv)
{
  static auto opts   = optionsWithoutArgs("threeDMode", "Switches to a 3d viewer.");
  auto        parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }
  view::logger().info("Enabling 3d viewer mode...");
  view::Context::get().set2dMode(false);
}

void zoomExtents(int argc, char** argv)
{
  static auto opts =
    optionsWithoutArgs("zoomextents", "Zooms to the extents of all visible objects.");
  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }
  view::logger().info("Zooming to extents...");
  view::Context::get().zoomExtents();
}

void show(int argc, char** argv)
{
  static auto opts =
    optionsWithPosnArgs<std::string>("show",
                                     "Sets the visibility of a panel to true.",
                                     {{"panel", "The name of the panel"}});
  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }
  setPanelVisibility(parsed.value()["panel"].as<std::string>(), true);
}

void hide(int argc, char** argv)
{
  static auto opts =
    optionsWithPosnArgs<std::string>("hide",
                                     "Sets the visibility of a panel to false.",
                                     {{"panel", "The name of the panel"}});
  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }
  setPanelVisibility(parsed.value()["panel"].as<std::string>(), false);
}

void wireframe(int argc, char** argv)
{
  static auto opts = optionsWithPosnArgs<std::string>(
    "wireframe", "Turn the wireframe mode on or off.", {{"flag", "on or off."}});
  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }

  bool               bflag = false;
  const std::string& flag  = parsed.value()["flag"].as<std::string>();
  if (flag == "on") {
    bflag = true;
  }
  else if (parsed.value()["flag"].as<std::string>() == "off") {
    bflag = false;
  }
  else {
    logger().error(
      "Unrecognized option {} for the wireframe command. Expected either on or off.",
      flag);
    return;
  }
  Context::get().setWireframeMode(bflag);
  logger().info("Wireframe mode flag set to {}.", flag);
}

void meshEdges(int argc, char** argv)
{
  static auto opts = optionsWithPosnArgs<std::string>(
    "meshedges",
    "Sets the visbility of mesh edges.",
    {{"flag", "on or off indicating whether the mesh edges shoule be visible."}});
  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }

  bool               bflag = false;
  const std::string& flag  = parsed.value()["flag"].as<std::string>();
  if (flag == "on") {
    bflag = true;
  }
  else if (parsed.value()["flag"].as<std::string>() == "off") {
    bflag = false;
  }
  else {
    logger().error(
      "Unrecognized option {} for the wireframe command. Expected either on or off.",
      flag);
    return;
  }
  Context::get().setMeshEdgeMode(bflag);
  logger().info("Mesh edge visibility flag set to {}.", flag);
}

void perspective(int argc, char** argv)
{
  static auto opts =
    optionsWithPosnArgs<std::string>("perspective",
                                     "Sets the perspective camera to on or off.",
                                     {{"flag",
                                       "on or off flag indicating whether the camera "
                                       "should use perspective projection or not."}});
  auto parsed = parseOptions(argc, argv, opts);
  if (!parsed) {
    return;
  }
  if (argc != 2) {
    logger().error("perspective command expects an on/off argument.");
    return;
  }

  const std::string& flag = parsed.value()["flag"].as<std::string>();
  if (flag == "on") {
    Context::get().setPerspective();
    logger().info("Now using a perspective camera.");
  }
  else if (flag == "off") {
    Context::get().setOrthographic();
    logger().info("Now using an orthographic camera.");
  }
  else {
    logger().error(
      "Unrecorgnized option {} for the perspective command. Expected either on or off.",
      flag);
    return;
  }
}

void help(int argc, char** argv0)
{
  static auto opts =
    optionsWithoutArgs("help", "Shows the help info for all available commands");
  auto parsed = parseOptions(argc, argv0, opts);
  if (!parsed) {
    return;
  }
  static std::array<std::string, 2>    sArgs = {"", "--help"};
  static std::stringstream             sTempStream;
  static std::vector<spdlog::sink_ptr> sTempSinks = {
    std::make_shared<spdlog::sinks::ostream_sink_mt>(sTempStream)};
  sTempSinks[0]->set_pattern("%v");

  sTempStream.str("");
  std::swap(logger().sinks(), sTempSinks);
  for (const auto& pair : cmdFuncMap()) {
    sArgs[0] = pair.first;
    std::array<char*, 2> argv1;
    std::transform(
      sArgs.begin(), sArgs.end(), argv1.begin(), [](std::string& s) { return s.data(); });
    logger().info("\n{}", pair.first);
    pair.second(int(argv1.size()), argv1.data());
  }
  std::swap(logger().sinks(), sTempSinks);

  logger().info("All avaliable commands: ");
  logger().info(sTempStream.str());
}

}  // namespace cmdfuncs

void setDemoFilepath(const fs::path& path)
{
  currentDemoPath() = path;
}

const std::filesystem::path& demoFilePath()
{
  return currentDemoPath();
}

void initCommands()
{
  cmdFuncMap().emplace("reload", cmdfuncs::reload);
  cmdFuncMap().emplace("2d", cmdfuncs::twoDMode);
  cmdFuncMap().emplace("3d", cmdfuncs::threeDMode);
  cmdFuncMap().emplace("ze", cmdfuncs::zoomExtents);
  cmdFuncMap().emplace("zoomextents", cmdfuncs::zoomExtents);
  cmdFuncMap().emplace("show", cmdfuncs::show);
  cmdFuncMap().emplace("hide", cmdfuncs::hide);
  cmdFuncMap().emplace("wireframe", cmdfuncs::wireframe);
  cmdFuncMap().emplace("meshedges", cmdfuncs::meshEdges);
  cmdFuncMap().emplace("perspective", cmdfuncs::perspective);
  cmdFuncMap().emplace("help", cmdfuncs::help);
}

}  // namespace view
}  // namespace gal
