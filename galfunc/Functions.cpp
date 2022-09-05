#include <assert.h>
#include <string>

#include <pybind11/pybind11.h>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <galcore/Annotations.h>
#include <galcore/Circle2d.h>
#include <galcore/Line.h>
#include <galcore/Types.h>
#include <galcore/Util.h>
#include <galfunc/Functions.h>
#include <galfunc/MapMacro.h>
#include <galfunc/TypeManager.h>
#include <spdlog/spdlog.h>

namespace gal {
namespace func {

static auto sLogger = spdlog::stdout_color_mt("galfunc");

spdlog::logger& logger()
{
  spdlog::set_level(spdlog::level::level_enum::debug);
  return *sLogger;
}

Function::Function()
    : mContextPath(python::getcontextpath())
{}

const fs::path& Function::contextpath() const
{
  return mContextPath;
}

const FuncInfo& Function::info() const
{
  return mInfo;
}

FuncInfo& Function::info()
{
  return mInfo;
}

size_t Function::numInputs() const
{
  return mInfo.mNumInputs;
}

size_t Function::numOutputs() const
{
  return mInfo.mNumOutputs;
}

int Function::index() const
{
  return mIndex;
}

int& Function::index()
{
  return mIndex;
}

namespace store {

static std::vector<std::unique_ptr<Function>> sFunctions;

Function* addFunction(const FuncInfo& fnInfo, std::unique_ptr<Function> fn)
{
  fn->info()  = fnInfo;
  fn->index() = sFunctions.size();
  sFunctions.push_back(std::move(fn));
  properties().resize(sFunctions.size());
  return sFunctions.back().get();
};

size_t numFunctions()
{
  return sFunctions.size();
}

const Function& function(size_t i)
{
  return *(sFunctions[i]);
}

Properties& properties()
{
  static Properties sProps;
  return sProps;
}

void unloadAllFunctions()
{
  sLogger->debug("Unloading all functions...");
  sFunctions.clear();
}

}  // namespace store

namespace python {
namespace py = pybind11;
fs::path getcontextpath()
{
  auto traceback = py::module_::import("traceback");
  auto summary   = py::list(traceback.attr("extract_stack")());
  int  n         = int(len(summary));

  fs::path    result;
  fs::path    filename, cfn;
  std::string name;
  for (int i = n - 1; i > -1; i--) {
    cfn = fs::path(std::string(py::cast<std::string>(summary[i].attr("filename"))));
    if (filename != cfn || filename.empty()) {
      if (!filename.empty()) {
        result = filename.stem() / result;
      }
      filename = cfn;
    }
    name = py::cast<std::string>(summary[i].attr("name"));
    if (name != "<module>") {
      result = name / result;
    }
  }
  result = filename.stem() / result;
  return result;
}

FuncDocString::FuncDocString(const FuncInfo& fnInfo)
{
  static constexpr std::string_view sColon = ": ";

  mDocString = fnInfo.mDesc;
  mDocString.push_back('\n');
  for (size_t i = 0; i < fnInfo.mNumInputs; i++) {
    const auto& name = fnInfo.mInputNames[i];
    const auto& desc = fnInfo.mInputDescriptions[i];
    std::copy(name.begin(), name.end(), std::back_inserter(mDocString));
    std::copy(sColon.begin(), sColon.end(), std::back_inserter(mDocString));
    std::copy(desc.begin(), desc.end(), std::back_inserter(mDocString));
    mDocString.push_back('\n');
  }

  mDocString += "\n**** Return values ****\n";

  for (size_t i = 0; i < fnInfo.mNumOutputs; i++) {
    const auto& name = fnInfo.mOutputNames[i];
    const auto& desc = fnInfo.mOutputDescriptions[i];
    std::copy(name.begin(), name.end(), std::back_inserter(mDocString));
    std::copy(sColon.begin(), sColon.end(), std::back_inserter(mDocString));
    std::copy(desc.begin(), desc.end(), std::back_inserter(mDocString));
    mDocString.push_back('\n');
  }
}

const char* FuncDocString::c_str() const
{
  return mDocString.c_str();
}

}  // namespace python

// Forward declare the binding functions.
void bind_UtilFunc(py::module&);
void bind_GeomFunc(py::module&);
void bind_CircleFunc(py::module&);
void bind_SphereFunc(py::module&);
void bind_LineFunc(py::module&);
void bind_MeshFunc(py::module&);
void bind_MathFunc(py::module&);
void bind_ListFunc(py::module&);
void bind_TreeFunc(py::module&);
void bind_SortFunc(py::module&);

}  // namespace func
}  // namespace gal

PYBIND11_MODULE(pygalfunc, pgf)
{
  using namespace gal::func::python;
  using namespace gal::func;

  typemanager::invoke<defClass>((py::module&)pgf);

  bind_UtilFunc(pgf);
  bind_GeomFunc(pgf);
  bind_CircleFunc(pgf);
  bind_SphereFunc(pgf);
  bind_LineFunc(pgf);
  bind_MeshFunc(pgf);
  bind_MathFunc(pgf);
  bind_ListFunc(pgf);
  bind_TreeFunc(pgf);
  bind_SortFunc(pgf);
};
