#include <spdlog/common.h>
#include <filesystem>
#include <string>

namespace gal {
namespace view {

void initCommands();
int  runPythonDemoFile(const std::filesystem::path& demoPath);
void setDemoFilepath(const std::filesystem::path& path);
void addLogSink(const spdlog::sink_ptr& sink);
void runCommand(const std::string& cmd);

}  // namespace view
}  // namespace gal
