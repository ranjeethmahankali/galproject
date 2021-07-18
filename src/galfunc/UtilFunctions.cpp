#include <galcore/Util.h>
#include <galfunc/UtilFunctions.h>

namespace gal {
namespace func {

GAL_FUNC_DEFN(absPath,
              1,
              1,
              "Gets the absolute path given the path relative to the current binary.",
              ((std::string, relpath, "Path relative to the current binary")),
              ((std::string, apath, "Absolute path output")))
{
  *apath = gal::utils::absPath(*relpath);
};

}  // namespace func
}  // namespace gal