#pragma once

#include <numeric>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <galview/Context.h>

namespace gal {
namespace view {

class TextTags : public Drawable
{
public:
  using ValueType = std::vector<std::pair<glm::vec3, std::string>>;
  friend struct MakeDrawable<ValueType>;

  TextTags() = default;
  ~TextTags();

  void draw() const override;

private:
  uint32_t mVAO;
  uint32_t mVBO;
  uint32_t mVSize;
};

const glm::ivec2&           charbearing(char c);
const glm::ivec2&           charsize(char c);
uint32_t                    charadvance(char c);
const std::array<float, 4>& chartexcoords(char c);

struct CharVertex
{
  glm::vec3 position  = glm::vec3 {0.f, 0.f, 0.f};
  glm::vec2 offset    = glm::vec2 {0.f, 0.f};
  glm::vec2 texCoords = glm::vec2 {0.f, 0.f};

  static void initAttributes();
};

using CharVertexBuffer = glutil::TVertexBuffer<CharVertex>;

template<>
struct MakeDrawable<TextTags::ValueType> : public std::true_type
{
  static std::shared_ptr<Drawable> get(const TextTags::ValueType&   tags,
                                       std::vector<RenderSettings>& renderSettings)
  {
    auto view = std::make_shared<TextTags>();

    CharVertexBuffer vBuf(
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

    view->mVSize = vBuf.size();
    vBuf.finalize(view->mVAO, view->mVBO);

    static constexpr glm::vec4 sPointColor = {1.f, 0.f, 0.f, 1.f};
    RenderSettings             settings;
    settings.pointColor = sPointColor;
    settings.shaderId   = Context::get().shaderId("text");
    renderSettings.push_back(settings);

    return view;
  }
};

}  // namespace view
}  // namespace gal
