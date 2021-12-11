#include <galview/Command.h>
#include <cstdint>
#include <memory>
#include <sstream>
#include <unordered_map>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <galview/GuiFunctions.h>
#include <galview/Views.h>

namespace gal {
namespace view {

static std::stringstream sHistoryStream;
static auto              sHistorySink =
  std::make_shared<spdlog::sinks::ostream_sink_mt>(sHistoryStream);
static auto     sLogger          = spdlog::stdout_color_mt("galview");
static fs::path sCurrentDemoPath = "";

using CmdFnType = void (*)(int, char**);
static std::unordered_map<std::string, CmdFnType> sCommandFnMap;

spdlog::logger& logger()
{
  return *sLogger;
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
  view::Context::get().set2dMode(true);
}

void threeDMode(int, char**)
{
  view::Context::get().set2dMode(false);
}

void zoomExtents(int, char**)
{
  view::Context::get().zoomExtents();
}

void show(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("show commands expects the name of the panel as an argument.");
  }
  viewfunc::setPanelVisibility(argv[1], true);
}

void hide(int argc, char** argv)
{
  if (argc != 2) {
    logger().error("hide commands expects the name of the panel as an argument.");
  }
  viewfunc::setPanelVisibility(argv[1], false);
}

}  // namespace cmdfuncs

void addLogSink(const spdlog::sink_ptr& sink)
{
  logger().sinks().push_back(sink);
}

void setDemoFilepath(const fs::path& path)
{
  sCurrentDemoPath = path;
}

void initCommands()
{
  logger().sinks().push_back(sHistorySink);
  sCommandFnMap.emplace("reload", cmdfuncs::reload);
  sCommandFnMap.emplace("2d", cmdfuncs::twoDMode);
  sCommandFnMap.emplace("3d", cmdfuncs::threeDMode);
  sCommandFnMap.emplace("ze", cmdfuncs::zoomExtents);
  sCommandFnMap.emplace("show", cmdfuncs::show);
  sCommandFnMap.emplace("hide", cmdfuncs::hide);
}

int runPythonDemoFile(const fs::path& demoPath)
{
  try {
    view::setDemoFilepath(demoPath);
    boost::python::dict global;
    global["__file__"] = demoPath.string();
    global["__name__"] = "__main__";
    boost::python::exec_file(demoPath.c_str(), global);
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

  match->second(int(sArgV.size()), sArgV.data());
}

}  // namespace view
}  // namespace gal
