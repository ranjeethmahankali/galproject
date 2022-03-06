#include <filesystem>
#include <string>

#include <imgui.h>

namespace gal {
namespace view {

void autocompleteCommand(const std::string& cmd, std::string& charsToInsert);
void initCommands();

void                         setDemoFilepath(const std::filesystem::path& path);
const std::filesystem::path& demoFilePath();

}  // namespace view
}  // namespace gal
