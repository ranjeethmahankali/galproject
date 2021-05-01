#pragma once

#include <stdint.h>

namespace gal {
namespace debug {

struct DebugGeom
{
  static void initDebugPanel();
  static void draw(uint64_t contextId);
};

}  // namespace debug
}  // namespace gal