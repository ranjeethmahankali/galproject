#include <cmath>
#include <filesystem>
#include <string>

#ifdef _MSC_VER
#include <Shlwapi.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#define MAX_PATH PATH_MAX
#endif

#include <spdlog/sinks/stdout_color_sinks.h>

#include <Util.h>

#define deletePtr(ptr, isArray) \
  if (isArray) {                \
    delete[] arr;               \
  }                             \
  else {                        \
    delete arr;                 \
  }

namespace fs = std::filesystem;

spdlog::logger& gal::utils::logger()
{
  static auto sLogger = spdlog::stdout_color_mt("core");
  return *sLogger;
}

/**
 * @brief For the given path relative to the executable, the absolute path is returned.
 * @param relPath Path relative to the executable.
 * @return std::string The absolute path.
 */
fs::path gal::utils::absPath(const fs::path& relPath)
{
  std::string apath(MAX_PATH, '\0');
  ssize_t     count = readlink("/proc/self/exe", apath.data(), PATH_MAX);
  fs::path    path;
  if (count == -1) {
    throw "Cannot find absolute path";
  }
  apath.erase(apath.begin() + count, apath.end());
  path = fs::path(apath).parent_path() / relPath;
  return path;
}

size_t gal::utils::numCombinations(size_t n, size_t k)
{
  if (k > n) {
    return 0;
  }
  if (k * 2 > n) {
    k = n - k;
  }
  if (k == 0) {
    return 1;
  }
  size_t c = n;
  for (size_t i = 2; i < k + 1; i++) {
    c *= n - i + 1;
    c /= i;
  }
  return c;
}
