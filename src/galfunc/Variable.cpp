#include <galfunc/Variable.h>

namespace gal {
namespace func {

template<typename T, typename... Ts>
struct ReaderImpl
{
  static bool read(const store::Register& reg, boost::python::object& dst)
  {
    if (TypeInfo<T>::id == reg.typeId) {
      auto data = gal::func::store::get<T>(reg.id);
      Converter<T, boost::python::object>::assign(*data, dst);
      return true;
    }

    if constexpr (sizeof...(Ts) > 0) {
      return ReaderImpl<Ts...>::read(reg, dst);
    }
    else {
      return false;
    }
  }
};

using Reader = ReaderImpl<float, int32_t, std::string, glm::vec3, glm::vec2>;

boost::python::object py_read(store::Register reg)
{
  boost::python::object dst;
  bool                  success = Reader::read(reg, dst);
  return dst;
}

}  // namespace func
}  // namespace gal