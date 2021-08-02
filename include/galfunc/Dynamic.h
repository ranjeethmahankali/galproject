#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

boost::python::object py_read(store::Register reg);

PyFnOutputType<1> py_listItem(store::Register listReg, store::Register indexReg);

PyFnOutputType<1> py_makeList(const boost::python::list& itemRegs);

PyFnOutputType<1> py_makeRepeatedList(store::Register itemReg, store::Register countReg);

}  // namespace func
}  // namespace gal

#define GAL_DynamicFunctions read, listItem, makeList, makeRepeatedList
