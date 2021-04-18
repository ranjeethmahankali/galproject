#include <galfunc/GeomFunctions.h>
#include <galfunc/MeshFunctions.h>
#include <galview/AllViews.h>
#include <galview/Context.h>
#include <galview/GuiFunctions.h>
#include <sstream>

namespace std {
std::ostream& operator<<(std::ostream& ostr, const glm::vec3& v)
{
  ostr << "(" << v.x << ", " << v.y << ", " << v.z << ")";
  return ostr;
};
}  // namespace std

namespace gal {
namespace viewfunc {

static view::Panel*          sInputPanel  = nullptr;
static view::Panel*          sOutputPanel = nullptr;
static std::vector<uint64_t> mShowFuncRegs;
static std::vector<uint64_t> mPrintFuncRegs;

void initPanels(view::Panel& inputs, view::Panel& outputs)
{
  sInputPanel  = &inputs;
  sOutputPanel = &outputs;
};

view::Panel& inputPanel()
{
  return *sInputPanel;
};

view::Panel& outputPanel()
{
  return *sOutputPanel;
};

void evalOutputs()
{
  for (uint64_t id : mShowFuncRegs) {
    auto success = gal::func::store::get<bool>(id);
  }
  for (uint64_t id : mPrintFuncRegs) {
    auto success = gal::func::store::get<bool>(id);
  }
};

template<typename T, typename... TRest>
struct DrawableManager
{
  static size_t draw(uint64_t typeId, const std::shared_ptr<void>& ptr, size_t oldDrawId)
  {
    static_assert(gal::TypeInfo<T>::value, "Unknown type");
    if (typeId == gal::TypeInfo<T>::id) {
      auto castsp = std::static_pointer_cast<T>(ptr);
      return view::Context::get().replaceDrawable<T>(oldDrawId, *castsp);
    }
    else if constexpr (sizeof...(TRest) > 0) {
      return DrawableManager<TRest...>::draw(typeId, ptr, oldDrawId);
    }
    else if constexpr (sizeof...(TRest) == 0) {
      std::cerr << "Datatype " << gal::TypeInfo<T>::name()
                << " is not a drawable object\n";
      throw std::bad_cast();
    }
  };
};
using dmanager = DrawableManager<gal::Box3,
                                 gal::PointCloud,
                                 gal::Sphere,
                                 gal::Circle2d,
                                 gal::Mesh,
                                 gal::Plane>;

ShowFunc::ShowFunc(uint64_t regId)
    : mObjRegId(regId)
    , mSuccess(std::make_shared<bool>(false))
{
  gal::func::store::useRegister(this, mObjRegId);
};

void ShowFunc::initOutputRegisters()
{
  mRegisterId = gal::func::store::allocate(
    this, gal::TypeInfo<bool>::id, gal::TypeInfo<bool>::name());
};

void ShowFunc::run()
{
  try {
    auto  obj = gal::func::store::get<void>(mObjRegId);
    auto& reg = gal::func::store::getRegister(mObjRegId);
    mDrawId   = dmanager::draw(reg.typeId, reg.ptr, mDrawId);
    *mSuccess = true;
  }
  catch (std::bad_alloc ex) {
    *mSuccess = false;
  }
  gal::func::store::set<bool>(mRegisterId, mSuccess);
};

size_t ShowFunc::numOutputs() const
{
  return 1;
};

uint64_t ShowFunc::outputRegister(size_t index) const
{
  if (index == 0) {
    return mRegisterId;
  }
  throw std::out_of_range("Index out of range");
};

template<typename T, typename... TRest>
struct PrintManager
{
  static std::string print(uint64_t typeId, const std::shared_ptr<void>& ptr)
  {
    static_assert(gal::TypeInfo<T>::value, "Unknown type");
    if (typeId == gal::TypeInfo<T>::id) {
      auto              castsp = std::static_pointer_cast<T>(ptr);
      std::stringstream ss;
      ss << *castsp;
      return ss.str();
    }
    else if (typeId == gal::TypeInfo<std::vector<T>>::id) {
      auto              castsp = std::static_pointer_cast<std::vector<T>>(ptr);
      std::stringstream ss;
      ss << *castsp;
      return ss.str();
    }
    else if constexpr (sizeof...(TRest) > 0) {
      return PrintManager<TRest...>::print(typeId, ptr);
    }
    else if constexpr (sizeof...(TRest) == 0) {
      std::cerr << "Datatype " << gal::TypeInfo<T>::name
                << " is not a printable object\n";
      throw std::bad_cast();
    }
  };
};

using printmanager = PrintManager<float, int32_t, glm::vec3, std::string>;

struct PrintFunc : public gal::func::Function, public gal::view::Text
{
private:
  std::string           mLabel;
  uint64_t              mObjRegId;
  std::shared_ptr<bool> mSuccess;
  uint64_t              mRegisterId;

public:
  PrintFunc(const std::string& label, uint64_t regId)
      : gal::view::Text("")
      , mLabel(label)
      , mObjRegId(regId)
      , mSuccess(std::make_shared<bool>(false))
  {
    gal::func::store::useRegister(this, mObjRegId);
  };

  void run() override
  {
    try {
      auto  obj    = gal::func::store::get<void>(mObjRegId);
      auto& reg    = gal::func::store::getRegister(mObjRegId);
      this->mValue = mLabel + ": " + printmanager::print(reg.typeId, reg.ptr);
      *mSuccess    = true;
    }
    catch (std::bad_alloc ex) {
      *mSuccess = false;
    }
    gal::func::store::set<bool>(mRegisterId, mSuccess);
  };
  void initOutputRegisters() override
  {
    mRegisterId = gal::func::store::allocate(this,
                                             gal::TypeInfo<bool>::id,
                                             gal::TypeInfo<bool>::name());
  };
  size_t   numOutputs() const override { return 1; };
  uint64_t outputRegister(size_t index) const override
  {
    if (index == 0) {
      return mRegisterId;
    }
    throw std::out_of_range("Index out of range");
  };
};

// Manual definition for show function because it has special needs.
gal::func::types::OutputTuple<1> show(const gal::func::store::Register& reg)
{
  using namespace gal::func;
  auto fn = store::makeFunction<ShowFunc>(reg.id);
  mShowFuncRegs.push_back(fn->outputRegister(0));
  return types::makeOutputTuple<1>(*fn);
};
boost::python::tuple py_show(gal::func::store::Register reg)
{
  return gal::func::pythonRegisterTuple(show(reg));
};

// Manual definition for the print function because it has special needs.
gal::func::types::OutputTuple<1> print(const std::string&                label,
                                       const gal::func::store::Register& reg)
{
  using namespace gal::func;
  auto fn = store::makeFunction<PrintFunc>(label, reg.id);
  mPrintFuncRegs.push_back(fn->outputRegister(0));
  auto wfn = std::dynamic_pointer_cast<gal::view::Widget>(fn);
  outputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return types::makeOutputTuple<1>(*fn);
};
boost::python::tuple py_print(const std::string& label, gal::func::store::Register reg)
{
  return gal::func::pythonRegisterTuple(print(label, reg));
};

// Simple functions that are not part of the functional system.
void py_set2dMode(bool flag)
{
  gal::view::Context::get().set2dMode(flag);
};

}  // namespace viewfunc
}  // namespace gal

BOOST_PYTHON_MODULE(pygalview)
{
  using namespace boost::python;
  using namespace gal::viewfunc;
  //   class_<gal::func::store::Register>("Register").def(self_ns::str(self_ns::self));
  // Bindings related to gui
  // Views for drawables
  // Labels for strings
  // Sliders for float input
  // Text fields for string inputs
  def("sliderf32", gal::viewfunc::py_slider<float>);
  def("slideri32", gal::viewfunc::py_slider<int32_t>);
  GAL_DEF_PY_FN(show);
  GAL_DEF_PY_FN(print);
  GAL_DEF_PY_FN(set2dMode);
};