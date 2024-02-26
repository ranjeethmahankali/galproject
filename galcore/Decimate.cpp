#include <Decimate.h>

#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <glm/geometric.hpp>
#include <stdexcept>
#include "ProbabilisticQuadrics.h"

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
    Base::mesh().add_property(mQuadrics);
    Base::mesh().add_property(mStdDev);
  }

  virtual ~ModProbQuadricT()
  {
    Base::mesh().remove_property(mQuadrics);
    Base::mesh().remove_property(mStdDev);
  }

public:  // Inherited
  virtual void initialize(void) override
  {
    static constexpr float EDGE_RATIO = 0.25f;
    TriMesh&               mesh       = Base::mesh();
    // Compute standard deviations of vertices.
    for (TriMesh::VertH vh : mesh.vertices()) {
      const glm::vec3& p0   = mesh.point(vh);
      float&           mean = mesh.property(mStdDev, vh);
      mean                  = 0.f;
      float denom           = 0.f;
      for (TriMesh::VertH nvh : mesh.vv_range(vh)) {
        mean += EDGE_RATIO * glm::length(mesh.point(nvh) - p0);
        denom += 1.f;
      }
      mean /= denom;
    }
    // Compute quadrics.
    for (TriMesh::FaceH fh : mesh.faces()) {
      std::array<glm::vec3, 3> pos;
      std::transform(
        mesh.cfv_begin(fh), mesh.cfv_end(fh), pos.begin(), [&](TriMesh::VertH vh) {
          return mesh.point(vh);
        });
      float sigma = std::accumulate(mesh.cfv_begin(fh),
                                    mesh.cfv_end(fh),
                                    0.f,
                                    [&](float total, TriMesh::VertH vh) {
                                      return total + mesh.property(mStdDev, vh);
                                    }) /
                    3.f;
      Quadric q = Quadric::probabilistic_triangle_quadric(pos[0], pos[1], pos[2], sigma);
      for (TriMesh::VertH vh : mesh.fv_range(fh)) {
        mesh.property(mQuadrics, vh) += q;
      }
    }
  }

  virtual float collapse_priority(const CollapseInfo& ci) override
  {
    Quadric q = Base::mesh().property(mQuadrics, ci.v0);
    q += Base::mesh().property(mQuadrics, ci.v1);
    return q(q.minimizer());
  }

  virtual void postprocess_collapse(const CollapseInfo& ci) override
  {
    Base::mesh().property(mQuadrics, ci.v1) += Base::mesh().property(mQuadrics, ci.v0);
    Base::mesh().point(ci.v1) = Base::mesh().property(mQuadrics, ci.v1).minimizer();
  }

public:  // Specific methods.
  void set_max_err(double err)
  {
    mMaxErr = err;
    Base::set_binary(false);
  }

  void unset_max_err() { set_max_err(DBL_MAX); }

  float max_err() const { return mMaxErr; }

private:
  float                           mMaxErr;
  OpenMesh::VPropHandleT<float>   mStdDev;
  OpenMesh::VPropHandleT<Quadric> mQuadrics;
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
