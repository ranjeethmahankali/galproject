#include <stdexcept>

#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/TypeHelper.h>

namespace gal {
namespace func {

GAL_FUNC(absPath,
         "Gets the absolute path given the path relative to the current binary.",
         ((std::string, relpath, "Path relative to the current binary")),
         ((std::string, apath, "Absolute path output")))
{
  apath = gal::utils::absPath(relpath);
}

GAL_FUNC_TEMPLATE(((typename, T)),
                  toString,
                  "Converts the input data to string, if such a conversion is supported.",
                  ((T, src, "The source data")),
                  ((std::string, result, "String result")))
{
  result = std::to_string(src);
}

void bind_UtilFunctions()
{
  GAL_FN_BIND(absPath);
  GAL_FN_BIND_TEMPLATE(toString, float);
  GAL_FN_BIND_TEMPLATE(toString, int32_t);
}

}  // namespace func
}  // namespace gal
