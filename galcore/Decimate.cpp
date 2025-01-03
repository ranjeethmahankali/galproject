#include <Decimate.h>

#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <glm/geometric.hpp>
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

  const std::string& name() const override
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
    Base::mesh().add_property(mStdDev);
    Base::mesh().add_property(mFaceQuadrics);
    Base::mesh().add_property(mEdgeLengths);
  }

  virtual ~ModProbQuadricT()
  {
    Base::mesh().remove_property(mVertQuadrics);
    Base::mesh().remove_property(mStdDev);
    Base::mesh().remove_property(mFaceQuadrics);
    Base::mesh().remove_property(mEdgeLengths);
  }

  ModProbQuadricT(const ModProbQuadricT&)            = delete;
  ModProbQuadricT(ModProbQuadricT&&)                 = delete;
  ModProbQuadricT& operator=(const ModProbQuadricT&) = delete;
  ModProbQuadricT& operator=(ModProbQuadricT&&)      = delete;

public:  // Specific methods.
  void set_max_err(float err)
  {
    mMaxErr = err;
    Base::set_binary(false);
  }

  void unset_max_err() { set_max_err(DBL_MAX); }

  float max_err() const { return mMaxErr; }

  void compute_edge_length(EdgeH eh)
  {
    Base::mesh().property(mEdgeLengths, eh) = Base::mesh().calc_edge_length(eh);
  }

  void compute_stddev(VertH vh)
  {
    TriMesh&               mesh       = Base::mesh();
    static constexpr float EDGE_RATIO = 0.1f;
    mesh.property(mStdDev, vh) =
      EDGE_RATIO *
      std::accumulate(
        mesh.cve_begin(vh),
        mesh.cve_end(vh),
        0.f,
        [&](float total, EdgeH eh) { return total + mesh.property(mEdgeLengths, eh); }) /
      float(std::distance(mesh.cve_begin(vh), mesh.cve_end(vh)));
  }

  void compute_face_quadric(FaceH fh)
  {
    TriMesh&                 mesh = Base::mesh();
    std::array<glm::vec3, 3> pos {};
    std::transform(mesh.cfv_begin(fh), mesh.cfv_end(fh), pos.begin(), [&](VertH vh) {
      return mesh.point(vh);
    });
    float sigma = std::accumulate(mesh.cfv_begin(fh),
                                  mesh.cfv_end(fh),
                                  0.f,
                                  [&](float total, VertH vh) {
                                    return total + mesh.property(mStdDev, vh);
                                  }) /
                  3.f;
    mesh.property(mFaceQuadrics, fh) =
      Quadric::probabilistic_triangle_quadric(pos[0], pos[1], pos[2], sigma);
  }

public:  // Inherited
  void initialize(void) override
  {
    TriMesh& mesh = Base::mesh();
    // Compute edge lengths.
    for (EdgeH eh : mesh.edges()) {
      compute_edge_length(eh);
    }
    // Compute standard deviations of vertices.
    for (VertH vh : mesh.vertices()) {
      compute_stddev(vh);
    }
    // Compute quadrics.
    for (FaceH fh : mesh.faces()) {
      compute_face_quadric(fh);
      for (VertH vh : mesh.fv_range(fh)) {
        mesh.property(mVertQuadrics, vh) += mesh.property(mFaceQuadrics, fh);
      }
    }
  }

  float collapse_priority(const CollapseInfo& ci) override
  {
    Quadric q = Base::mesh().property(mVertQuadrics, ci.v0);
    q += Base::mesh().property(mVertQuadrics, ci.v1);
    return q(q.minimizer());
  }

  void postprocess_collapse(const CollapseInfo& ci) override
  {
    TriMesh& mesh = Base::mesh();
    mesh.property(mVertQuadrics, ci.v1) += mesh.property(mVertQuadrics, ci.v0);
    mesh.point(ci.v1) = mesh.property(mVertQuadrics, ci.v1).minimizer();
    // Compute edge lengths.
    for (EdgeH eh : mesh.ve_range(ci.v1)) {
      compute_edge_length(eh);
    }
    // Compute standard deviations.
    compute_stddev(ci.v1);
    for (VertH vh : mesh.vv_range(ci.v1)) {
      compute_stddev(vh);
      mesh.property(mVertQuadrics, vh) = Quadric();
    }
    // Compute face quadrics.
    for (FaceH fh : mesh.vf_range(ci.v1)) {
      compute_face_quadric(fh);
    }
    mesh.property(mVertQuadrics, ci.v1) = Quadric();
    for (FaceH fh : mesh.vf_range(ci.v1)) {
      mesh.property(mVertQuadrics, ci.v1) += mesh.property(mFaceQuadrics, fh);
    }
    for (VertH vh : mesh.vv_range(ci.v1)) {
      Quadric& q = mesh.property(mVertQuadrics, vh);
      q          = Quadric();
      for (FaceH vfh : mesh.vf_range(vh)) {
        q += mesh.property(mFaceQuadrics, vfh);
      }
    }
  }

private:
  float                           mMaxErr = 0.f;
  OpenMesh::VPropHandleT<float>   mStdDev;
  OpenMesh::EPropHandleT<float>   mEdgeLengths;
  OpenMesh::VPropHandleT<Quadric> mVertQuadrics;
  OpenMesh::FPropHandleT<Quadric> mFaceQuadrics;
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
