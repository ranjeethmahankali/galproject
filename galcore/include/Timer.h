#pragma once

#include <assert.h>
#include <chrono>
#include <functional>
#include <optional>
#include <ostream>

namespace gal {

/**
 * @brief RAII timer for measuring execution times.
 *
 */
class Timer
{
  using InternalClockT    = std::chrono::high_resolution_clock;
  using InternalDurationT = InternalClockT::duration;

  std::string                                    mName;
  std::function<void()>                          mOnFinish;
  std::chrono::high_resolution_clock::time_point mStart;

public:
  inline void start() { mStart = InternalClockT::now(); }

  /**
   * @brief Create a new timer instance that assigns the time elapsed to the given
   * duration pointer.
   *
   * @tparam DurationT duration type.
   * @param name Name of the timer.
   * @param dptr Pointer to the duration. Elapsed time will be written to this address.
   */
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

  Timer(Timer const&)            = delete;
  Timer(Timer&&)                 = delete;
  Timer& operator=(Timer const&) = delete;
  Timer& operator=(Timer&&)      = delete;

  /**
   * @brief Create a new timer that prints the elapsed time to the given output stream.
   *
   * @param name Name of the timer.
   * @param out Output stream.
   */
  Timer(const std::string& name, std::ostream& out);

  ~Timer();
};

}  // namespace gal
