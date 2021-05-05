#include <galcore/Serialization.h>
#include <fstream>

namespace gal {

Bytes::Bytes()
{
  mData.reserve(5120);
};

Bytes& Bytes::writeBytes(const char* src, size_t nBytes)
{
  std::copy(src, src + nBytes, std::back_inserter(mData));
  return *this;
}

Bytes& Bytes::readBytes(size_t nBytes, char* dst)
{
  const char* src = mData.data() + mReadPos;
  mReadPos += nBytes;
  if (mReadPos > mData.size()) {
    throw std::out_of_range("Out of bounds while reading bytes!");
  }
  std::copy(src, src + nBytes, dst);
  return *this;
}

Bytes& Bytes::writeNested(Bytes nested)
{
  write(uint64_t(nested.mData.size()));
  return writeBytes(nested.mData.data(), nested.mData.size());
}

Bytes& Bytes::readNested(Bytes& nested)
{
  uint64_t size = 0;
  read(size);
  nested.mData.resize(size);
  return readBytes(size, nested.mData.data());
}

void Bytes::saveToFile(const fs::path& path) const
{
  std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
  file.write(mData.data(), mData.size());
  file.close();
}

Bytes Bytes::loadFromFile(const fs::path& filepath)
{
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  auto          pos = file.tellg();
  Bytes         bytes;
  bytes.mData.resize(size_t(pos));

  file.seekg(0, std::ios::beg);
  file.read(bytes.mData.data(), pos);

  file.close();

  return bytes;
}

}  // namespace gal