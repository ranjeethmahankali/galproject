#include <galcore/Util.h>
#include <galview/GLUtil.h>
#include <galview/TextTags.h>

#include <array>
#include <iostream>
#include <vector>

namespace gal {
namespace view {

static constexpr size_t NUMCHARS = 128;

class CharAtlas
{
  static constexpr uint32_t                  NX        = 1 << 4;
  static constexpr uint32_t                  NY        = 1 << 3;
  uint32_t                                   mTileSize = 0;
  std::vector<uint8_t>                       mAtlas;
  std::array<std::array<float, 4>, NUMCHARS> mTexCoords;
  std::array<glm::ivec2, NUMCHARS>           mBearings;
  std::array<glm::ivec2, NUMCHARS>           mSizes;
  std::array<uint32_t, NUMCHARS>             mAdvances;
  uint32_t                                   mGLTextureId = 0;

  uint8_t* tilestart(uint8_t c)
  {
    uint32_t xi = (c % NX) * mTileSize;
    uint32_t yi = (c / NX) * mTileSize;

    return mAtlas.data() + (yi * width() + xi);
  }
  uint32_t width() const { return NX * mTileSize; }
  uint32_t height() const { return NY * mTileSize; }

  static void getFontTextureData(
    FT_Face                                              face,
    std::vector<uint8_t>&                                textureData,
    std::array<size_t, NUMCHARS>&                        offsets,
    std::array<std::pair<uint32_t, uint32_t>, NUMCHARS>& sizes,
    uint32_t&                                            tilesize)
  {
    for (unsigned char c = 0; c < 128; c++) {
      // load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        continue;
      }

      uint32_t width  = face->glyph->bitmap.width;
      uint32_t height = face->glyph->bitmap.rows;
      tilesize        = std::max(std::max(width, height), tilesize);
      sizes[c]        = std::make_pair(width, height);
      offsets[c]      = textureData.size();
      std::copy_n((uint8_t*)face->glyph->bitmap.buffer,
                  width * height,
                  std::back_inserter(textureData));
    }
  }

  void initGLTexture()
  {
    glGenTextures(1, &mGLTextureId);
    glBindTexture(GL_TEXTURE_2D, mGLTextureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 width(),
                 height(),
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 mAtlas.data());
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void deleteGLTexture()
  {
    if (mGLTextureId) {
      glDeleteTextures(1, &mGLTextureId);
    }
  }

  CharAtlas()
  {
    static const std::string sFontFilePath = utils::absPath("CascadiaMono.ttf").string();
    FT_Library               sFtLib;
    if (FT_Init_FreeType(&sFtLib)) {
      throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
    }

    FT_Face face;
    if (FT_New_Face(sFtLib, sFontFilePath.c_str(), 0, &face)) {
      throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
    }
    FT_Set_Pixel_Sizes(face, 0, 64);

    std::vector<uint8_t>                                textureData;
    std::array<size_t, NUMCHARS>                        offsets;
    std::array<std::pair<uint32_t, uint32_t>, NUMCHARS> sizes;
    getFontTextureData(face, textureData, offsets, sizes, mTileSize);
    mAtlas.resize(mTileSize * NX * mTileSize * NY, 0);

    const float    wf = float(width());
    const float    hf = float(height());
    const uint32_t w  = width();
    const uint32_t h  = height();
    for (uint8_t c = 0; c < NUMCHARS; c++) {
      size_t      offset    = offsets[c];
      uint8_t*    src       = textureData.data() + offset;
      const auto& dims      = sizes.at(c);
      uint8_t*    dst       = tilestart(c);
      size_t      dstoffset = std::distance(mAtlas.data(), dst);

      uint32_t x1 = dstoffset % w;
      uint32_t y1 = dstoffset / w;
      uint32_t x2 = x1 + dims.first;
      uint32_t y2 = y1 + dims.second;

      for (uint32_t r = 0; r < dims.second; r++) {
        std::copy_n(src, dims.first, dst);
        src += dims.first;
        dst += width();
      }

      mTexCoords[c] = {float(x1) / wf, float(y1) / hf, float(x2) / wf, float(y2) / hf};

      mSizes[c]    = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
      mBearings[c] = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
      mAdvances[c] = face->glyph->advance.x;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(sFtLib);

    initGLTexture();
  }

public:
  ~CharAtlas() { deleteGLTexture(); }

  const glm::ivec2&           bearing(uint8_t c) const { return mBearings[c]; }
  const glm::ivec2&           charsize(uint8_t c) const { return mSizes[c]; }
  uint32_t                    advance(uint8_t c) const { return mAdvances[c]; }
  const std::array<float, 4>& texcoords(uint8_t c) const { return mTexCoords[c]; }

  void bindTexture() const { GL_CALL(glBindTexture(GL_TEXTURE_2D, mGLTextureId)); }
  void unbindTexture() const { GL_CALL(glBindTexture(GL_TEXTURE_2D, 0)); }

  static const CharAtlas& get()
  {
    static CharAtlas sInstance = CharAtlas();
    return sInstance;
  }
};

void CharVertex::initAttributes()
{
  static constexpr size_t stride    = sizeof(CharVertex);
  static const void*      posOffset = (void*)(&(((CharVertex*)nullptr)->position));
  static const void*      nrmOffset = (void*)(&(((CharVertex*)nullptr)->offset));
  static const void*      texOffset = (void*)(&(((CharVertex*)nullptr)->texCoords));
  // Vertex position attribute.
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, posOffset));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, nrmOffset));
  GL_CALL(glEnableVertexAttribArray(1));
  GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, texOffset));
  GL_CALL(glEnableVertexAttribArray(2));
}

const glm::ivec2& charbearing(char c)
{
  return CharAtlas::get().bearing(uint8_t(c));
}
const glm::ivec2& charsize(char c)
{
  return CharAtlas::get().charsize(uint8_t(c));
}
uint32_t charadvance(char c)
{
  return CharAtlas::get().advance(uint8_t(c));
}

const std::array<float, 4>& chartexcoords(char c)
{
  return CharAtlas::get().texcoords(uint8_t(c));
}

TextTags::~TextTags()
{
  GL_CALL(glDeleteVertexArrays(1, &mVAO));
  GL_CALL(glDeleteBuffers(1, &mVBO));
};

void TextTags::draw() const
{
  static const size_t shaderId = Context::get().shaderId("text");
  Context::get().useShader(shaderId);
  Context::get().setUniform("textColor", glm::vec3 {1.f, 1.f, 1.f});

  CharAtlas::get().bindTexture();
  GL_CALL(glBindVertexArray(mVAO));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVSize));
  CharAtlas::get().unbindTexture();
};

}  // namespace view
}  // namespace gal
