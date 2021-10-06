#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/CircleFunctions.h>
#include <galfunc/Functions.h>
#include <galfunc/GeomFunctions.h>
#include <galfunc/LineFunctions.h>
#include <galfunc/MapMacro.h>
#include <galfunc/MeshFunctions.h>
#include <galfunc/SphereFunctions.h>
#include <galfunc/TypeHelper.h>
#include <galfunc/UtilFunctions.h>
#include <boost/python/class.hpp>
#include <boost/python/import.hpp>
#include <string>
#include "galcore/Annotations.h"
#include "galcore/Circle2d.h"
#include "galcore/Line.h"

namespace gal {
namespace func {

namespace store {

static std::unordered_map<uint64_t, std::shared_ptr<Function>, CustomSizeTHash>
  sFunctionMap;

static std::unordered_map<uint64_t,
                          std::vector<std::reference_wrapper<std::atomic_bool>>,
                          CustomSizeTHash>
  sSubscriberMap;

std::shared_ptr<Function> addFunction(const std::shared_ptr<Function>& fn)
{
  sFunctionMap.emplace(uint64_t(fn.get()), fn);
  return fn;
};

void addSubscriber(const Function* fn, std::atomic_bool& dirtyFlag)
{
  sSubscriberMap[uint64_t(fn)].push_back(dirtyFlag);
}

void markDirty(const Function* fn)
{
  auto match = sSubscriberMap.find(uint64_t(fn));
  if (match != sSubscriberMap.end()) {
    auto& flags = std::get<1>(*match);
    for (std::atomic_bool& flag : flags) {
      flag = true;
    }
  }
}

void unloadAllFunctions()
{
  std::cout << "Unloading all functions...\n";
  store::sFunctionMap.clear();
}

}  // namespace store

namespace python {
fs::path getcontextpath()
{
  using namespace boost::python;
  auto traceback = import("traceback");
  auto summary   = list(traceback.attr("extract_stack")());
  int  n         = int(len(summary));

  fs::path    result;
  fs::path    filename, cfn;
  std::string name;
  for (int i = n - 1; i > -1; i--) {
    cfn = fs::path(std::string(extract<std::string>(summary[i].attr("filename"))));
    if (filename != cfn || filename.empty()) {
      if (!filename.empty()) {
        result = filename.stem() / result;
      }
      filename = cfn;
    }
    name = extract<std::string>(summary[i].attr("name"));
    if (name != "<module>") {
      result = name / result;
    }
  }
  result = filename.stem() / result;
  return result;
}
}  // namespace python

}  // namespace func
}  // namespace gal

#define GAL_DEF_PY_FN_ALL(fnNames) MAP(GAL_DEF_PY_FN, fnNames)

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  using namespace gal::func::python;
  using namespace gal::func;

  typemanager::invoke<defClass>();

  def("var", py_variable<std::string, std::string>);
  def("var", py_variable<float, float>);
  def("var", py_variable<int32_t, int32_t>);
  def("var", py_variable<glm::vec3, boost::python::list>);
  def("var", py_variable<glm::vec2, boost::python::list>);

  def("listf32", py_list<float>);
  def("listi32", py_list<int32_t>);
  def("listvec3", py_list<glm::vec3>);
  def("liststring", py_list<std::string>);

  GAL_DEF_PY_FN_ALL(GAL_UtilFunctions)
  GAL_DEF_PY_FN_ALL(GAL_GeomFunctions)
  GAL_DEF_PY_FN_ALL(GAL_CircleFunctions)
  GAL_DEF_PY_FN_ALL(GAL_SphereFunctions)
  GAL_DEF_PY_FN_ALL(GAL_LineFunctions)
  GAL_DEF_PY_FN_ALL(GAL_MeshFunctions)
};
