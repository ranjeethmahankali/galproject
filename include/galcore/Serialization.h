#pragma once
#include <stdint.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace gal {

class Bytes;

namespace fs = std::filesystem;

template<typename T>
struct IsValueType : public std::is_fundamental<T>
{
};

template<>
struct IsValueType<glm::vec3> : public std::true_type
{
};

template<>
struct IsValueType<glm::vec2> : public std::true_type
{
};

template<typename T>
struct Serial : std::false_type
{
  static T     deserialize(Bytes& bytes);
  static Bytes serialize(const T& data);
};

class Bytes
{
public:
  Bytes();
  Bytes(uint32_t version);
  Bytes(const fs::path& filepath);

private:
  std::vector<char> mData;
  size_t            mReadPos = 0;

public:
  uint32_t version() const noexcept;

  void saveToFile(const fs::path& path) const;

  template<typename T>
  Bytes& write(const T& data)
  {
    static_assert(IsValueType<T>::value, "Must be a fundamental type");
    const uint8_t* ptr = (uint8_t*)(&data);
    std::copy(ptr, ptr + sizeof(T), std::back_inserter(mData));
    return *this;
  };

  template<typename T>
  Bytes& read(T& data)
  {
    static_assert(IsValueType<T>::value, "Must be a fundamental type");
    char* dst    = (char*)(&data);
    char* src    = mData.data() + mReadPos;
    size_t   offset = sizeof(T);
    mReadPos += offset;
    if (mReadPos > mData.size()) {
      throw std::out_of_range("Out of bounds while reading bytes!");
    }
    std::copy(src, src + offset, dst);
    return *this;
  };

  Bytes& writeBytes(const char* src, size_t nBytes);

  Bytes& readBytes(size_t nBytes, char* dst);

  Bytes& writeNested(Bytes nested);

  Bytes& readNested(Bytes& nested);

  template<typename T>
  Bytes& operator<<(const T& data)
  {
    if constexpr (IsValueType<T>::value) {
      return write(data);
    }
    else if constexpr (Serial<T>::value) {
      return writeNested(Serial<T>::serialize(data));
    }
    else {
      throw std::runtime_error("Don't know how to serialize this type");
    }
  };

  template<typename T>
  Bytes& operator>>(T& data)
  {
    if constexpr (IsValueType<T>::value) {
      return read(data);
    }
    else if constexpr (Serial<T>::value) {
      Bytes nested;
      readNested(nested);
      data = Serial<T>::deserialize(nested);
      return *this;
    }
    else {
      throw std::runtime_error("Don't know how to deserialize this type");
    }
  };
};

}  // namespace gal