#include <galview/Views.h>

namespace gal {
namespace view {

static std::vector<Views::RenderData>  sDrawables               = {};
static std::vector<Views::RenderData*> sSortedDrawables         = {};
static bool                            sSortedDrawablesUptoDate = false;

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
  sSortedDrawables.resize(sDrawables.size());
  std::iota(sSortedDrawables.begin(), sSortedDrawables.end(), sDrawables.data());
  std::sort(sSortedDrawables.begin(),
            sSortedDrawables.end(),
            [](const Views::RenderData* a, const Views::RenderData* b) {
              return drawOrderIndex(a->mDrawable) < drawOrderIndex(b->mDrawable);
            });
  sSortedDrawablesUptoDate = true;
}

Views::RenderData::RenderData(Views::VariantT drawable, const bool* visibility)
    : mDrawable(std::move(drawable))
    , mVisibility(visibility)
{}

void Views::render()
{
  if (!sSortedDrawablesUptoDate) {
    updateSortedDrawables();
  }
  for (const Views::RenderData* rdata : sSortedDrawables) {
    if (isVisible(*rdata)) {
      std::visit([](const auto& v) { v.draw(); }, rdata->mDrawable);
    }
  }
}

Box3 Views::visibleBounds()
{
  Box3 bounds;
  for (const auto& d : sDrawables) {
    if (isVisible(d)) {
      const Box3& b = std::visit([](const auto& v) { return v.bounds(); }, d.mDrawable);
      bounds.inflate(b);
    }
  }
  return bounds;
}

void Views::clear()
{
  sDrawables.clear();
}

size_t Views::addInternal(VariantT&& view, const bool* visibility)
{
  sDrawables.emplace_back(std::move(view), visibility);
  sSortedDrawablesUptoDate = false;
  return sDrawables.size() - 1;
}

Views::VariantT& Views::getDrawable(size_t i)
{
  return sDrawables[i].mDrawable;
}

}  // namespace view
}  // namespace gal
