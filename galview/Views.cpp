#include <Views.h>

namespace gal {
namespace view {

static std::vector<Views::RenderData>& drawables()
{
  static std::vector<Views::RenderData> sDrawables = {};
  return sDrawables;
}

static std::vector<Views::RenderData*> sortedDrawables()
{
  static std::vector<Views::RenderData*> sSortedDrawables = {};
  return sSortedDrawables;
}

static bool& sortedDrawablesUptoDate()
{
  static bool sSortedDrawablesUptoDate = false;
  return sSortedDrawablesUptoDate;
}

static bool isVisible(const Views::RenderData& rdata)
{
  const bool* ptr = rdata.mVisibility;
  return ptr == nullptr ? false : *ptr;
}

static uint64_t drawOrderIndex(const Views::VariantT& view)
{
  return std::visit([](const auto& v) { return v.drawOrderIndex(); }, view);
}

static void updateSortedDrawables()
{
  sortedDrawables().resize(drawables().size());
  std::iota(sortedDrawables().begin(), sortedDrawables().end(), drawables().data());
  std::sort(sortedDrawables().begin(),
            sortedDrawables().end(),
            [](const Views::RenderData* a, const Views::RenderData* b) {
              return drawOrderIndex(a->mDrawable) < drawOrderIndex(b->mDrawable);
            });
  sortedDrawablesUptoDate() = true;
}

Views::RenderData::RenderData(Views::VariantT drawable, const bool* visibility)
    : mDrawable(std::move(drawable))
    , mVisibility(visibility)
{}

void Views::render()
{
  if (!sortedDrawablesUptoDate()) {
    updateSortedDrawables();
  }
  for (const Views::RenderData* rdata : sortedDrawables()) {
    if (isVisible(*rdata)) {
      std::visit([](const auto& v) { v.draw(); }, rdata->mDrawable);
    }
  }
}

Box3 Views::visibleBounds()
{
  Box3 bounds;
  for (const auto& d : drawables()) {
    if (isVisible(d)) {
      const Box3& b = std::visit([](const auto& v) { return v.bounds(); }, d.mDrawable);
      bounds.inflate(b);
    }
  }
  return bounds;
}

void Views::clear()
{
  drawables().clear();
}

size_t Views::addInternal(VariantT&& view, const bool* visibility)
{
  drawables().emplace_back(std::move(view), visibility);
  sortedDrawablesUptoDate() = false;
  return drawables().size() - 1;
}

Views::VariantT& Views::getDrawable(size_t i)
{
  return drawables()[i].mDrawable;
}

}  // namespace view
}  // namespace gal
