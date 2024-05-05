#include <Decimate.h>

#include <tbb/parallel_for_each.h>
#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <glm/geometric.hpp>
#include <numeric>
#include <stdexcept>

#include <ProbabilisticQuadrics.h>

namespace gal {

class ModProbQuadricT : public OpenMesh::Decimater::ModBaseT<TriMesh>
{
public:
  using Self         = ModProbQuadricT;
  using Handle       = OpenMesh::Decimater::ModHandleT<Self>;
  using Base         = OpenMesh::Decimater::ModBaseT<TriMesh>;
  using Mesh         = Base::Mesh;
  using CollapseInfo = Base::CollapseInfo;
  using Quadric      = pq::quadric<pq::math<float, glm::vec3, glm::vec3, glm::mat3>>;

  virtual const std::string& name() const override
  {
    static std ::string _s_modname_("ProbabilisticQuadric");
    return _s_modname_;
  }

public:
  explicit ModProbQuadricT(TriMesh& mesh)
      : Base(mesh, false)
  {
    unset_max_err();
    Base::mesh().add_property(mVertQuadrics);
  }

  virtual ~ModProbQuadricT() { Base::mesh().remove_property(mVertQuadrics); }

public:  // Specific methods.
  void set_max_err(double err)
  {
    mMaxErr = err;
    Base::set_binary(false);
  }

  void unset_max_err() { set_max_err(DBL_MAX); }

  float max_err() const { return mMaxErr; }

public:  // Inherited
  virtual void initialize(void) override
  {
    TriMesh&                      mesh = Base::mesh();
    OpenMesh::EPropHandleT<float> edgelen;
    mesh.add_property(edgelen);
    // Compute edge lengths.
    tbb::parallel_for_each(mesh.edges(), [&](TriMesh::EdgeH eh) {
      mesh.property(edgelen, eh) = mesh.calc_edge_length(eh);
    });
    OpenMesh::VPropHandleT<float> stddev;
    mesh.add_property(stddev);
    // Compute standard deviations of vertices.
    tbb::parallel_for_each(mesh.vertices(), [&](TriMesh::VertH vh) {
      mesh.property(stddev, vh) =
        0.1f *
        std::accumulate(mesh.cve_begin(vh),
                        mesh.cve_end(vh),
                        0.f,
                        [&](float total, TriMesh::EdgeH eh) {
                          return total + mesh.property(edgelen, eh);
                        }) /
        float(std::distance(mesh.cve_begin(vh), mesh.cve_end(vh)));
    });
    mesh.remove_property(edgelen);
    OpenMesh::FPropHandleT<Quadric> fqprop;
    mesh.add_property(fqprop);
    // Compute quadrics.
    tbb::parallel_for_each(mesh.faces(), [&](TriMesh::FaceH fh) {
      std::array<glm::vec3, 3> pos;
      std::transform(
        mesh.cfv_begin(fh), mesh.cfv_end(fh), pos.begin(), [&](TriMesh::VertH vh) {
          return mesh.point(vh);
        });
      float sigma = std::accumulate(mesh.cfv_begin(fh),
                                    mesh.cfv_end(fh),
                                    0.f,
                                    [&](float total, TriMesh::VertH vh) {
                                      return total + mesh.property(stddev, vh);
                                    }) /
                    3.f;
      mesh.property(fqprop, fh) =
        Quadric::probabilistic_triangle_quadric(pos[0], pos[1], pos[2], sigma);
    });
    mesh.remove_property(stddev);
    tbb::parallel_for_each(mesh.vertices(), [&](TriMesh::VertH vh) {
      mesh.property(mVertQuadrics, vh) =
        std::accumulate(mesh.cvf_begin(vh),
                        mesh.cvf_end(vh),
                        Quadric(),
                        [&](Quadric total, TriMesh::FaceH fh) {
                          return total + mesh.property(fqprop, fh);
                        });
    });
    mesh.remove_property(fqprop);
  }

  virtual float collapse_priority(const CollapseInfo& ci) override
  {
    const TriMesh& mesh = Base::mesh();
    if (mesh.is_boundary(ci.v0) && !mesh.is_boundary(ci.v1)) {
      return ILLEGAL_COLLAPSE;
    }
    Quadric q = mesh.property(mVertQuadrics, ci.v0) + mesh.property(mVertQuadrics, ci.v1);
    return q(q.minimizer()) * glm::distance(mesh.point(ci.v0), mesh.point(ci.v1));
  }

  virtual void postprocess_collapse(const CollapseInfo& ci) override
  {
    TriMesh& mesh = Base::mesh();
    mesh.property(mVertQuadrics, ci.v1) += mesh.property(mVertQuadrics, ci.v0);
    if (!mesh.is_boundary(ci.v1)) {
      mesh.point(ci.v1) = mesh.property(mVertQuadrics, ci.v1).minimizer();
    }
  }

private:
  float                           mMaxErr;
  OpenMesh::VPropHandleT<Quadric> mVertQuadrics;
};

TriMesh decimate(TriMesh mesh, int nCollapses)
{
  OpenMesh::Decimater::DecimaterT<TriMesh> decimater(mesh);
  ModProbQuadricT::Handle                  quadric;
  decimater.add(quadric);
  decimater.module(quadric).unset_max_err();
  decimater.initialize();
  decimater.decimate(nCollapses);
  decimater.mesh().garbage_collection();
  return decimater.mesh();
}

}  // namespace gal
