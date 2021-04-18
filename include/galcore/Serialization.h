#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace gal {

class Bytes;

template<typename T>
struct IsValueType : public std::is_fundamental<T>
{
};

template<>
struct IsValueType<glm::vec3> : public std::true_type
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

private:
  std::vector<uint8_t> mData;
  size_t               mReadPos = 0;
  uint32_t             mVersion = 0;

public:
  uint32_t version() const noexcept;

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
    uint8_t* dst    = (uint8_t*)(&data);
    uint8_t* src    = mData.data() + mReadPos;
    size_t   offset = sizeof(T);
    mReadPos += offset;
    if (mReadPos > mData.size()) {
      throw std::out_of_range("Out of bounds while reading bytes!");
    }
    std::copy(src, src + offset, dst);
    return *this;
  };

  Bytes& writeBytes(const uint8_t* src, size_t nBytes);

  Bytes& readBytes(size_t nBytes, uint8_t* dst);

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

template<typename T>
struct Serial<std::vector<T>>
{
  static constexpr bool value = Serial<T>::value;
  static std::vector<T> deserialize(Bytes& bytes)
  {
    uint64_t size = 0;
    bytes >> size;
    std::vector<T> res(size);
    for (auto& v : res) {
      bytes >> v;
    }
    return res;
  }

  static Bytes serialize(const std::vector<T>& data)
  {
    Bytes ser;
    ser << uint64_t(data.size());
    for (const auto& v : data) {
      ser << v;
    }
    return ser;
  }
};

}  // namespace gal