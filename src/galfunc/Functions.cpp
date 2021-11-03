#include <string>

#include <boost/python/class.hpp>
#include <boost/python/import.hpp>

#include <galcore/Annotations.h>
#include <galcore/Circle2d.h>
#include <galcore/Line.h>
#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/MapMacro.h>
#include <galfunc/TypeHelper.h>

namespace gal {
namespace func {

Function::Function()
    : mContextPath(python::getcontextpath())
{}

const fs::path& Function::contextpath() const
{
  return mContextPath;
}

const std::string& Function::name() const
{
  return mName;
}

void Function::name(std::string name)
{
  mName = std::move(name);
}

namespace store {

static std::unordered_map<uint64_t, std::shared_ptr<Function>, CustomSizeTHash>
  sFunctionMap;

static std::unordered_map<uint64_t,
                          std::vector<std::reference_wrapper<std::atomic_bool>>,
                          CustomSizeTHash>
  sSubscriberMap;

std::shared_ptr<Function> addFunction(std::string                      name,
                                      const std::shared_ptr<Function>& fn)
{
  fn->name(std::move(name));  // Name the function.
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

FuncDocString::FuncDocString(
  const std::string&                                      desc,
  const std::vector<std::pair<std::string, std::string>>& inputs,
  const std::vector<std::pair<std::string, std::string>>& outputs)
{
  mDocString = desc + "\n";
  for (const auto& argpair : inputs) {
    mDocString += argpair.first + ": " + argpair.second + "\n";
  }
  mDocString +=
    "\n****"
    " Return values "
    "****\n";
  for (const auto& argpair : outputs) {
    mDocString += argpair.first + ": " + argpair.second + "\n";
  }
}

const char* FuncDocString::c_str() const
{
  return mDocString.c_str();
}

}  // namespace python

// Forward declare the binding functions.
void bind_UtilFunctions();
void bind_GeomFunctions();
void bind_CircleFunctions();
void bind_SphereFunctions();
void bind_LineFunctions();
void bind_MeshFunctions();
void bind_MathFunctions();
void bind_ListFunctions();

}  // namespace func
}  // namespace gal

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  using namespace gal::func::python;
  using namespace gal::func;

  typemanager::invoke<defClass>();

  bind_UtilFunctions();
  bind_GeomFunctions();
  bind_CircleFunctions();
  bind_SphereFunctions();
  bind_LineFunctions();
  bind_MeshFunctions();
  bind_MathFunctions();
  bind_ListFunctions();
};
