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

const FuncInfo& Function::info() const
{
  return mInfo;
}

void Function::info(const FuncInfo& info)
{
  mInfo = info;
}

size_t Function::depth() const
{
  return mDepth;
}

void Function::clearDepth()
{
  mDepth = SIZE_MAX;
}

size_t Function::numInputs() const
{
  return mInfo.mNumInputs;
}

size_t Function::numOutputs() const
{
  return mInfo.mNumOutputs;
}

namespace store {

static std::vector<std::shared_ptr<Function>> sFunctions;

static std::unordered_map<uint64_t,
                          std::vector<std::reference_wrapper<std::atomic_bool>>,
                          CustomSizeTHash>
  sSubscriberMap;

std::shared_ptr<Function> addFunction(const FuncInfo&                  fnInfo,
                                      const std::shared_ptr<Function>& fn)
{
  fn->info(fnInfo);
  sFunctions.push_back(fn);
  return fn;
};

void getGraphData(std::vector<FunctionGraphData>& gdata)
{
  // Clear previously computed depths.
  for (const auto& fn : sFunctions) {
    fn->clearDepth();
  }

  // Recompute depths.
  for (auto& fn : sFunctions) {
    fn->calcDepth();
  }

  gdata.resize(sFunctions.size());
  std::transform(sFunctions.begin(),
                 sFunctions.end(),
                 gdata.begin(),
                 [](const std::shared_ptr<Function>& fn) {
                   return FunctionGraphData {fn.get(), int(fn->depth()), int(0)};
                 });
  std::sort(gdata.begin(),
            gdata.end(),
            [](const FunctionGraphData& a, const FunctionGraphData& b) {
              return a.mCol < b.mCol;
            });
  std::vector<const Function*>                         upstream;
  std::unordered_map<uint64_t, float, CustomSizeTHash> rscores;
  for (auto colbegin = gdata.begin(); colbegin != gdata.end();) {
    size_t curCol   = colbegin->mCol;
    size_t colstart = std::distance(gdata.begin(), colbegin);
    auto   colend =
      std::find_if(colbegin, gdata.end(), [&curCol](const FunctionGraphData& fn) {
        return fn.mCol != curCol;
      });
    size_t colsize = std::distance(colbegin, colend);
    if (curCol > 0) {
      for (auto begin = colbegin; begin != colend; begin++) {
        begin->mFunc->getUpstreamFunctions(upstream);
        float rsum = 0.f;
        for (const auto upfn : upstream) {
          rsum += rscores[uint64_t(upfn)];
        }
        rscores.emplace(uint64_t(begin->mFunc), rsum / float(upstream.size()));
      }
      std::sort(colbegin,
                colend,
                [&rscores](const FunctionGraphData& a, const FunctionGraphData& b) {
                  return rscores[uint64_t(a.mFunc)] < rscores[uint64_t(b.mFunc)];
                });
    }
    int nrows  = int(colsize);
    int center = nrows / 2;
    colbegin   = gdata.begin() + colstart;
    colend     = colbegin + colsize;
    for (int ri = 0; ri < nrows; ri++, colbegin++) {
      colbegin->mRow                     = ri - center;
      rscores[uint64_t(colbegin->mFunc)] = float(colbegin->mRow);
    }
    colbegin = colend;
  }

  if (gdata.empty()) {
    return;
  }

  auto minRowNode =
    std::min_element(gdata.begin(),
                     gdata.end(),
                     [](const FunctionGraphData& a, const FunctionGraphData& b) {
                       return a.mRow < b.mRow;
                     });
  int minRow = -minRowNode->mRow + 1;
  for (auto& g : gdata) {
    g.mRow += minRow;
  }
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
