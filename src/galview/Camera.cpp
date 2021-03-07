#include <galview/Camera.h>
#include <glm/gtc/matrix_transform.hpp>

namespace gal {
namespace view {

Camera::Camera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
    : mEyePos {eye}
    , mTarget {target}
    , mUp {up}
{}

glm::mat4 Camera::projMatrix() const
{
  switch (mType) {
  case Type::perspective:
    return glm::perspective(mFOVY, mAspect, mNear, mFar);
  case Type::orthographic:
    return glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
  }
  return glm::identity<glm::mat4>();
}

glm::mat4 Camera::viewMatrix() const
{
  return glm::lookAt(mEyePos, mTarget, mUp);
}

}  // namespace view
}  // namespace gal