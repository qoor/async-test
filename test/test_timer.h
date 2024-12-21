#ifndef TEST_TEST_TIMER_H_
#define TEST_TEST_TIMER_H_

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "absl/strings/str_format.h"

class TestTimer {
 public:
  TestTimer() : start_(std::chrono::steady_clock::now()) {}

  void Reset() { start_ = std::chrono::steady_clock::now(); }

  int64_t ElapsedInMilliseconds() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_)
        .count();
  }

 private:
  std::chrono::steady_clock::time_point start_;
};

class TestAnalyzer {
 public:
  static TestAnalyzer& GetInstance() {
    static TestAnalyzer* instance = nullptr;
    if (!instance) {
      instance = new TestAnalyzer();
    }
    return *instance;
  }

  void FinishedTest(int test_id, TestTimer timer) {
    int64_t elapsed = timer.ElapsedInMilliseconds();

    test_times_[test_id].emplace_back(elapsed);
    absl::PrintF("%dms\n\n", elapsed);
  }

  void PrintAll() {
    printf("=== total %ld tests results ===\n", test_times_.size());
    for (uint32_t i = 0; i < test_times_.size(); ++i) {
      Print(i);
    }
    printf("===============================\n");
  }

  void Print(int test_id) {
    int64_t total = 0;
    for (int64_t time : test_times_[test_id]) {
      total += time;
    }
    total /= test_times_[test_id].size();
    absl::PrintF("test %d avg elapsed = %dms\n", test_id, total);

    if (total == 0) {
      return;
    }

    for (uint64_t i = 0; i < test_times_[test_id].size(); ++i) {
      absl::PrintF(" - %d cycle = %dms\n", i + 1, test_times_[test_id][i]);
    }
  }

  void SetNumTestCases(int num_test_cases) {
    test_times_.clear();
    test_times_.resize(num_test_cases);
  }

 private:
  TestAnalyzer() = default;

  std::vector<std::vector<int64_t>> test_times_;
};

#endif  // TEST_TEST_TIMER_H_
