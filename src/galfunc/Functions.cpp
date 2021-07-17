#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/CircleFunctions.h>
#include <galfunc/Functions.h>
#include <galfunc/GeomFunctions.h>
#include <galfunc/MapMacro.h>
#include <galfunc/MeshFunctions.h>
#include <galfunc/SphereFunctions.h>
#include <galfunc/UtilFunctions.h>
#include <galfunc/Variable.h>

namespace std {
std::ostream& operator<<(std::ostream& ostr, const gal::func::store::Register& reg)
{
  ostr << "[" << reg.typeName << " in reg " << reg.id << "]";
  return ostr;
};
}  // namespace std

namespace gal {
namespace func {

namespace store {

static std::unordered_map<uint64_t, std::shared_ptr<Function>, CustomSizeTHash>
                                                                  sFunctionMap;
static std::unordered_map<uint64_t, Register, CustomSizeTHash>    sRegisterMap;
static std::unordered_map<uint64_t, std::vector<const Function*>> sRegisterUserMap;

static uint64_t sRegisterId = 0;

std::shared_ptr<Function> Register::ownerFunc() const
{
  auto match = sFunctionMap.find(uint64_t(owner));
  if (match == sFunctionMap.end()) {
    std::cerr << "Not a function\n";
    throw std::bad_alloc();
  }
  return match->second;
};

uint64_t allocate(const Function* fn, uint32_t typeId, const std::string& typeName)
{
  auto match = sFunctionMap.find(uint64_t(fn));
  if (match == sFunctionMap.end()) {
    std::cerr << "Not a function\n";
    throw std::bad_alloc();
  }

  Register reg;
  reg.id       = ++sRegisterId;
  reg.owner    = fn;
  reg.typeId   = typeId;
  reg.typeName = typeName;
  sRegisterMap.emplace(reg.id, reg);
  sRegisterUserMap.emplace(reg.id, std::vector<const Function*>());
  return reg.id;
};

void free(uint64_t id)
{
  if (!sRegisterMap.empty()) {
    auto match = sRegisterMap.find(id);
    if (match != sRegisterMap.end()) {
      sRegisterMap.erase(match);
    }
  }
};

Register& getRegister(uint64_t id)
{
  auto match = sRegisterMap.find(id);
  if (match == sRegisterMap.end()) {
    std::cerr << "Not a valid register\n";
    throw std::bad_alloc();
  }
  return match->second;
};

std::shared_ptr<Function> addFunction(const std::shared_ptr<Function>& fn)
{
  sFunctionMap.emplace(uint64_t(fn.get()), fn);
  fn->initOutputRegisters();
  return fn;
};

void useRegister(const Function* fn, uint64_t id)
{
  auto match = sRegisterUserMap.find(id);
  if (match != sRegisterUserMap.end()) {
    match->second.push_back(fn);
  }
};

void markDirty(uint64_t id)
{
  std::vector<uint64_t> ids;
  ids.reserve(sRegisterMap.size());
  ids.push_back(id);

  while (!ids.empty()) {
    uint64_t  current = ids.back();
    Register& reg     = getRegister(current);
    ids.pop_back();
    if (reg.isDirty) {
      // If this register is already dirty, we expect downstream to be dirty.
      continue;
    }
    reg.isDirty       = true;
    const auto& users = sRegisterUserMap[current];
    for (const auto& user : users) {
      for (size_t j = 0; j < user->numOutputs(); j++) {
        ids.push_back(user->outputRegister(j));
      }
    }
  }
};

}  // namespace store

}  // namespace func
}  // namespace gal

#define GAL_DEF_PY_FN_ALL(fnNames) MAP(GAL_DEF_PY_FN, fnNames)

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  using namespace gal::func;

  class_<gal::func::store::Register>("Register").def(self_ns::str(self_ns::self));

  def("string", py_variable<std::string>);
  def("numberf32", py_variable<float>);

  def("listf32", py_list<float>);
  def("listvec3", py_list<glm::vec3>);
  def("liststring", py_list<std::string>);

  GAL_DEF_PY_FN_ALL(GAL_UtilFunctions)
  GAL_DEF_PY_FN_ALL(GAL_GeomFunctions)
  GAL_DEF_PY_FN_ALL(GAL_MeshFunctions)
  GAL_DEF_PY_FN_ALL(GAL_CircleFunctions)
  GAL_DEF_PY_FN_ALL(GAL_SphereFunctions)

  GAL_DEF_PY_FN(read);
};
