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
}

GAL_FUNC_DEFN(mapValueToColor,
              3,
              1,
              "Maps the given value, w.r.t the given range, to the given color scheme. "
              "And returns the color",
              ((float, value, "value"),
               (glm::vec2, range, "Possible range of values"),
               (std::vector<glm::vec3>, colorScheme, "The scheme of colors")),
              ((glm::vec3, color, "Color")))
{
  const auto& val = *value;
  const auto& rng = *range;
  float       r   = float(colorScheme->size() - 1) *
            std::clamp((val - rng[0]) / (rng[1] - rng[0]), 0.f, 1.f);

  float  fi = std::floor(r);
  size_t i  = size_t(fi);
  size_t j  = size_t(std::ceil(r));
  r -= fi;

  *color = colorScheme->at(i) * (1.f - r) + colorScheme->at(j) * r;
}

}  // namespace func
}  // namespace gal