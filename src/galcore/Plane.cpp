#include <galcore/Plane.h>

namespace gal {

Plane::Plane()
    : Plane(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f))
{}

Plane::Plane(const glm::vec3& origin, const glm::vec3& normal)
    : mOrigin(origin)
{
  setNormal(normal);
};

const glm::vec3& Plane::origin() const
{
  return mOrigin;
};

void Plane::origin(const glm::vec3& o)
{
  mOrigin = o;
}

const glm::vec3& Plane::normal() const
{
  return mNormal;
};

void Plane::normal(const glm::vec3& n)
{
  mNormal = n;
}

void Plane::setOrigin(const glm::vec3& origin)
{
  mOrigin = origin;
};

void Plane::setNormal(const glm::vec3& normal)
{
  static constexpr glm::vec3 sXAxis = {1.0f, 0.0f, 0.0f};
  static constexpr glm::vec3 sYAxis = {0.0f, 1.0f, 0.0f};
  static constexpr glm::vec3 sZAxis = {0.0f, 0.0f, 1.0f};
  if (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f) {
    return;
  }
  mNormal = glm::normalize(normal);

  if (sZAxis == mNormal) {
    mXAxis = glm::normalize(sXAxis);
    mYAxis = glm::normalize(sYAxis);
  }
  else if (sZAxis == -mNormal) {
    mXAxis = glm::normalize(-sXAxis);
    mYAxis = glm::normalize(-sYAxis);
  }
  else {
    mXAxis = glm::normalize(glm::cross(mNormal, sZAxis));
    mYAxis = glm::normalize(glm::cross(mNormal, mXAxis));
  }
};

const glm::vec3& Plane::xaxis() const
{
  return mXAxis;
};

const glm::vec3& Plane::yaxis() const
{
  return mYAxis;
};

}  // namespace gal