#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _DEBUG
#define GL_CALL(fncall)                                       \
  {                                                           \
    gal::glutil::clear_errors();                              \
    fncall;                                                   \
    if (gal::glutil::log_errors(#fncall, __FILE__, __LINE__)) \
      __debugbreak();                                         \
  }
#else
#define GL_CALL(fncall) fncall
#endif  // DEBUG

using uint = GLuint;

namespace gal {
namespace glutil {

static bool log_errors(const char* function, const char* file, uint line);
static void clear_errors();

};  // namespace glutil
}  // namespace gal
