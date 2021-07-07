#pragma once
#include <galfunc/Functions.h>
#include <galfunc/Variable.h>
#include <galview/Widget.h>

namespace gal {
namespace viewfunc {

void         initPanels(view::Panel& inputs, view::Panel& outputs);
view::Panel& inputPanel();
view::Panel& outputPanel();
void         evalOutputs();

template<typename T>
struct SliderFunc : public gal::func::TVariable<T>, public gal::view::Slider<T>
{
public:
  SliderFunc(const std::string& label, T min, T max, T value)
      : gal::func::TVariable<T>(value)
      , gal::view::Slider<T>(label, min, max, value) {};

private:
  using gal::view::Slider<T>::addHandler;

protected:
  void handleChanges() override
  {
    if (this->isEdited())
      this->set(this->mValue);

    this->clearEdited();
  };
};

struct ShowFunc : public gal::func::Function, public gal::view::CheckBox
{
  ShowFunc(const std::string& label, uint64_t regId);
  ShowFunc(const std::string& label, const std::vector<uint64_t>& regIds);

  void     run() override;
  void     initOutputRegisters() override;
  size_t   numOutputs() const override;
  uint64_t outputRegister(size_t index) const override;

private:
  using gal::view::CheckBox::addHandler;

  /* The first is the reg-id of the object being shown. The second is the drawId of that
   * same object that is tracked by the viewer. Knowing the reg-id is needed in order to
   * get retriggered when the upstream changes, and in order to get the latest geometry.
   * Knowing the drawId is needed in order to interact with the viewer.*/
  using Showable = std::pair<uint64_t, size_t>;

  void useUpstreamRegisters();

private:
  std::vector<Showable> mShowables;
  std::shared_ptr<bool> mSuccess;
  uint64_t              mRegisterId;  // Output
};

struct TagsFunc : public gal::func::Function, public gal::view::CheckBox
{
  TagsFunc(const std::string& label, uint64_t locRegId, uint64_t wordsRegId);

  void     run() override;
  void     initOutputRegisters() override;
  size_t   numOutputs() const override;
  uint64_t outputRegister(size_t index) const override;

private:
  using gal::view::CheckBox::addHandler;

private:
  uint64_t              mLocsRegId;
  uint64_t              mWordsRegId;
  size_t                mDrawId = 0;
  std::shared_ptr<bool> mSuccess;
  uint64_t              mRegisterId;
};

template<typename T>
gal::func::types::OutputTuple<1> slider(const std::string& label,
                                        const T&           min,
                                        const T&           max,
                                        const T&           value)
{
  auto fn = gal::func::store::makeFunction<SliderFunc<T>>(label, min, max, value);
  inputPanel().addWidget(std::dynamic_pointer_cast<gal::view::Widget>(fn));
  return gal::func::types::makeOutputTuple<1>(*fn);
};

template<typename T>
boost::python::tuple py_slider(const std::string& label,
                               const T&           min,
                               const T&           max,
                               const T&           value)
{
  return gal::func::pythonRegisterTuple(slider<T>(label, min, max, value));
};

}  // namespace viewfunc
}  // namespace gal

// Forward declaration of the module initializer for embedded scripts.
// This will be defined by boost later.
extern "C" PyObject* PyInit_pygalview();
