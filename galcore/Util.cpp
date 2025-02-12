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

namespace gal {
namespace utils {

spdlog::logger& logger()
{
  static auto sLogger = spdlog::stdout_color_mt("core");
  return *sLogger;
}

/**
 * @brief For the given path relative to the executable, the absolute path is returned.
 * @param relPath Path relative to the executable.
 * @return std::string The absolute path.
 */
fs::path absPath(const fs::path& relPath)
{
  std::string apath(MAX_PATH, '\0');
#ifdef _MSC_VER
  HMODULE binary = NULL;
  int     ret    = 0;
  /*First get the handle to this DLL by passing in a pointer to any function inside this
   * dll (cast to a char pointer).*/
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (char*)&absPath,
                          &binary)) {
    /*If we fail to get this handle, NULL will be used as the module, this means
    GetModuleFileNameA will assume we are asking about the current executable. This is
    harmless for our application because this dll is right next to our application. But
    this will
    cause the unit tests to fail because the executable in that case is not our
    application.*/
    ret = GetLastError();
  }
  GetModuleFileNameA(binary, apath.data(), (DWORD)MAX_PATH);
#else
  size_t count = readlink("/proc/self/exe", apath.data(), PATH_MAX);
  if (count == -1) {
    throw "Cannot find absolute path";
  }
  apath.erase(apath.begin() + int64_t(count), apath.end());
#endif
  return fs::path(apath).parent_path() / relPath;
}

int bitscanForward(uint32_t i)
{
#ifdef _MSC_VER
  unsigned long index = 0;  // This is the same as unsigned long.
  if (_BitScanForward(&index, i)) {
    return int(index - 1);
  }
  else {
    return -1;
  }
#else
  if (i) {
    return __builtin_ctz(i);
  }
  else {
    return -1;
  }
#endif
}

int bitscanForward(uint64_t i)
{
#ifdef _MSC_VER
  unsigned long index = 0;
  if (_BitScanForward64(&index, i)) {
    return int(index);
  }
  else {
    return -1;
  }
#else
  if (i) {
    return __builtin_ctzl(i);
  }
  else {
    return -1;
  }
#endif
}

size_t numCombinations(size_t n, size_t k)
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

}  // namespace utils
}  // namespace gal
