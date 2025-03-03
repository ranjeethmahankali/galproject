#include <array>
#include <iostream>
#include <numeric>
#include <vector>

#include <png++/png.hpp>

#include <AnnotationsView.h>
#include <GLUtil.h>
#include <Util.h>

namespace gal {
namespace view {

template<size_t NX, size_t NY, typename TPixel>
struct TextureAtlas
{
  static_assert(std::is_same_v<TPixel, uint8_t> || std::is_same_v<TPixel, uint32_t>,
                "Unsupported pixel type");

  static constexpr TPixel        ZERO         = TPixel(0);
  uint32_t                       mTileSize    = 0;
  uint32_t                       mGLTextureId = 0;
  std::vector<TPixel>            mTexture {};
  std::array<glm::vec4, NX * NY> mTexCoords {};

  TextureAtlas() = default;
  explicit TextureAtlas(uint32_t tilesize)
      : mTileSize(tilesize)
  {
    allocate();
  }

  ~TextureAtlas() { deleteGLTexture(); }

  TextureAtlas(TextureAtlas const&)            = delete;
  TextureAtlas(TextureAtlas&&)                 = delete;
  TextureAtlas& operator=(TextureAtlas const&) = delete;
  TextureAtlas& operator=(TextureAtlas&&)      = delete;

  void allocate()
  {
    mTexture.clear();
    mTexture.resize(size_t(mTileSize) * mTileSize * NX * NY, ZERO);
  }

  void bind() const { GL_CALL(glBindTexture(GL_TEXTURE_2D, mGLTextureId)); }

  TPixel* tilestart(size_t c)
  {
    uint32_t xi = (c % NX) * mTileSize;
    uint32_t yi = (c / NX) * mTileSize;

    return mTexture.data() + (yi * width() + xi);
  }
  uint32_t width() const { return NX * mTileSize; }
  uint32_t height() const { return NY * mTileSize; }

  void initGLTexture()
  {
    GLint pixformat = 0;
    if constexpr (std::is_same_v<TPixel, uint8_t>) {
      pixformat = GL_RED;
    }
    else if constexpr (std::is_same_v<TPixel, uint32_t>) {
      pixformat = GL_RGBA;
    }
    GL_CALL(glGenTextures(1, &mGLTextureId));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, mGLTextureId));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D,
                         0,
                         pixformat,
                         width(),
                         height(),
                         0,
                         pixformat,
                         GL_UNSIGNED_BYTE,
                         mTexture.data()));
    // set texture options
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
  }

  void deleteGLTexture()
  {
    if (mGLTextureId) {
      glDeleteTextures(1, &mGLTextureId);
      mGLTextureId = 0;
    }
  }
};

template<size_t NUMCHARS>
void getFontTextureData(FT_Face                           face,
                        std::vector<uint8_t>&             textureData,
                        std::array<size_t, NUMCHARS>&     offsets,
                        std::array<glm::ivec2, NUMCHARS>& sizes,
                        std::array<glm::ivec2, NUMCHARS>& bearings,
                        std::array<uint32_t, NUMCHARS>&   advances,
                        uint32_t&                         tilesize)
{
  for (unsigned char c = 0; c < 128; c++) {
    // load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      glutil::logger().error("ERROR::FREETYTPE: Failed to load glyph for character {}",
                             c);
      continue;
    }

    uint32_t width  = face->glyph->bitmap.width;
    uint32_t height = face->glyph->bitmap.rows;
    tilesize        = std::max(std::max(width, height), tilesize);
    sizes[c]        = {width, height};
    bearings[c]     = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
    offsets[c]      = textureData.size();
    advances[c]     = face->glyph->advance.x;
    std::copy_n((uint8_t*)face->glyph->bitmap.buffer,
                width * height,
                std::back_inserter(textureData));
  }
}

class CharAtlas
{
  static constexpr size_t       NX       = 1 << 4;
  static constexpr size_t       NY       = 1 << 3;
  static constexpr size_t       NUMCHARS = size_t(NX * NY);
  TextureAtlas<NX, NY, uint8_t> mAtlas;

  std::array<glm::ivec2, NUMCHARS> mBearings {};
  std::array<glm::ivec2, NUMCHARS> mSizes {};
  std::array<uint32_t, NUMCHARS>   mAdvances {};

  CharAtlas()
  {
    static const std::string sFontFilePath = utils::absPath("CascadiaMono.ttf").string();
    FT_Library               sFtLib        = nullptr;
    if (FT_Init_FreeType(&sFtLib)) {
      throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
    }

    FT_Face face = nullptr;
    if (FT_New_Face(sFtLib, sFontFilePath.c_str(), 0, &face)) {
      throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    std::vector<uint8_t>         textureData;
    std::array<size_t, NUMCHARS> offsets {};
    getFontTextureData(
      face, textureData, offsets, mSizes, mBearings, mAdvances, mAtlas.mTileSize);
    mAtlas.allocate();

    const float    wf = float(mAtlas.width());
    const float    hf = float(mAtlas.height());
    const uint32_t w  = mAtlas.width();
    for (uint8_t c = 0; c < NUMCHARS; c++) {
      size_t      offset    = offsets[c];
      uint8_t*    src       = textureData.data() + offset;
      const auto& dims      = mSizes.at(c);
      uint8_t*    dst       = mAtlas.tilestart(c);
      size_t      dstoffset = std::distance(mAtlas.mTexture.data(), dst);

      uint32_t x1 = dstoffset % w;
      uint32_t y1 = dstoffset / w;
      uint32_t x2 = x1 + dims.x;
      uint32_t y2 = y1 + dims.y;

      for (uint32_t r = 0; r < dims.y; r++) {
        std::copy_n(src, dims.x, dst);
        src += dims.x;
        dst += w;
      }

      mAtlas.mTexCoords[c] = {
        float(x1) / wf, float(y1) / hf, float(x2) / wf, float(y2) / hf};
    }

    FT_Done_Face(face);
    FT_Done_FreeType(sFtLib);

    mAtlas.initGLTexture();
  }

public:
  const glm::ivec2&                    bearing(uint8_t c) const { return mBearings[c]; }
  const glm::ivec2&                    charsize(uint8_t c) const { return mSizes[c]; }
  uint32_t                             advance(uint8_t c) const { return mAdvances[c]; }
  const TextureAtlas<NX, NY, uint8_t>& atlas() const { return mAtlas; }

  static const CharAtlas& get()
  {
    static CharAtlas sInstance = CharAtlas();
    return sInstance;
  }
};

class GlyphAtlas
{
private:
  static constexpr uint32_t GLYPHSIZE = 128;
  static constexpr size_t   NX        = 1 << 4;
  static constexpr size_t   NY        = 1 << 3;
  static constexpr size_t   NUMGLYPHS = size_t(NX * NY);

  TextureAtlas<NX, NY, uint32_t>           mAtlas;
  std::vector<png::image<png::rgba_pixel>> mImages;

  GlyphAtlas()
      : mAtlas(GLYPHSIZE)
  {}

  // cppcheck-suppress unusedPrivateFunction
  /**
   * @brief This is for debugging purposes only.
   * @param path
   */
  void exportAtlasAsImage(const fs::path path)
  {
    uint32_t                    w = mAtlas.width();
    uint32_t                    h = mAtlas.height();
    png::image<png::rgba_pixel> image(w, h);
    for (uint32_t i = 0; i < mAtlas.mTexture.size(); i++) {
      uint32_t x        = i % w;
      uint32_t y        = i / w;
      image[y][x].red   = mAtlas.mTexture[i] & 0x000000ff;
      image[y][x].green = (mAtlas.mTexture[i] & 0x0000ff00) >> 8;
      image[y][x].blue  = (mAtlas.mTexture[i] & 0x00ff0000) >> 16;
      image[y][x].alpha = (mAtlas.mTexture[i] & 0xff000000) >> 24;
    }
    image.write(path.string());
  }

public:
  const TextureAtlas<NX, NY, uint32_t>& atlas() const { return mAtlas; }

  glm::ivec2 glyphSize(size_t i)
  {
    if (i >= mImages.size()) {
      std::cerr << "Not a valid glyph index\n";
      throw std::out_of_range("Not a valid glyph index");
    }
    const auto& image = mImages[i];
    return {int(image.get_width()), int(image.get_height())};
  }

  std::vector<int32_t> loadGlyphs(const std::vector<fs::path>& labeledPNGPaths)
  {
    // Clear the current texture atlas.
    mAtlas.deleteGLTexture();
    if (mImages.size() + labeledPNGPaths.size() >= NUMGLYPHS) {
      std::cerr << "Cannot load glyphs because only a maximum of " << NUMGLYPHS
                << " glyphs can be loaded.";
      throw std::out_of_range("Too many glyphs!");
    }

    int32_t ibegin = int32_t(mImages.size());
    mImages.reserve(mImages.size() + labeledPNGPaths.size());
    for (const auto& path : labeledPNGPaths) {
      if (!fs::exists(path)) {
        std::cerr << "Cannot find glyph file: " << path << std::endl;
        continue;
      }
      png::image<png::rgba_pixel> image(path.string());
      if (image.get_width() > GLYPHSIZE || image.get_height() > GLYPHSIZE) {
        std::cerr << "Cannot load glyph because the image is too large! Image size must "
                     "be smaller than "
                  << GLYPHSIZE << "x" << GLYPHSIZE << std::endl;
        continue;
      }
      mImages.emplace_back(std::move(image));
    }
    int32_t iend = int32_t(mImages.size());

    mAtlas.allocate();  // Clears the entire texture.
    uint32_t    width = mAtlas.width();
    const float wf    = float(mAtlas.width());
    const float hf    = float(mAtlas.height());
    for (size_t i = 0; i < mImages.size(); i++) {
      const auto& image     = mImages[i];
      uint32_t*   dst       = mAtlas.tilestart(i);
      size_t      dstoffset = std::distance(mAtlas.mTexture.data(), dst);
      uint32_t    x1        = dstoffset % width;
      uint32_t    y1        = dstoffset / width;
      uint32_t    x2        = x1 + image.get_width();
      uint32_t    y2        = y1 + image.get_height();
      for (uint32_t y = 0; y < image.get_height(); y++) {
        for (uint32_t x = 0; x < image.get_width(); x++) {
          const auto& pix = image[y][x];
          *(dst++) = pix.red | (pix.green << 8) | (pix.blue << 16) | (pix.alpha << 24);
        }
        dst += width - image.get_width();
      }

      mAtlas.mTexCoords[i] = {
        float(x1) / wf, float(y1) / hf, float(x2) / wf, float(y2) / hf};
    }
    mAtlas.initGLTexture();

    // Indices.
    std::vector<int32_t> indices(size_t(iend - ibegin));
    std::iota(indices.begin(), indices.end(), ibegin);
    return indices;
  }

  static GlyphAtlas& get()
  {
    static GlyphAtlas sInstance = GlyphAtlas();
    return sInstance;
  }
};

void bindCharAtlasTexture()
{
  CharAtlas::get().atlas().bind();
}

void bindGlyphAtlasTexture()
{
  GlyphAtlas::get().atlas().bind();
}

std::vector<int32_t> loadGlyphs(const std::vector<fs::path>& labeledPNGPaths)
{
  return GlyphAtlas::get().loadGlyphs(labeledPNGPaths);
}

void unbindTexture()
{
  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void AnnotationVertex::initAttributes()
{
  static constexpr size_t stride    = sizeof(AnnotationVertex);
  static const void*      posOffset = (void*)(&(((AnnotationVertex*)nullptr)->position));
  static const void*      nrmOffset = (void*)(&(((AnnotationVertex*)nullptr)->offset));
  static const void*      texOffset = (void*)(&(((AnnotationVertex*)nullptr)->texCoords));
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

const glm::vec4& chartexcoords(char c)
{
  return CharAtlas::get().atlas().mTexCoords.at(uint8_t(c));
}

const glm::vec4& glyphtexcoords(size_t i)
{
  return GlyphAtlas::get().atlas().mTexCoords.at(i);
}

glm::ivec2 glyphSize(size_t i)
{
  return GlyphAtlas::get().glyphSize(i);
}

}  // namespace view
}  // namespace gal
