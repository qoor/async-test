#ifndef ASYNC_ASYNC_UTIL_H_
#define ASYNC_ASYNC_UTIL_H_

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "absl/strings/str_format.h"
#include "test/server.h"
#include "uv.h"

template <typename T, typename U>
class TestWork {
 public:
  explicit TestWork(T input) : input_(input) { req_.data = this; }

  void SetOutput(U output) { output_ = output; }

  T input() const { return input_; }

  uv_work_t* req() { return &req_; }

 private:
  T input_;
  U output_;
  uv_work_t req_;
};

template <typename T, typename U>
class TestWork<T*, U> {
 public:
  explicit TestWork(T* input) : input_(input) { req_.data = this; }

  void SetOutput(U output) { output_ = output; }

  T* input() const { return input_; }

  uv_work_t* req() { return &req_; }

 private:
  T* input_;
  U output_;
  uv_work_t req_;
};

uint64_t GetFileSize(std::string_view path);
int DelayedSum(int value);
int ReadEntireFile(std::string_view path);
int ReadPacketFromSession(Session* session);

void LibuvDelayedSum(uv_work_t* work) {
  TestWork<int, int>& data = *static_cast<TestWork<int, int>*>(work->data);
  data.SetOutput(DelayedSum(data.input()));
}
void LibuvReadEntireFile(uv_work_t* work) {
  TestWork<std::string_view, int>& data =
      *static_cast<TestWork<std::string_view, int>*>(work->data);
  data.SetOutput(ReadEntireFile(data.input()));
}
void LibuvReadPacketFromSession(uv_work_t* work) {
  TestWork<Session*, int>& data =
      *static_cast<TestWork<Session*, int>*>(work->data);
  std::string str;
  data.SetOutput(ReadPacketFromSession(data.input()));
}

#endif  // ASYNC_ASYNC_UTIL_H_
