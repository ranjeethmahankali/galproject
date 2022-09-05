#pragma once
#include <glm/glm.hpp>

namespace gal {

struct Plane
{
  Plane();
  Plane(const glm::vec3& origin, const glm::vec3& normal);

  /**
   * @brief Gets the origin
   * @return const glm::vec3& origin.
   */
  const glm::vec3& origin() const;

  /**
   * @brief Sets the origin to the given value.
   * @param newOrigin
   */
  void origin(const glm::vec3& newOrigin);

  /**
   * @brief Gets the normal.
   * @return const glm::vec3& normal.
   */
  const glm::vec3& normal() const;

  /**
   * @brief Sets the normal to the new value.
   * @param n new normal.
   */
  void normal(const glm::vec3& n);

  const glm::vec3& xaxis() const;
  const glm::vec3& yaxis() const;

private:
  glm::vec3 mOrigin;
  glm::vec3 mNormal;
  glm::vec3 mXAxis;
  glm::vec3 mYAxis;
};

}  // namespace gal