#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/CircleFunctions.h>
#include <galfunc/Functions.h>
#include <galfunc/GeomFunctions.h>
#include <galfunc/LineFunctions.h>
#include <galfunc/MapMacro.h>
#include <galfunc/MeshFunctions.h>
#include <galfunc/SphereFunctions.h>
#include <galfunc/UtilFunctions.h>
#include <galfunc/Dynamic.h>

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

Lambda::Lambda(std::vector<uint64_t> inputs, std::vector<uint64_t> outputs)
    : mInputs(std::move(inputs))
    , mOutputs(std::move(outputs))
{}

Lambda::Lambda(const boost::python::list& pyInputs, const boost::python::list& pyOutputs)
{
  std::vector<Register> temp;
  Converter<boost::python::list, decltype(temp)>::assign(pyInputs, temp);
  mInputs.resize(temp.size());
  std::transform(temp.begin(), temp.end(), mInputs.begin(), [](const Register& reg) {
    return reg.id;
  });
  Converter<boost::python::list, decltype(temp)>::assign(pyOutputs, temp);
  mOutputs.resize(temp.size());
  std::transform(temp.begin(), temp.end(), mOutputs.begin(), [](const Register& reg) {
    return reg.id;
  });
}

const std::vector<uint64_t>& Lambda::inputs() const
{
  return mInputs;
}

const std::vector<uint64_t>& Lambda::outputs() const
{
  return mOutputs;
}

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
    std::cerr << "Cannot find a register with id: " << id << std::endl;
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

void useLambdaCapturedRegisters(const Function* fn, const Lambda& lda)
{
  const auto& outputs = lda.outputs();
  const auto& inputs  = lda.inputs();

  for (uint64_t current : outputs) {
    Register& reg     = getRegister(current);
    auto      owner   = reg.ownerFunc();
    size_t    nInputs = owner->numInputs();
    for (size_t i = 0; i < nInputs; i++) {
      uint64_t inp = owner->inputRegister(i);
      if (std::find(inputs.begin(), inputs.end(), inp) == inputs.end()) {
        store::useRegister(fn, inp);
      }
    }
  }
}

void markDirty(uint64_t id)
{
  std::vector<uint64_t> ids;
  ids.reserve(sRegisterMap.size());
  ids.push_back(id);

  while (!ids.empty()) {
    uint64_t  current = ids.back();
    Register& reg     = getRegister(current);
    ids.pop_back();
    if (!ids.empty() && id == current) {
      // Cycle detected... bail out.
      return;
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

void unloadAllFunctions()
{
  std::cout << "Unloading all functions...\n";
  store::sFunctionMap.clear();
  store::sRegisterMap.clear();
  store::sRegisterUserMap.clear();
  store::sRegisterId = 0;
}

}  // namespace func
}  // namespace gal

#define GAL_DEF_PY_FN_ALL(fnNames) MAP(GAL_DEF_PY_FN, fnNames)

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  using namespace gal::func;

  class_<gal::func::store::Register>("Register").def(self_ns::str(self_ns::self));

  def("string", py_variable<std::string, std::string>);
  def("numberf32", py_variable<float, float>);
  def("vec3Var", py_variable<glm::vec3, boost::python::list>);
  def("vec2Var", py_variable<glm::vec2, boost::python::list>);
  def("lambdaFromRegisters",
      py_variable<store::Lambda, boost::python::list, boost::python::list>);

  def("listf32", py_list<float>);
  def("listvec3", py_list<glm::vec3>);
  def("liststring", py_list<std::string>);

  GAL_DEF_PY_FN_ALL(GAL_UtilFunctions)
  GAL_DEF_PY_FN_ALL(GAL_GeomFunctions)
  GAL_DEF_PY_FN_ALL(GAL_MeshFunctions)
  GAL_DEF_PY_FN_ALL(GAL_CircleFunctions)
  GAL_DEF_PY_FN_ALL(GAL_SphereFunctions)
  GAL_DEF_PY_FN_ALL(GAL_LineFunctions)

  GAL_DEF_PY_FN(read);
};
