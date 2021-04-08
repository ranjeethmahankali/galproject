#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>

namespace std {
std::ostream& operator<<(std::ostream& ostr, const gal::func::store::Register& reg)
{
  ostr << "[" << reg.typeName << " in reg " << reg.typeId << "]\n";
  return ostr;
};
}  // namespace std

namespace gal {
namespace func {

// Temp test code - begins.

std::shared_ptr<gal::Mesh> py_loadObjFile(const std::string& filepath)
{
  return std::make_shared<gal::Mesh>(std::move(io::ObjMeshData(filepath, true).toMesh()));
};

// Temp test code - ends.

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;

  class_<store::Register>("Register").def(self_ns::str(self_ns::self));

  class_<std::shared_ptr<gal::Mesh>>("Mesh");
  def("meshCentroid", py_meshCentroid);
  def("loadObjFile", py_loadObjFile);
};

}  // namespace func
}  // namespace gal