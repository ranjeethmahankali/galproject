#pragma once

#define GAL_ASSET_DIR std::filesystem::path(GAL_ASSET_DIR_STR)

namespace gal {
namespace test {

/**
 * @brief Initializes the python environment and loads the pygalfunc module, only if it
 * hasn't been done already.
 *
 */
void initPythonEnv();

}  // namespace test
}  // namespace gal
