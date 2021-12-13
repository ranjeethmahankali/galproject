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
#include <galfunc/TypeManager.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace gal {
namespace func {

static auto sLogger = spdlog::stdout_color_mt("galfunc");

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

static std::vector<std::shared_ptr<Function>> sFunctions;

static std::unordered_map<uint64_t,
                          std::vector<std::reference_wrapper<std::atomic_bool>>,
                          CustomSizeTHash>
  sSubscriberMap;

std::shared_ptr<Function> addFunction(std::string                      name,
                                      const std::shared_ptr<Function>& fn)
{
  fn->name(std::move(name));  // Name the function.
  sFunctions.push_back(fn);
  return fn;
};

const std::vector<std::shared_ptr<Function>>& allFunctions()
{
  return sFunctions;
}

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
  sLogger->debug("Unloading all functions...");
  store::sFunctions.clear();
  store::sSubscriberMap.clear();
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
void bind_UtilFunc();
void bind_GeomFunc();
void bind_CircleFunc();
void bind_SphereFunc();
void bind_LineFunc();
void bind_MeshFunc();
void bind_MathFunc();
void bind_ListFunc();
void bind_TreeFunc();
void bind_SortFunc();

}  // namespace func
}  // namespace gal

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  using namespace gal::func::python;
  using namespace gal::func;

  typemanager::invoke<defClass>();

  bind_UtilFunc();
  bind_GeomFunc();
  bind_CircleFunc();
  bind_SphereFunc();
  bind_LineFunc();
  bind_MeshFunc();
  bind_MathFunc();
  bind_ListFunc();
  bind_TreeFunc();
  bind_SortFunc();
};
