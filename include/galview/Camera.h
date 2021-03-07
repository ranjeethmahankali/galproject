#pragma once

#include <glm/glm.hpp>
#include <variant>

namespace gal {
namespace view {

struct Camera
{
public:
  enum class Type
  {
    orthographic,
    perspective
  };

  glm::vec3 mEyePos;
  glm::vec3 mTarget;
  glm::vec3 mUp;

  // Orthographic params
  float mLeft   = -2.0f;
  float mRight  = 2.0f;
  float mTop    = 1.1f;
  float mBottom = -1.1f;

  // Perspective params
  float mFOVY   = 1.25f;  // Field of view in y direction in radians.
  float mAspect = 1.8f;   // Aspect ratio of the view.

  // Common params
  float mNear = 0.01f;   // Near clipping plane.
  float mFar  = 100.0f;  // Far clipping plane

  Type mType = Type::perspective;

  Camera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

  glm::mat4 projMatrix() const;
  glm::mat4 viewMatrix() const;
};

}  // namespace view
}  // namespace gal