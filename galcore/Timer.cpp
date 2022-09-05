#include <Timer.h>

namespace gal {

Timer::Timer(const std::string& name, std::ostream& out)
    : mName(name)
{
  mOnFinish = [&out, this]() {
    out << "Timer '" << mName << "' stopped after "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(InternalClockT::now() -
                                                                mStart)
             .count()
        << "ns\n";
  };
  assert(bool(mOnFinish));
  start();
}

Timer::~Timer()
{
  if (mOnFinish) {
    mOnFinish();
  }
}

}  // namespace gal
