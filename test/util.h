#ifndef ASYNC_ASYNC_UTIL_H_
#define ASYNC_ASYNC_UTIL_H_

#include <cstdint>
#include <memory>
#include <string_view>

#include "test/server.h"
#include "uv.h"

uint64_t GetFileSize(std::string_view path);
int DelayedSum(int value);
int ReadEntireFile(std::string_view path);
int ReadPacketFromSession(Session* session);

void LibuvDelayedSum(uv_work_t* work) {
  DelayedSum(*reinterpret_cast<int*>(work->data));
}
void LibuvReadEntireFile(uv_work_t* work) {
  ReadEntireFile(*reinterpret_cast<std::string_view*>(work->data));
}
void LibuvReadPacketFromSession(uv_work_t* work) {
  ReadPacketFromSession(reinterpret_cast<Session*>(work->data));
}

template <typename T>
std::unique_ptr<uv_work_t> MakeWork(std::unique_ptr<T>& data) {
  auto work = std::make_unique<uv_work_t>();
  work->data = data.get();
  return work;
}

#endif  // ASYNC_ASYNC_UTIL_H_
