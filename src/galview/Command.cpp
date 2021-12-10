#include <galview/Command.h>
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

void runCommand(const std::string& cmd)
{
  sLogger.info("Received command: {}", cmd);
}

}  // namespace view
}  // namespace gal
