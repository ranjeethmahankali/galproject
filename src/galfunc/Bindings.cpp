#include <galfunc/Bindings.h>

using namespace gal::func;

boost::python::tuple py_loadObjFile(store::Register filepathReg)
{
  return pythonRegisterTuple(loadObjFile(filepathReg));
};

boost::python::tuple py_meshCentroid(store::Register meshReg)
{
  return pythonRegisterTuple(meshCentroid(meshReg));
};

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;

  class_<store::Register>("Register").def(self_ns::str(self_ns::self));

  class_<std::shared_ptr<gal::Mesh>>("Mesh");

  def("string", py_constant<std::string>);
  def("loadObjFile", py_loadObjFile);
  def("meshCentroid", py_meshCentroid);

  def("readFloat", py_readRegister<float>);
};
