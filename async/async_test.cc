#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "async/server.h"
#include "async/test_timer.h"
#include "boost/asio/thread_pool.hpp"

namespace {

constexpr const char kTestFilePath[] = "data/1G.dummy";

int DelayedSum(int value) {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  return value + 9999;
}

int ReadBigFile(const std::string* path) {
  std::ifstream file(*path);
  if (!file.is_open()) {
    return -1;
  }

  printf("Reading file %s...\n", path->c_str());

  std::string line;
  std::string::size_type size = 0;
  while (std::getline(file, line)) {
    // nothing
    size += line.size();
  }

  printf("Done.\n");
  return size;
}

int ReadPacketFromSession(Session* session) {
  std::string data;
  printf("Read %lu bytes\n", session->Read(data));
  return 0;
}

}  // namespace

namespace {

void SyncDelayedSumTest() {
  printf("Starting sync delayed sum test...\n");

  TestTimer timer;  // test start

  printf("result = %d\n", DelayedSum(1));
  printf("result = %d\n", DelayedSum(90001));

  TestAnalyzer::GetInstance().FinishedTest(0, timer);  // test end
}

void AsyncDelayedSumTest() {
  std::vector<std::future<int>> tests;

  printf("Starting async delayed sum test...\n");

  TestTimer timer;  // test start

  tests.emplace_back(std::async(std::launch::async, DelayedSum, 1));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

  for (auto& test : tests) {
    printf("result = %d\n", test.get());
  }

  TestAnalyzer::GetInstance().FinishedTest(1, timer);  // test end
}

void SyncBigFileReadAndDelayedSumTest(const std::string& file_path) {
  printf("Starting sync file read + delayed sum test...\n");

  TestTimer timer;  // test start

  if (!ReadBigFile(&file_path)) {
    printf("Failed to open file: %s\n", file_path.c_str());
  } else {
    DelayedSum(1);
    ReadBigFile(&file_path);
    DelayedSum(90001);

    TestAnalyzer::GetInstance().FinishedTest(2, timer);  // test end
  }
}

void AsyncBigFileReadAndDelayedSumTest(const std::string& file_path) {
  std::vector<std::future<int>> tests;

  printf("Starting async file read + delayed sum test...\n");

  TestTimer timer;  // test start

  tests.emplace_back(std::async(std::launch::async, ReadBigFile, &file_path));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 1));
  tests.emplace_back(std::async(std::launch::async, ReadBigFile, &file_path));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

  for (auto& test : tests) {
    test.get();
  }

  TestAnalyzer::GetInstance().FinishedTest(3, timer);  // test end
}

void SyncTcpPacketReadTest(Server& server,
                           int num_tcp_clients,
                           const std::string& file_path) {
  std::string data;

  server.Listen();
  printf("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> sessions =
      server.AcceptNumClients(num_tcp_clients);
  server.Close();

  printf("\nStarting sync TCP packet read + file read + delayed sum test...\n");

  TestTimer timer;  // test start

  ReadBigFile(&file_path);
  DelayedSum(1);

  for (std::unique_ptr<Session>& session : sessions) {
    ReadPacketFromSession(session.get());
  }

  ReadBigFile(&file_path);
  DelayedSum(90001);

  TestAnalyzer::GetInstance().FinishedTest(4, timer);  // test end
}

void AsyncTcpPacketReadTest(Server& server,
                            int num_tcp_clients,
                            const std::string& file_path) {
  server.Listen();
  printf("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> sessions =
      server.AcceptNumClients(num_tcp_clients);
  LaunchClient(num_tcp_clients);
  server.Close();
  std::vector<std::future<int>> tests;

  printf(
      "\nStarting async TCP packet read + file read + delayed sum test...\n");

  TestTimer timer;  // test start

  tests.emplace_back(std::async(std::launch::async, ReadBigFile, &file_path));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 1));

  for (std::unique_ptr<Session>& session : sessions) {
    std::string data;
    tests.emplace_back(
        std::async(std::launch::async, ReadPacketFromSession, session.get()));
  }

  tests.emplace_back(std::async(std::launch::async, ReadBigFile, &file_path));
  tests.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

  for (auto& reader : tests) {
    reader.get();
  }

  TestAnalyzer::GetInstance().FinishedTest(5, timer);  // test end
}

void AsioTcpPacketReadTest(Server& server,
                           int num_tcp_clients,
                           const std::string& file_path,
                           boost::asio::io_context& io) {
  server.Listen();
  printf("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> sessions =
      server.AcceptNumClients(num_tcp_clients);
  LaunchClient(num_tcp_clients);
  server.Close();
  std::vector<std::future<int>> readers;

  printf(
      "\nStarting Boost.Asio single threaded TCP packet read + file read + "
      "delayed sum test...\n");

  TestTimer timer;  // test timer

  boost::asio::post(io, [&file_path] { ReadBigFile(&file_path); });
  boost::asio::post(io, [] { DelayedSum(1); });

  for (std::unique_ptr<Session>& session : sessions) {
    std::string data;
    boost::asio::post(io, [&session] { ReadPacketFromSession(session.get()); });
  }

  boost::asio::post(io, [&file_path] { ReadBigFile(&file_path); });
  boost::asio::post(io, [] { DelayedSum(90001); });

  io.run();

  TestAnalyzer::GetInstance().FinishedTest(6, timer);  // test end
}

void AsioMtTcpPacketReadTest(Server& server,
                             int num_tcp_clients,
                             const std::string& file_path,
                             boost::asio::io_context& io) {
  server.Listen();
  printf("Waiting for new client...\n");

  std::vector<std::unique_ptr<Session>> sessions =
      server.AcceptNumClients(num_tcp_clients);
  LaunchClient(num_tcp_clients);
  server.Close();
  std::vector<std::future<int>> readers;

  printf(
      "\nStarting Boost.Asio multi threaded TCP packet read + file read + "
      "delayed sum test...\n");

  boost::asio::thread_pool tp;

  TestTimer timer;

  boost::asio::post(tp, [&file_path] { ReadBigFile(&file_path); });
  boost::asio::post(tp, [] { DelayedSum(1); });

  for (std::unique_ptr<Session>& session : sessions) {
    std::string data;
    boost::asio::post(tp, [&session] { ReadPacketFromSession(session.get()); });
  }

  boost::asio::post(tp, [&file_path] { ReadBigFile(&file_path); });
  boost::asio::post(tp, [] { DelayedSum(1); });

  tp.join();

  TestAnalyzer::GetInstance().FinishedTest(7, timer);  // test end
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

  std::string file_path = kTestFilePath;

  printf("Starting tests...\n\n");

  for (int i = 0; i < test_count; ++i) {
    TestAnalyzer::GetInstance().SetNumTestCases(8);

    // // sync delayed sum test
    // SyncDelayedSumTest();
    //
    // // async delayed sum test
    // AsyncDelayedSumTest();
    //
    // // sync file read + delayed sum test
    // SyncBigFileReadAndDelayedSumTest(file_path);
    //
    // // async file read + delayed sum test
    // AsyncBigFileReadAndDelayedSumTest(file_path);

    boost::asio::io_context io;
    Server server(io, 7696);

    // // sync TCP packet read + file read + delayed sum test
    // SyncTcpPacketReadTest(server, num_tcp_clients, file_path);
    //
    // // async TCP packet read
    // AsyncTcpPacketReadTest(server, num_tcp_clients, file_path);
    //
    // // Boost.Asio single threaded async TCP packet read + file read +
    // // delayed sum test
    // AsioTcpPacketReadTest(server, num_tcp_clients, file_path, io);
    //
    // Boost.Asio multi threaded async TCP packet read + file read +
    // delayed sum test
    AsioMtTcpPacketReadTest(server, num_tcp_clients, file_path, io);

    for (int i = 0; i < 10; ++i) {
      printf("\n");
    }

    TestAnalyzer::GetInstance().PrintAll();
  }

  printf("Finished all tests.\n");
  return EXIT_SUCCESS;
}
