#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>

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
    uint64_t current             = ids.back();
    getRegister(current).isDirty = true;
    ids.pop_back();
    auto   fn    = getRegister(current).ownerFunc();
    size_t nOuts = fn->numOutputs();
    for (size_t i = 0; i < nOuts; i++) {
      ids.push_back(fn->outputRegister(i));
    }
  }
};

}  // namespace store

}  // namespace func
}  // namespace gal

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;

  class_<gal::func::store::Register>("Register").def(self_ns::str(self_ns::self));

  class_<std::shared_ptr<gal::Mesh>>("Mesh");

  def("string", gal::func::py_constant<std::string>);
  def("loadObjFile", gal::func::py_loadObjFile);
  def("meshCentroid", gal::func::py_meshCentroid);

  def("readFloat", gal::func::py_readRegister<float>);
};