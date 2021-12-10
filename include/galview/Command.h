#include <spdlog/common.h>
#include <string>

namespace gal {
namespace view {

void addLogSink(const spdlog::sink_ptr& sink);
void runCommand(const std::string& cmd);

}  // namespace view
}  // namespace gal
