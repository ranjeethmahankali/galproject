#include <galcore/Util.h>
#include <cmath>
#include <filesystem>
#ifdef _MSC_VER
#include <Shlwapi.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#define MAX_PATH PATH_MAX
#endif

#define deletePtr(ptr, isArray) \
  if (isArray) {                \
    delete[] arr;               \
  }                             \
  else {                        \
    delete arr;                 \
  }

bool gal::IndexPair::operator==(const gal::IndexPair& pair) const
{
  return (p == pair.p && q == pair.q) || (p == pair.q && q == pair.p);
}

bool gal::IndexPair::operator!=(const gal::IndexPair& pair) const
{
  return (p != pair.q && p != pair.p) || (q != pair.p && q != pair.q);
}

gal::IndexPair::IndexPair(size_t i, size_t j)
    : p(i)
    , q(j)
{}

gal::IndexPair::IndexPair()
    : p(-1)
    , q(-1)
{}

void gal::IndexPair::set(size_t i, size_t j)
{
  p = i;
  q = j;
}

size_t gal::IndexPair::hash() const
{
  return p + q + p * q;
}

void gal::IndexPair::unset(size_t i)
{
  if (p == i) {
    p = -1;
  }
  else if (q == i) {
    q = -1;
  }
}

bool gal::IndexPair::add(size_t i)
{
  if (p == -1) {
    p = i;
    return true;
  }
  else if (q == -1) {
    q = i;
    return true;
  }
  return false;
}

bool gal::IndexPair::contains(size_t i) const
{
  return (i != -1) && (i == p || i == q);
}

bool gal::utils::barycentricWithinBounds(float const (&coords)[3])
{
  return 0 <= coords[0] && coords[0] <= 1 && 0 <= coords[1] && coords[1] <= 1 &&
         0 <= coords[2] && coords[2] <= 1;
}

glm::vec3 gal::utils::barycentricEvaluate(float const (&coords)[3],
                                          glm::vec3 const (&pts)[3])
{
  return pts[0] * coords[0] + pts[1] * coords[1] + pts[2] * coords[2];
}

static int getAbsolutePath(char const* relative, char* absolute, size_t absPathSize)
{
#ifdef _MSC_VER
  HMODULE binary = NULL;
  int     ret    = 0;
  /*First get the handle to this DLL by passing in a pointer to any function inside this
   * dll (cast to a char pointer).*/
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (char*)&get_absolute_path,
                          &binary)) {
    /*If we fail to get this handle, NULL will be used as the module, this means
    GetModuleFileNameA will assume we are asking about the current executable. This is
    harmless for our application because this dll is right next to our application. But
    this will
    cause the unit tests to fail because the executable in that case is not our
    application.*/
    ret = GetLastError();
  }
  GetModuleFileNameA(binary, absolute, (DWORD)absPathSize);
  PathRemoveFileSpecA(absolute);
  PathAppendA(absolute, relative);
  return ret;
#else
  char    result[MAX_PATH];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  auto    path  = std::filesystem::current_path();
  if (count != -1) {
    path = std::filesystem::path(result).parent_path() / std::filesystem::path(relative);
  }
  auto pathstr = path.string();
  std::fill(absolute, absolute + MAX_PATH, '\0');
  std::copy(pathstr.begin(), pathstr.end(), absolute);
  return 0;
#endif
};

/**
 * @brief For the given path relative to the executable, the absolute path is returned.
 * @param relPath Path relative to the executable.
 * @return std::string The absolute path.
 */
std::string gal::utils::absPath(const std::string& relPath)
{
  std::string absPath(MAX_PATH, '\0');
  if (getAbsolutePath(relPath.c_str(), absPath.data(), MAX_PATH) != 0) {
    throw "Cannot find absolute path";
  }
  absPath.erase(std::find(absPath.begin(), absPath.end(), '\0'), absPath.end());
  return absPath;
}