#include <galfunc/Dynamic.h>
#include <unordered_map>

namespace gal {
namespace func {

namespace types {
template<typename... Ts>
struct voider
{
  using type = void;
};

/**
 * @brief Template to check at compile time, whether the type has an indexing operator
 * ([]). This can be used to determine if it is a valid list type. This is the general
 * template that is always false;
 * @tparam TList
 */
template<typename TList, typename = void>
struct HasValidIndexOperator : public std::false_type
{
  using ListItemtype = void;
};

/**
 * @brief This is the template specialization that returns true for types that can be
 * indexed with a size_t index.
 * @tparam TList The list type.
 */
template<typename TList>
struct HasValidIndexOperator<
  TList,
  typename voider<decltype(std::declval<TList>()[std::declval<size_t>()])>::type>
{
  using ListItemType = std::remove_const_t<
    std::remove_reference_t<decltype(std::declval<TList>()[std::declval<size_t>()])>>;

  /**
   * @brief The list item type must be a supported type.
   */
  static constexpr bool value = TypeInfo<ListItemType>::value;
};

}  // namespace types

template<typename ListType, typename ItemType>
struct TListItemFn : public TFunction<2, TypeList<ListType, int32_t, ItemType>>
{
  static_assert(
    types::HasValidIndexOperator<ListType>::value,
    "Only indexible list / container types are allowed. Index type must be size_t");
  static void sItemGetter(std::shared_ptr<ListType> list,
                          std::shared_ptr<int32_t>  index,
                          std::shared_ptr<ItemType> item)
  {
    *item = (*list)[size_t(*index)];
  }

  TListItemFn(const store::Register& listReg, const store::Register& indexReg)
      : TFunction<2, TypeList<ListType, int32_t, ItemType>>(
          &sItemGetter,
          std::array<uint64_t, 2> {listReg.id, indexReg.id})
  {}

  virtual ~TListItemFn() = default;
};

struct Callbacks
{
  std::function<void(uint64_t, boost::python::object&)> readCb;
  std::function<std::shared_ptr<Function>(const store::Register&, const store::Register&)>
    makeListItemFnCb;
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

  if constexpr (types::HasValidIndexOperator<T>::value) {
    using ItemType = typename types::HasValidIndexOperator<T>::ListItemType;
    cb.makeListItemFnCb =
      [](const store::Register& listReg,
         const store::Register& indexReg) -> std::shared_ptr<Function> {
      return store::makeFunction<TListItemFn<T, ItemType>>(listReg, indexReg);
    };
  }
  else {
    cb.makeListItemFnCb =
      [](const store::Register& listReg,
         const store::Register& indexReg) -> std::shared_ptr<Function> {
      std::cerr << TypeInfo<T>::name() << " is not a list or container type.\n";
      throw std::bad_alloc();
    };
  }

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

  static CallbackMap getNewCallbackMap()
  {
    std::unordered_map<uint32_t, Callbacks> map;
    populateCallbackMap(map);
    return map;
  }

  static const CallbackMap& getCallbackMap()
  {
    static const CallbackMap sMap = getNewCallbackMap();
    return sMap;
  }

  static const Callbacks& getCallbacksForReg(const store::Register& reg)
  {
    static const CallbackMap& sMap  = getCallbackMap();
    auto                      match = sMap.find(reg.typeId);
    if (match == sMap.end()) {
      std::cerr << "Type " << reg.typeName << " is not supported for this operation\n";
      throw std::bad_alloc();
    }

    return std::get<1>(*match);
  }
};

using CbManager = CallbackManager<float, int32_t, std::string, glm::vec3, glm::vec2>;

boost::python::object py_read(store::Register reg)
{
  const auto&           cbs = CbManager::getCallbacksForReg(reg);
  boost::python::object dst;
  cbs.readCb(reg.id, dst);
  return dst;
}

PyFnOutputType<1> py_listItem(store::Register listReg, store::Register indexReg)
{
  const auto& cbs = CbManager::getCallbacksForReg(listReg);
  auto        fn  = cbs.makeListItemFnCb(listReg, indexReg);
  return pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
}

}  // namespace func
}  // namespace gal
