#pragma once

#include <galcore/Mesh.h>
#include <galview/GLUtil.h>
#include <array>

namespace gal {
namespace view {

class MeshView
{
public:
  static MeshView create(const Mesh& mesh);

  ~MeshView();

  void draw() const;

private:
  MeshView()                = default;
  MeshView(const MeshView&) = delete;
  const MeshView& operator=(const MeshView&) = delete;

  MeshView(MeshView&&) = default;

private:
  uint mVAO   = 0;
  uint mVBO   = 0;
  uint mVSize = 0;
};

class SmoothMeshView
{
public:
  static SmoothMeshView create(const Mesh& mesh);
  ~SmoothMeshView();

  void draw() const;

private:
  SmoothMeshView()                      = default;
  SmoothMeshView(const SmoothMeshView&) = delete;
  const SmoothMeshView& operator=(const SmoothMeshView&) = delete;

  SmoothMeshView(SmoothMeshView&& other) = default;

private:
  uint mVAO   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mIBO   = 0;  // index buffer object.
  uint mVSize = 0;  // vertex buffer size.
  uint mISize = 0;  // index buffer size.
};

}  // namespace view
}  // namespace gal
