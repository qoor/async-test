#include <sys/wait.h>

#include <cstdlib>
#include <cstring>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "boost/asio/thread_pool.hpp"
#include "test/server.h"
#include "test/test_timer.h"
#include "test/util.h"
#include "uv.h"

namespace {

constexpr const char kTestFilePath[] = "data/10G.dummy";

}  // namespace

namespace {

void SyncDelayedSumTest() {
  absl::PrintF("Starting sync delayed sum test...\n");

  TestTimer timer;  // test start

  DelayedSum(1);
  DelayedSum(90001);

  TestAnalyzer::GetInstance().FinishedTest(0, timer);  // test end
}

void AsyncDelayedSumTest() {
  std::vector<std::future<int>> task_list;

  absl::PrintF("Starting async delayed sum test...\n");

  TestTimer timer;  // test start

  task_list.emplace_back(std::async(std::launch::async, DelayedSum, 1));
  task_list.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

  for (auto& task : task_list) {
    task.get();
  }

  TestAnalyzer::GetInstance().FinishedTest(1, timer);  // test end
}

void SyncBigFileReadAndDelayedSumTest(std::string_view file_path) {
  absl::PrintF("Starting sync file read + delayed sum test...\n");

  TestTimer timer;  // test start

  if (!ReadEntireFile(file_path)) {
    absl::PrintF("Failed to open file: %s\n", file_path);
  } else {
    DelayedSum(1);
    ReadEntireFile(file_path);
    DelayedSum(90001);

    TestAnalyzer::GetInstance().FinishedTest(2, timer);  // test end
  }
}

void AsyncBigFileReadAndDelayedSumTest(std::string_view file_path) {
  std::vector<std::future<int>> tests;

  absl::PrintF("Starting async file read + delayed sum test...\n");

  TestTimer timer;  // test start

  tests.emplace_back(std::async(std::launch::async, ReadEntireFile, file_path));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 1));
  tests.emplace_back(std::async(std::launch::async, ReadEntireFile, file_path));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

  for (auto& test : tests) {
    test.get();
  }

  TestAnalyzer::GetInstance().FinishedTest(3, timer);  // test end
}

void SyncTcpPacketReadTest(int num_tcp_clients, std::string_view file_path) {
  boost::asio::io_context io;
  Server server(io, 7696);
  server.Listen();
  absl::PrintF("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> session_list =
      server.AcceptNumClients(num_tcp_clients);
  server.Close();

  absl::PrintF(
      "\nStarting sync TCP packet read + file read + delayed sum test...\n");

  TestTimer timer;  // test start

  ReadEntireFile(file_path);
  DelayedSum(1);

  for (std::unique_ptr<Session>& session : session_list) {
    ReadPacketFromSession(session.get());
  }

  ReadEntireFile(file_path);
  DelayedSum(90001);

  TestAnalyzer::GetInstance().FinishedTest(4, timer);  // test end
}

void AsyncTcpPacketReadTest(int num_tcp_clients, std::string_view file_path) {
  boost::asio::io_context io;
  Server server(io, 7696);
  server.Listen();
  absl::PrintF("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> session_list =
      server.AcceptNumClients(num_tcp_clients);
  server.Close();
  std::vector<std::future<int>> task_list;

  absl::PrintF(
      "\nStarting async TCP packet read + file read + delayed sum test...\n");

  TestTimer timer;  // test start

  task_list.emplace_back(
      std::async(std::launch::async, ReadEntireFile, file_path));
  task_list.emplace_back(std::async(std::launch::async, DelayedSum, 1));

  for (std::unique_ptr<Session>& session : session_list) {
    std::string data;
    task_list.emplace_back(
        std::async(std::launch::async, ReadPacketFromSession, session.get()));
  }

  task_list.emplace_back(
      std::async(std::launch::async, ReadEntireFile, file_path));
  task_list.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

  for (auto& task : task_list) {
    task.get();
  }

  TestAnalyzer::GetInstance().FinishedTest(5, timer);  // test end
}

void AsioTcpPacketReadTest(int num_tcp_clients, std::string_view file_path) {
  boost::asio::io_context io;
  Server server(io, 7696);
  server.Listen();
  absl::PrintF("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> session_list =
      server.AcceptNumClients(num_tcp_clients);
  server.Close();

  absl::PrintF(
      "\nStarting Boost.Asio single threaded TCP packet read + file read + "
      "delayed sum test...\n");

  TestTimer timer;  // test timer

  boost::asio::post(io, [&] { ReadEntireFile(file_path); });
  boost::asio::post(io, [] { DelayedSum(1); });

  for (std::unique_ptr<Session>& session : session_list) {
    std::string data;
    boost::asio::post(io, [&session] { ReadPacketFromSession(session.get()); });
  }

  boost::asio::post(io, [&] { ReadEntireFile(file_path); });
  boost::asio::post(io, [] { DelayedSum(90001); });

  io.run();

  TestAnalyzer::GetInstance().FinishedTest(6, timer);  // test end
}

void AsioMtTcpPacketReadTest(int num_tcp_clients, std::string_view file_path) {
  static constexpr const int kNumThreads = 8;

  boost::asio::io_context io;
  Server server(io, 7696);
  server.Listen();
  absl::PrintF("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> session_list =
      server.AcceptNumClients(num_tcp_clients);
  server.Close();

  absl::PrintF(
      "\nStarting Boost.Asio multi threaded TCP packet read + file read + "
      "delayed sum test...\n");

  boost::asio::thread_pool tp(kNumThreads);

  TestTimer timer;  // test start

  boost::asio::post(io, [&] { ReadEntireFile(file_path); });
  boost::asio::post(io, [] { DelayedSum(1); });

  for (std::unique_ptr<Session>& session : session_list) {
    std::string data;
    boost::asio::post(io, [&session] { ReadPacketFromSession(session.get()); });
  }

  boost::asio::post(io, [&] { ReadEntireFile(file_path); });
  boost::asio::post(io, [] { DelayedSum(90001); });

  for (int i = 0; i < kNumThreads; ++i) {
    boost::asio::post(tp, [&] { io.run(); });
  }

  tp.join();

  TestAnalyzer::GetInstance().FinishedTest(7, timer);  // test end
}

void LibuvTcpPacketReadTest(int num_tcp_clients, std::string_view file_path) {
  boost::asio::io_context io;
  Server server(io, 7696);
  server.Listen();
  absl::PrintF("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> session_list =
      server.AcceptNumClients(num_tcp_clients);
  server.Close();

  absl::PrintF(
      "\nStarting libuv multi threaded TCP packet read + file read + "
      "delayed sum test...\n");

  uv_loop_t* loop = uv_default_loop();

  TestTimer timer;  // test start

  auto data1 = std::make_unique<std::string_view>(file_path);
  std::unique_ptr<uv_work_t> work1 = MakeWork(data1);
  uv_queue_work(loop, work1.get(), LibuvReadEntireFile, nullptr);

  auto data2 = std::make_unique<int>(1);
  std::unique_ptr<uv_work_t> work2 = MakeWork(data1);
  uv_queue_work(loop, work2.get(), LibuvDelayedSum, nullptr);

  std::vector<std::unique_ptr<uv_work_t>> work3;
  for (std::unique_ptr<Session>& session : session_list) {
    std::unique_ptr<uv_work_t> work = MakeWork(session);
    std::unique_ptr<uv_work_t>& data = work3.emplace_back(std::move(work));
    uv_queue_work(loop, data.get(), LibuvReadPacketFromSession, nullptr);
  }

  auto data4 = std::make_unique<std::string_view>(file_path);
  std::unique_ptr<uv_work_t> work4 = MakeWork(data4);
  uv_queue_work(loop, work4.get(), LibuvReadEntireFile, nullptr);

  auto data5 = std::make_unique<int>(90001);
  std::unique_ptr<uv_work_t> work5 = MakeWork(data5);
  uv_queue_work(loop, work5.get(), LibuvDelayedSum, nullptr);

  uv_run(loop, UV_RUN_DEFAULT);

  TestAnalyzer::GetInstance().FinishedTest(8, timer);  // test end
}

}  // namespace

int main(int argc, char* argv[]) {
  TestTimer timer;

  int test_count = 1;
  if (argc >= 2) {
    test_count = atoi(argv[1]);
  }

  int num_tcp_clients = 1;
  if (argc >= 3) {
    num_tcp_clients = atoi(argv[2]);
  }

  std::string_view file_path(kTestFilePath, sizeof(kTestFilePath));

  TestAnalyzer::GetInstance().SetNumTestCases(9);
  absl::PrintF("Starting tests...\n\n");

  for (int i = 0; i < test_count; ++i) {
    // sync delayed sum test
    SyncDelayedSumTest();

    // async delayed sum test
    AsyncDelayedSumTest();

    // sync file read + delayed sum test
    SyncBigFileReadAndDelayedSumTest(file_path);

    // async file read + delayed sum test
    AsyncBigFileReadAndDelayedSumTest(file_path);

    // sync TCP packet read + file read + delayed sum test
    SyncTcpPacketReadTest(num_tcp_clients, file_path);

    // async TCP packet read
    AsyncTcpPacketReadTest(num_tcp_clients, file_path);

    // Boost.Asio single threaded async TCP packet read + file read +
    // delayed sum test
    AsioTcpPacketReadTest(num_tcp_clients, file_path);

    // Boost.Asio multi threaded async TCP packet read + file read +
    // delayed sum test
    AsioMtTcpPacketReadTest(num_tcp_clients, file_path);

    // libuv async TCP packet read + file read + delayed sum test
    LibuvTcpPacketReadTest(num_tcp_clients, file_path);
  }

  for (int i = 0; i < 10; ++i) {
    absl::PrintF("\n");
  }
  TestAnalyzer::GetInstance().PrintAll();

  absl::PrintF("Finished all tests.\n");
  return EXIT_SUCCESS;
}
