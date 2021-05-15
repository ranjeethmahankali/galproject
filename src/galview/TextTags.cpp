#include <galcore/Util.h>
#include <galview/GLUtil.h>
#include <galview/TextTags.h>

#include <array>
#include <iostream>

namespace gal {
namespace view {
namespace text {

static std::array<Character, 128> sCharacters;

int initFreeType()
{
  static const std::string sFontFilePath = utils::absPath("CascadiaMono.ttf").string();
  FT_Library               sFtLib;
  if (FT_Init_FreeType(&sFtLib)) {
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    return -1;
  }

  FT_Face face;
  if (FT_New_Face(sFtLib, sFontFilePath.c_str(), 0, &face)) {
    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    return -1;
  }

  FT_Set_Pixel_Sizes(face, 0, 48);

  GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));  // disable byte-alignment restriction
  for (unsigned char c = 0; c < 128; c++) {
    // load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
      continue;
    }
    // generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 face->glyph->bitmap.width,
                 face->glyph->bitmap.rows,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // now store character for later use
    sCharacters[c] =
      Character {texture,
                 glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                 glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                 uint32_t(face->glyph->advance.x)};
  }
  GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));  // restore byte-alignment restriction

  FT_Done_Face(face);
  FT_Done_FreeType(sFtLib);

  return 0;
}
}  // namespace text
}  // namespace view
}  // namespace gal