#include <galfunc/Functions.h>
#include <galtest/TestUtils.h>

namespace gal {
namespace test {

void initPythonEnv()
{
  static bool sInitFlag = false;
  if (!sInitFlag) {
    PyImport_AppendInittab("pygalfunc", &PyInit_pygalfunc);
    Py_Initialize();
    sInitFlag = true;
  }
}

}  // namespace test
}  // namespace gal
