#include <galcore/Serialization.h>

namespace gal {

Bytes::Bytes()
{
  mData.reserve(5120);
};

Bytes::Bytes(uint32_t version)
    : mVersion(version)
{
  mData.reserve(5120);
}

uint32_t Bytes::version() const noexcept
{
  return mVersion;
}

Bytes& Bytes::writeBytes(const uint8_t* src, size_t nBytes)
{
  std::copy(src, src + nBytes, std::back_inserter(mData));
  return *this;
}

Bytes& Bytes::readBytes(size_t nBytes, uint8_t* dst)
{
  const uint8_t* src = mData.data() + mReadPos;
  mReadPos += nBytes;
  if (mReadPos > mData.size()) {
    throw std::out_of_range("Out of bounds while reading bytes!");
  }
  std::copy(src, src + nBytes, dst);
  return *this;
}

Bytes& Bytes::writeNested(Bytes nested)
{
  write(nested.mVersion);
  write(uint64_t(nested.mData.size()));
  return writeBytes(nested.mData.data(), nested.mData.size());
}

Bytes& Bytes::readNested(Bytes& nested)
{
  read(nested.mVersion);
  uint64_t size = 0;
  read(size);
  nested.mData.resize(size);
  return readBytes(size, nested.mData.data());
}

}  // namespace gal