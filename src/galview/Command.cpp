#include <galview/Command.h>
#include <cstdint>
#include <memory>
#include <sstream>
#include <unordered_map>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <galview/GuiFunctions.h>
#include <galview/Views.h>

namespace gal {
namespace view {

static std::stringstream sHistoryStream;
static auto              sHistorySink =
  std::make_shared<spdlog::sinks::ostream_sink_mt>(sHistoryStream);
static spdlog::logger sLogger          = spdlog::logger("cmd", sHistorySink);
static fs::path       sCurrentDemoPath = "";

using CmdFnType = void (*)(int, char**);
static std::unordered_map<std::string, CmdFnType> sCommandFnMap;

namespace cmdfuncs {

void reload(int, char**)
{
  gal::viewfunc::unloadAllOutputs();
  gal::func::store::unloadAllFunctions();
  gal::view::Views::clear();
  int err = runPythonDemoFile(sCurrentDemoPath);
  if (err != 0) {
    std::cerr << "Unable to run the demo file. Aborting...\n";
    std::exit(err);
  }
}

}  // namespace cmdfuncs

void addLogSink(const spdlog::sink_ptr& sink)
{
  sLogger.sinks().push_back(sink);
}

void setDemoFilepath(const fs::path& path)
{
  sCurrentDemoPath = path;
}

void initCommands()
{
  sCommandFnMap.emplace("reload", cmdfuncs::reload);
}

int runPythonDemoFile(const fs::path& demoPath)
{
  try {
    std::cout << "Running demo file: " << demoPath << std::endl;
    view::setDemoFilepath(demoPath);
    boost::python::dict global;
    global["__file__"] = demoPath.string();
    global["__name__"] = "__main__";
    boost::python::exec_file(demoPath.c_str(), global);
    return 0;
  }
  catch (boost::python::error_already_set) {
    PyErr_Print();
    std::cerr << "Unable to load the demo... aborting...\n";
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

  std::string command = sArgV[0];
  auto        match   = sCommandFnMap.find(command);
  if (match == sCommandFnMap.end()) {
    sLogger.error("Unrecognized command {}", command);
    return;
  }

  match->second(int(sArgV.size()), sArgV.data());
}

}  // namespace view
}  // namespace gal
