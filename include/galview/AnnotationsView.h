#pragma once

#include <numeric>
#include <type_traits>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <galcore/Annotations.h>
#include <galview/Context.h>

namespace gal {
namespace view {

void unbindTexture();

void              bindCharAtlasTexture();
const glm::ivec2& charbearing(char c);
const glm::ivec2& charsize(char c);
uint32_t          charadvance(char c);
const glm::vec4&  chartexcoords(char c);

void   bindGlyphAtlasTexture();
void   loadGlyphs(const std::vector<std::pair<std::string, fs::path>>& labeledPNGPaths);
size_t getGlyphIndex(const std::string& label);
const glm::vec4& glyphtexcoords(size_t i);
glm::ivec2       glyphSize(size_t i);

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
private:
  Box3     mBounds;
  uint32_t mVAO;
  uint32_t mVBO;
  uint32_t mVSize;

public:
  Drawable(const Annotations<std::string>& tags)
  {
    AnnotationVertBuffer vBuf(
      6 * std::accumulate(tags.begin(),
                          tags.end(),
                          0ULL,
                          [](size_t total, const std::pair<glm::vec3, std::string>& tag) {
                            return total + tag.second.size();
                          }));

    auto vbegin = vBuf.begin();
    for (const auto& tag : tags) {
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

    mVSize = vBuf.size();
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<Annotations<std::string>>()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
    }
  }

  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;

  const Drawable& operator=(Drawable&& other)
  {
    mVAO = std::exchange(other.mVAO, 0);
    mVBO = std::exchange(other.mVBO, 0);
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  Box3 bounds() const { return mBounds; }

  static RenderSettings settings()
  {
    static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};
    RenderSettings             settings;
    settings.pointColor = sPointColor;
    settings.shaderId   = Context::get().shaderId("glyph");
    return settings;
  }

  void draw() const
  {
    static RenderSettings rsettings = settings();
    Context::get().setUniform("textColor", glm::vec3 {1.f, 1.f, 1.f});
    bindCharAtlasTexture();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVSize));
    unbindTexture();
  }
};

template<>
class Drawable<Annotations<Glyph>> : public std::true_type
{
private:
  Box3     mBounds;
  uint32_t mVAO;
  uint32_t mVBO;
  uint32_t mVSize;

public:
  Drawable(const Annotations<Glyph>& tags)
  {
    AnnotationVertBuffer vBuf(6 * tags.size());

    auto vbegin = vBuf.begin();
    for (const auto& tag : tags) {
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

    mVSize = vBuf.size();
    vBuf.finalize(mVAO, mVBO);
  }

  ~Drawable<Annotations<Glyph>>()
  {
    if (mVAO) {
      GL_CALL(glDeleteVertexArrays(1, &mVAO));
    }
    if (mVBO) {
      GL_CALL(glDeleteBuffers(1, &mVBO));
    }
  }

  Drawable(const Drawable&) = delete;
  const Drawable& operator=(const Drawable&) = delete;

  const Drawable& operator=(Drawable&& other)
  {
    mVAO = std::exchange(other.mVAO, 0);
    mVBO = std::exchange(other.mVBO, 0);
    return *this;
  }
  Drawable(Drawable&& other) { *this = std::move(other); }

  Box3 bounds() const { return mBounds; }

  static RenderSettings settings()
  {
    static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};
    RenderSettings             settings;
    settings.pointColor = sPointColor;
    settings.shaderId   = Context::get().shaderId("text");
    return settings;
  }

  void draw() const
  {
    static RenderSettings rsettings = settings();
    rsettings.apply();
    bindGlyphAtlasTexture();
    GL_CALL(glBindVertexArray(mVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVSize));
    unbindTexture();
  }
};

}  // namespace view
}  // namespace gal
