#include <galcore/Util.h>
#include <galfunc/UtilFunctions.h>


namespace gal {
namespace func {

GAL_FUNC_DEFN(((std::string, path, "Absolute path")),
             absPath,
             true,
             1,
             "Gets the absolute path given the path relative to the current binary.",
             (std::string, relpath, "Path relative to the current binary"))
{
  return std::make_tuple(std::make_shared<std::string>(gal::utils::absPath(*relpath)));
};

}  // namespace func
}  // namespace gal