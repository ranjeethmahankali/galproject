#pragma once

#include <numeric>
#include <type_traits>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <Annotations.h>
#include <Box.h>
#include <Context.h>

namespace gal {
namespace view {

void unbindTexture();

void              bindCharAtlasTexture();
const glm::ivec2& charbearing(char c);
const glm::ivec2& charsize(char c);
uint32_t          charadvance(char c);
const glm::vec4&  chartexcoords(char c);

void                 bindGlyphAtlasTexture();
std::vector<int32_t> loadGlyphs(const std::vector<fs::path>& labeledPNGPaths);
const glm::vec4&     glyphtexcoords(size_t i);
glm::ivec2           glyphSize(size_t i);

struct AnnotationVertex
{
  glm::vec3 position  = glm::vec3 {0.f, 0.f, 0.f};
  glm::vec2 offset    = glm::vec2 {0.f, 0.f};
  glm::vec2 texCoords = glm::vec2 {0.f, 0.f};

  static void initAttributes();
};

using AnnotationVertBuffer = glutil::TVertexBuffer<AnnotationVertex>;

template<>
struct Drawable<Annotations<std::string>> : public std::true_type
{
  static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};

  using AnnotationsT = SafeInstanceType<Annotations<std::string>>;

private:
  AnnotationVertBuffer mVBuf;
  Box3                 mBounds;

public:
  void update(const std::vector<AnnotationsT>& tags)
  {
    mVBuf.resize(std::accumulate(
      tags.begin(), tags.end(), size_t(0), [](size_t total0, const AnnotationsT& t) {
        return total0 +
               6 * std::accumulate(
                     t.begin(),
                     t.end(),
                     size_t(0),
                     [](size_t total, const std::pair<glm::vec3, std::string>& tag) {
                       return total + tag.second.size();
                     });
      }));

    mBounds     = gal::Box3();
    auto vbegin = mVBuf.begin();
    for (const auto& ann : tags) {
      for (const auto& tag : ann) {
        float x = 0.f;
        float y = 0.f;
        mBounds.inflate(tag.first);
        for (char c : tag.second) {
          const auto& b    = charbearing(c);
          const auto& s    = charsize(c);
          const auto& a    = charadvance(c);
          const auto& tc   = chartexcoords(c);
          float       xpos = x + (float(b.x) / 1920.f);
          float       ypos = y - (float(s.y - b.y) / 1080.f);
          float       w    = float(s.x) / 1920.f;
          float       h    = float(s.y) / 1080.f;

          *(vbegin++) = {tag.first, {xpos, ypos + h}, {tc[0], tc[1]}};
          *(vbegin++) = {tag.first, {xpos, ypos}, {tc[0], tc[3]}};
          *(vbegin++) = {tag.first, {xpos + w, ypos}, {tc[2], tc[3]}};
          *(vbegin++) = {tag.first, {xpos, ypos + h}, {tc[0], tc[1]}};
          *(vbegin++) = {tag.first, {xpos + w, ypos}, {tc[2], tc[3]}};
          *(vbegin++) = {tag.first, {xpos + w, ypos + h}, {tc[2], tc[1]}};

          x += float(a >> 6) / 1920.f;
        }
      }
    }

    mVBuf.alloc();
  }

  Box3 bounds() const { return mBounds; }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = uint64_t((1.f - sPointColor.a) * 255.f);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.pointColor = sPointColor;
    settings.shaderId   = Context::get().shaderId("text");
    return settings;
  }

  void draw() const
  {
    static RenderSettings rsettings = renderSettings();
    rsettings.apply();
    Context::get().setUniform("textColor", glm::vec3 {1.f, 1.f, 1.f});
    bindCharAtlasTexture();
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
    unbindTexture();
  }
};

template<>
class Drawable<Annotations<Glyph>> : public std::true_type
{
  static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};

  using AnnotationsT = SafeInstanceType<Annotations<Glyph>>;

private:
  AnnotationVertBuffer mVBuf;
  Box3                 mBounds;

public:
  void update(const std::vector<AnnotationsT>& tags)
  {
    mVBuf.resize(std::accumulate(
      tags.begin(), tags.end(), size_t(0), [](size_t total, const AnnotationsT& t) {
        return total + 6 * t.size();
      }));

    mBounds     = gal::Box3();
    auto vbegin = mVBuf.begin();
    for (const auto& ann : tags) {
      for (const auto& tag : ann) {
        float x = 0.f;
        float y = 0.f;
        mBounds.inflate(tag.first);
        const auto& tc    = glyphtexcoords(tag.second.mIndex);
        auto        isize = glyphSize(tag.second.mIndex);
        glm::vec2   size  = {float(isize.x) / 1920.f, float(isize.y) / 1080.f};

        *(vbegin++) = {tag.first, {0.f, size.y}, {tc[0], tc[1]}};
        *(vbegin++) = {tag.first, {0.f, 0.f}, {tc[0], tc[3]}};
        *(vbegin++) = {tag.first, {size.x, 0.f}, {tc[2], tc[3]}};
        *(vbegin++) = {tag.first, {0.f, size.y}, {tc[0], tc[1]}};
        *(vbegin++) = {tag.first, {size.x, 0.f}, {tc[2], tc[3]}};
        *(vbegin++) = {tag.first, size, {tc[2], tc[1]}};
      }
    }

    mVBuf.alloc();
  }

  Box3 bounds() const { return mBounds; }

  uint64_t drawOrderIndex() const
  {
    static const uint64_t sIdx = uint64_t((1.f - sPointColor.a) * 255.f);
    return sIdx;
  }

  RenderSettings renderSettings() const
  {
    RenderSettings settings;
    settings.pointColor = sPointColor;
    settings.shaderId   = Context::get().shaderId("glyph");
    return settings;
  }

  void draw() const
  {
    static RenderSettings rsettings = renderSettings();
    rsettings.apply();
    bindGlyphAtlasTexture();
    mVBuf.bindVao();
    mVBuf.bindVbo();
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
    unbindTexture();
  }
};

}  // namespace view
}  // namespace gal
