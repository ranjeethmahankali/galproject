#include <galview/Views.h>
#include <variant>

namespace gal {
namespace view {

std::vector<Views::RenderData> Views::mDrawables = {};

void Views::remove(size_t id)
{
  mDrawables.erase(
    std::remove_if(mDrawables.begin(),
                   mDrawables.end(),
                   [id](const RenderData& rdata) { return std::get<2>(rdata) == id; }),
    mDrawables.end());
}

static bool isVisible(const Views::RenderData& rdata)
{
  const bool* ptr = std::get<1>(rdata);
  return ptr == nullptr ? false : *ptr;
}

void Views::render()
{
  for (const Views::RenderData& rdata : mDrawables) {
    if (isVisible(rdata)) {
      std::visit([](const auto& v) { v.draw(); }, std::get<0>(rdata));
    }
  }
}

Box3 Views::visibleBounds()
{
  Box3 bounds;
  for (const auto& d : mDrawables) {
    if (isVisible(d)) {
      const Box3& b =
        std::visit([](const auto& v) { return v.bounds(); }, std::get<0>(d));
      bounds.inflate(b.min);
      bounds.inflate(b.max);
    }
  }
  return bounds;
}

void Views::clear()
{
  mDrawables.clear();
}

static size_t newViewId()
{
  static size_t sId = 1;
  return sId++;
}

size_t Views::addInternal(VariantT&& view, const bool* visibility)
{
  mDrawables.emplace_back(std::move(view), visibility, newViewId());
  return std::get<2>(mDrawables.back());
}

}  // namespace view
}  // namespace gal
