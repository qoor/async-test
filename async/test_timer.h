#ifndef ASYNC_TEST_TIMER_H_
#define ASYNC_TEST_TIMER_H_

#include <chrono>
#include <cstdint>

class TimerTest {
 public:
  TimerTest() : start_(std::chrono::steady_clock::now()) {}

  void Reset() { start_ = std::chrono::steady_clock::now(); }

  int64_t ElapsedInMilliseconds() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_)
        .count();
  }

 private:
  std::chrono::steady_clock::time_point start_;
};

#endif  // ASYNC_TEST_TIMER_H_
