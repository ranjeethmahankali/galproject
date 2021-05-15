#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

namespace gal {
namespace view {
namespace text {

struct Character
{
  uint32_t   textureID;  // ID handle of the glyph texture
  glm::ivec2 size;       // Size of glyph
  glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
  uint32_t   advance;    // Offset to advance to next glyph
};

int initFreeType();
}  // namespace text

}  // namespace view
}  // namespace gal