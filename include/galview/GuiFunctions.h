#pragma once
#include <galfunc/Functions.h>
#include <galview/Widget.h>

namespace gal {
namespace viewfunc {

void initPanels(view::Panel& inputs, view::Panel& outputs);

}
}  // namespace gal

// Forward declaration of the module initializer for embedded scripts.
// This will be defined by boost later.
extern "C" PyObject* PyInit_pygalview();