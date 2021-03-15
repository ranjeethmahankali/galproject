#include <galview/GLUtil.h>
#include <iostream>

bool gal::glutil::log_errors(const char* function, const char* file, uint line)
{
  static bool found_error = false;
  while (GLenum error = glGetError()) {
    std::cout << "[OpenGL Error] (0x" << std::hex << error << std::dec << ")";
#ifdef _DEBUG
    std::cout << " in " << function << " at " << file << ":" << line;
#endif  // NDEBUG
    std::cout << std::endl;
    found_error = true;
  }
  if (found_error) {
    found_error = false;
    return true;
  }
  else {
    return false;
  }
};

void gal::glutil::clear_errors()
{
  // Just loop over and consume all pending errors.
  GLenum error = glGetError();
  while (error) {
    error = glGetError();
  }
};