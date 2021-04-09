#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>

namespace std {
std::ostream& operator<<(std::ostream& ostr, const gal::func::store::Register& reg)
{
  ostr << "[" << reg.typeName << " in reg " << reg.typeId << "]\n";
  return ostr;
};
}  // namespace std

namespace gal {
namespace func {

namespace store {

static std::unordered_map<uint64_t, std::shared_ptr<Function>, CustomSizeTHash>
                                              sFunctionMap;
static std::unordered_map<uint64_t, Register> sRegisterMap;
static uint64_t                               sRegisterId = 0;

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
  return reg.id;
};

void free(uint64_t id)
{
  sRegisterMap.erase(id);
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

void addFunction(const std::shared_ptr<Function>& fn)
{
  sFunctionMap.emplace(uint64_t(fn.get()), fn);
};

}  // namespace store

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;

  class_<store::Register>("Register").def(self_ns::str(self_ns::self));

  class_<std::shared_ptr<gal::Mesh>>("Mesh");

  def("string", py_constant<std::string>);
  def("loadObjFile", py_loadObjFile);
  def("meshCentroid", py_meshCentroid);
};

}  // namespace func
}  // namespace gal