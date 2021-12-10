#include <galview/Command.h>
#include <cstdint>
#include <memory>
#include <sstream>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

namespace gal {
namespace view {

static std::stringstream sHistoryStream;
static auto              sHistorySink =
  std::make_shared<spdlog::sinks::ostream_sink_mt>(sHistoryStream);

static spdlog::logger sLogger = spdlog::logger("cmd", sHistorySink);

void addLogSink(const spdlog::sink_ptr& sink)
{
  sLogger.sinks().push_back(sink);
}

static void runCommandInternal(size_t argc, char** argv)
{
  sLogger.info("Received the following command args:");
  for (size_t i = 0; i < argc; i++) {
    sLogger.info(argv[i]);
  }
}

void runCommand(const std::string& cmd)
{
  static std::string         sParsed;
  static std::vector<size_t> sIndices;
  static std::vector<char*>  sArgV;

  sParsed = cmd;
  sIndices.clear();
  bool inWord = false;
  for (size_t i = 0; i < sParsed.size(); i++) {
    char& c = sParsed[i];
    if (c == ' ') {
      c      = '\0';
      inWord = false;
    }
    else if (!inWord) {
      sIndices.push_back(i);
      inWord = true;
    }
  }
  sArgV.resize(sIndices.size());
  std::transform(sIndices.begin(), sIndices.end(), sArgV.begin(), [](size_t i) {
    return i + sParsed.data();
  });

  runCommandInternal(sArgV.size(), sArgV.data());
}

}  // namespace view
}  // namespace gal
