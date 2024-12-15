#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <future>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "async/test_timer.h"
#include "boost/asio.hpp"

namespace {

constexpr const char kTestFilePath[] = "data/1G.dummy";

int DelayedSum(int value) {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  return value + 9999;
}

int ReadBigFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return -1;
  }

  printf("Reading file %s...\n", path.c_str());

  std::string line;
  std::string::size_type size = 0;
  while (std::getline(file, line)) {
    // nothing
    size += line.size();
  }

  printf("Done.\n");
  return size;
}

void AcceptHandler(boost::system::error_code error) {
  if (error) {
    printf("Failed to accept the client\n");
  }
}

void ConnectHandler(boost::system::error_code error) {
  if (error) {
    printf("Failed to connect to the server\n");
  }
}

std::pair<boost::asio::ip::tcp::socket, boost::asio::ip::tcp::socket>
GetEchoServerAndClient(boost::asio::io_context& io) {
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 7696);
  boost::asio::ip::tcp::acceptor acceptor(io, endpoint);
  boost::asio::ip::tcp::socket accepted_client(io);

  acceptor.listen();

  boost::asio::ip::tcp::socket client(io);
  boost::asio::ip::tcp::resolver resolver(io);

  client.async_connect(resolver.resolve("127.0.0.1", "7696")->endpoint(),
                       ConnectHandler);
  acceptor.async_accept(accepted_client, AcceptHandler);

  return std::make_pair(std::move(accepted_client), std::move(client));
}

}  // namespace

int main() {
  TimerTest timer;

  boost::asio::io_context io;
  GetEchoServerAndClient(io);
  io.run();

  printf("Starting tests...\n\n");

  // sync delayed sum test
  {
    printf("Starting sync delayed sum test...\n");
    timer.Reset();  // test start

    printf("result = %d\n", DelayedSum(1));
    printf("result = %d\n", DelayedSum(90001));

    printf("%lldms\n\n", timer.ElapsedInMilliseconds());  // test end
  }

  // async delayed sum test
  {
    std::vector<std::future<int>> tests;

    printf("Starting async delayed sum test...\n");

    timer.Reset();  // test start

    tests.emplace_back(std::async(std::launch::async, DelayedSum, 1));
    tests.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

    for (auto& test : tests) {
      printf("result = %d\n", test.get());
    }

    printf("%lldms\n\n", timer.ElapsedInMilliseconds());  // test end
  }

  // sync file read + delayed sum test
  {
    std::string path = kTestFilePath;

    printf("Starting sync file read + delayed sum test...\n");

    timer.Reset();

    if (!ReadBigFile(path)) {
      printf("Failed to open file: %s\n", path.c_str());
    } else {
      DelayedSum(1);
      ReadBigFile(path);
      DelayedSum(90001);

      printf("%lldms\n\n", timer.ElapsedInMilliseconds());
    }
  }

  // async file read + delayed sum test
  {
    std::string path = kTestFilePath;
    std::vector<std::future<int>> tests;

    printf("Starting async file read + delayed sum test...\n");

    timer.Reset();

    tests.emplace_back(std::async(std::launch::async, ReadBigFile, path));
    tests.emplace_back(std::async(std::launch::async, DelayedSum, 1));
    tests.emplace_back(std::async(std::launch::async, ReadBigFile, path));
    tests.emplace_back(std::async(std::launch::async, DelayedSum, 90001));

    for (auto& test : tests) {
      test.get();
    }

    printf("%lldms\n\n", timer.ElapsedInMilliseconds());
  }

  printf("Finished all tests.\n");
  return EXIT_SUCCESS;
}
