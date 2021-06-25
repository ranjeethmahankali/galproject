#include <string>
#include <vector>

#include <galcore/Serialization.h>

namespace gal {

using PositionalTextType = std::pair<glm::vec3, std::string>;

class Annotations : public std::vector<PositionalTextType>
{
public:
  Annotations() = default;
  Annotations(std::vector<PositionalTextType> tags) { *this = std::move(tags); }
};

template<>
struct Serial<Annotations> : public std::true_type
{
  static Annotations deserialize(Bytes& bytes)
  {
    std::vector<PositionalTextType> tags;
    bytes >> tags;
    return Annotations(std::move(tags));
  }

  static Bytes serialize(const Annotations& tags)
  {
    Bytes bytes;
    bytes << (const std::vector<PositionalTextType>&)tags;
    return bytes;
  }
};

}  // namespace gal