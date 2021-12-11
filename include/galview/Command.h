#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <filesystem>
#include <optional>
#include <string>

namespace gal {
namespace view {

spdlog::logger& logger();
void            initCommands();
void            autocompleteCommand(const std::string& cmd, std::string& charsToInsert);
int             runPythonDemoFile(const std::filesystem::path& demoPath);
void            setDemoFilepath(const std::filesystem::path& path);
void            addLogSink(const spdlog::sink_ptr& sink);
void            runCommand(const std::string& cmd);

}  // namespace view
}  // namespace gal
