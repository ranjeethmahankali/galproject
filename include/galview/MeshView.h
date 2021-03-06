#pragma once

#include <galcore/Mesh.h>
#include <galview/GLUtil.h>
#include <array>

namespace gal {

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

  MeshView(MeshView&& other) = default;

private:
  uint mVao   = 0;  // vertex array object.
  uint mVBO   = 0;  // vertex buffer object.
  uint mIBO   = 0;  // index buffer object.
  uint mVSize = 0;  // vertex buffer size.
  uint mISize = 0;  // index buffer size.
};

}  // namespace gal
