#include <galfunc/Functions.h>

namespace gal {
namespace func {

static auto fn =
  Functor<uint32_t, int, int>([](int x, int y) -> uint32_t { return uint32_t(x + y); });

}  // namespace func
}  // namespace gal