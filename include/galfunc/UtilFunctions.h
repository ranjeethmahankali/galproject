#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

GAL_FUNC_DECL(((std::string, path, "Absolute path")),
              absPath,
              true,
              1,
              "Gets the absolute path given the path relative to the current binary.",
              (std::string, relpath, "Path relative to the current binary"));

}  // namespace func
}  // namespace gal

// These are all the functions exposed from this translation unit.
#define GAL_UtilFunctions absPath
