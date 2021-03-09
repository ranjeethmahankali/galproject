#include <galcore/ObjLoader.h>
#include <galcore/Util.h>
#include <algorithm>
#include <fstream>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace gal {
namespace io {

using CStrIter = std::string::const_iterator;

static void findWord(const std::string& line,
                     char               delim,
                     size_t             begin,
                     size_t&            from,
                     size_t&            to)
{
  from = line.find_first_not_of(delim, begin);
  if (from == std::string::npos) {
    to = std::string::npos;
    return;
  }
  to = line.find_first_of(delim, from);
};

static void findWord(const std::string& line, char delim, size_t& from, size_t& to)
{
  findWord(line, delim, 0, from, to);
};

static void findWord(const std::string& line,
                     char               delim,
                     size_t             begin,
                     size_t             end,
                     size_t&            from,
                     size_t&            to)
{
  findWord(line, delim, 0, from, to);
  if (to > end) {
    to = end;
  }
};

static bool checkWord(const std::string& line,
                      size_t             from,
                      size_t             to,
                      const std::string& word)
{
  return std::equal(line.cbegin() + from, line.cbegin() + to, word.cbegin());
}

static void readCoords(const std::string& line, size_t from, float* dst, size_t nCoords)
{
  for (size_t i = 0; i < nCoords; i++) {
    size_t to;
    findWord(line, ' ', from, from, to);
    auto begin = line.cbegin() + from;
    auto end   = to < line.length() ? line.cbegin() + to : line.cend();
    dst[i]     = std::stof(std::string(begin, end));
    from       = to + 1;
  }
}

static glm::vec3 readVector3(const std::string& line, size_t from)
{
  float coords[3];
  readCoords(line, from, coords, 3);
  return glm::vec3(coords[0], coords[1], coords[2]);
}

static glm::vec2 readVector2(const std::string& line, size_t from)
{
  float coords[2];
  readCoords(line, from, coords, 2);
  return glm::vec2(coords[0], coords[1]);
};

static void readUint32s(const std::string& line,
                        size_t             from,
                        size_t             stop,
                        uint32_t*          dst,
                        size_t             nVals,
                        char               delim)
{
  for (size_t i = 0; i < nVals; i++) {
    size_t to;
    findWord(line, delim, from, from, to);
    auto begin = line.cbegin() + from;
    auto end   = to < line.length() ? line.cbegin() + to : line.cend();
    dst[i]     = std::stoul(std::string(begin, end)) - 1;
    if (to == std::string::npos)
      return;
    from = to + 1;
    if (from >= stop)
      return;
  }
};

static ObjMeshData::Face readFace(const std::string& line, size_t from)
{
  uint32_t vals[9];
  for (size_t i = 0; i < 3; i++) {
    size_t to;
    findWord(line, ' ', from, from, to);
    readUint32s(line, from, to, vals + (i * 3), 3, '/');
    from = to + 1;
  }

  return {{vals[0], vals[3], vals[6]},
          {vals[1], vals[4], vals[7]},
          {vals[2], vals[5], vals[8]}};
};

static std::filesystem::path makeAbsolute(const std::filesystem::path& path)
{
  return path.is_absolute() ? path : std::filesystem::path(utils::absPath(path));
}

ObjMeshData::ObjMeshData(const std::filesystem::path& pathIn, bool flipYZ)
    : mPath(makeAbsolute(pathIn))
{
  std::ifstream file;
  file.open(mPath);
  if (!file) {
    std::cerr << "Unable to open the obj file\n";
  }

  static const std::string sv  = "v";
  static const std::string svt = "vt";
  static const std::string svn = "vn";
  static const std::string sf  = "f";

  std::string line;
  size_t      from  = 0;
  size_t      to    = 0;
  glm::mat4   xform = glm::rotate(float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
  while (std::getline(file, line)) {
    findWord(line, ' ', from, to);
    if (from == std::string::npos)
      continue;
    if (line[from] == '#')
      continue;

    if (to == std::string::npos || to > line.length())
      continue;

    if (checkWord(line, from, to, sv)) {
      glm::vec3 v = readVector3(line, to + 1);
      if (flipYZ)
        v = glm::vec3(xform * glm::vec4 {v.x, v.y, v.z, 1.0f});
      mVertices.push_back(v);
    }
    else if (checkWord(line, from, to, svt)) {
      mTexCoords.push_back(std::move(readVector2(line, to + 1)));
    }
    else if (checkWord(line, from, to, svn)) {
      glm::vec3 v = readVector3(line, to + 1);
      if (flipYZ)
        v = glm::vec3(xform * glm::vec4 {v.x, v.y, v.z, 1.0f});
      mNormals.push_back(v);
    }
    else if (checkWord(line, from, to, sf)) {
      mFaces.push_back(std::move(readFace(line, to + 1)));
    }
    else {
      continue;
    }
  }

  file.close();
};

Mesh ObjMeshData::toMesh() const
{
  std::vector<size_t> indices;
  indices.reserve(mFaces.size() * 3);
  for (const auto& face : mFaces) {
    for (size_t i = 0; i < 3; i++) {
      indices.push_back(face.vertices[i]);
    }
  }
  return Mesh((float*)mVertices.data(), mVertices.size(), indices.data(), mFaces.size());
}

}  // namespace io
}  // namespace gal