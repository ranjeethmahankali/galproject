#pragma once

#include <galcore/Util.h>
#include <stdint.h>

namespace gal {
namespace debug {

void initSession(const fs::path& dirpath);
void loadCallstack();

}  // namespace debug
}  // namespace gal