#pragma once

#include <galcore/Mesh.h>
#include <galfunc/MapMacro.h>
#include <string.h>
#include <boost/python.hpp>
#include <functional>
#include <glm/glm.hpp>

namespace gal {
namespace func {

glm::vec3 meshCentroid(std::shared_ptr<gal::Mesh> mesh);

std::shared_ptr<gal::Mesh> loadObjFile(const std::string& filepath);

}  // namespace func
}  // namespace gal
