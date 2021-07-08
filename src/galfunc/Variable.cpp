#include <galfunc/Variable.h>
#include <unordered_map>

namespace gal {
namespace func {

struct Callbacks
{
  std::function<void(uint64_t, boost::python::object&)> readCb;
};

using CallbackMap = std::unordered_map<uint32_t, Callbacks>;

template<typename T>
void insertCallbacks(CallbackMap& map)
{
  Callbacks cb;
  cb.readCb = [](uint64_t regId, boost::python::object& dst) {
    auto data = gal::func::store::get<T>(regId);
    Converter<T, boost::python::object>::assign(*data, dst);
  };
  map.emplace(TypeInfo<T>::id, std::move(cb));
}

template<typename T, typename... Ts>
struct CallbackManager
{
  static void populateCallbackMap(CallbackMap& map)
  {
    insertCallbacks<T>(map);
    insertCallbacks<std::vector<T>>(map);
    if constexpr (sizeof...(Ts) > 0) {
      CallbackManager<Ts...>::populateCallbackMap(map);
    }
  }

  static CallbackMap getCallbackMap()
  {
    std::unordered_map<uint32_t, Callbacks> map;
    populateCallbackMap(map);
    return map;
  }
};

using CbManager = CallbackManager<float, int32_t, std::string, glm::vec3, glm::vec2>;

boost::python::object py_read(store::Register reg)
{
  static const auto map = CbManager::getCallbackMap();

  boost::python::object dst;
  auto                  match = map.find(reg.typeId);

  if (match != map.end()) {
    match->second.readCb(reg.id, dst);
  }
  else {
    std::cerr << "Cannot read objects of this type into python\n";
  }
  //   bool                  success = Reader::read(reg, dst);
  return dst;
}

}  // namespace func
}  // namespace gal