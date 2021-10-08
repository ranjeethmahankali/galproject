#pragma once

#include <assert.h>
#include <chrono>
#include <functional>
#include <optional>
#include <ostream>

namespace gal {

class Timer
{
  using InternalClockT    = std::chrono::high_resolution_clock;
  using InternalDurationT = InternalClockT::duration;

  std::string                                    mName;
  std::function<void()>                          mOnFinish;
  std::chrono::high_resolution_clock::time_point mStart;

public:
  inline void start() { mStart = InternalClockT::now(); }

  template<typename DurationT>
  Timer(const std::string& name, DurationT* dptr)
      : mName(name)
  {
    mOnFinish = [dptr, this]() {
      if (dptr) {
        *dptr = std::chrono::duration_cast<DurationT>(InternalClockT::now() - mStart);
      }
    };
    assert(bool(mOnFinish));
    start();
  }

  Timer(const std::string& name, std::ostream& out);

  ~Timer();
};

}  // namespace gal
