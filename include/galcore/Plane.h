#pragma once
#include <glm/glm.hpp>

namespace gal {

struct Plane
{
  Plane(const glm::vec3& origin, const glm::vec3& normal);

  const glm::vec3& origin() const;
  const glm::vec3& normal() const;
  const glm::vec3& xaxis() const;
  const glm::vec3& yaxis() const;

  void setOrigin(const glm::vec3&);
  void setNormal(const glm::vec3&);

private:
  glm::vec3 mOrigin;
  glm::vec3 mNormal;
  glm::vec3 mXAxis;
  glm::vec3 mYAxis;
};

}  // namespace gal