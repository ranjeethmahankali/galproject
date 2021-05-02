#pragma once

#include <galcore/Util.h>
#include <stdint.h>

namespace gal {
namespace debug {

void initSession(const fs::path& dirpath);
void loadCallstack();
void clearCallstack();

}  // namespace debug
}  // namespace gal