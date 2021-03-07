#pragma once
#include <stdint.h>
#include <iostream>
#include <string>
#include <unordered_map>

namespace gal {
namespace view {

class Shader
{
public:
  static Shader loadFromSources(const std::string& vertSrc, const std::string& fragSrc);
  static Shader loadFromFiles(const std::string& vertFilePath,
                              const std::string& fragFilePath);
  static Shader loadFromName(const std::string& name);

  void use() const;

  ~Shader();

private:
  uint32_t mVertId;
  uint32_t mFragId;
  uint32_t mId;

  Shader(const std::string& vertSrc, const std::string& fragSrc);

  template<typename T>
  void setUniformInternal(int location, T val);

public:
  template<typename T>
  void setUniform(const std::string& name, T val)
  {
    int loc = glGetUniformLocation(mId, name.c_str());
    if (loc == -1) {
      std::cerr << "Uniform " << name << " not found.\n";
      return;
    }
    setUniformInternal<T>(loc, val);
  };
};

}  // namespace view
}  // namespace gal