#pragma once

#include <filesystem>
#include <string>

#include <imgui.h>

namespace gal {
namespace view {

/**
 * @brief Initialize the command interface.
 */
void initCommands();
/**
 * @brief Autocomplete the given command from known commands.
 *
 * @param cmd Incomplete commands.
 * @param charsToInsert Characters to insert to complate the command.
 */
void autocompleteCommand(const std::string& cmd, std::string& charsToInsert);
/**
 * @brief Queue commands to be run later.
 *
 * @param cmd Command(s) to be queued. If queueing more than one command, they must be
 * separated by a newline character.
 */
void queueCommands(std::string_view cmd);
/**
 * @brief Run all commands that have been queued so far and clear the queue.
 */
void runQueuedCommands();
/**
 * @brief Get a reference to the file path of the current demo.
 */
std::filesystem::path& demoFilePath();

}  // namespace view
}  // namespace gal
