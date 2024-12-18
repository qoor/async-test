#include "client.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

#include "boost/asio/io_context.hpp"

namespace {

void RunClient(boost::asio::io_context* io, int id, const char* server_port) {
  printf("Waiting for connecting the server... (%d)\n", id);

  Client client(*io);

  if (!client.Connect("127.0.0.1", server_port)) {
    return;
  }

  std::ifstream file("data/1G.dummy");
  if (!file.is_open()) {
    printf("Failed to open dummy file\n");
    return;
  }

  constexpr const int kBufferSize = 65535;
  auto buffer = std::unique_ptr<char[]>(new char[kBufferSize]);
  while (!file.eof()) {
    file.read(buffer.get(), kBufferSize);
    std::streamsize bytes = file.gcount();
    if (client.Write(std::string_view(buffer.get(), bytes)) == 0 &&
        !file.eof()) {
      printf("Server lost the connection\n");
      break;
    }
  }

  printf("Success to send dummy file to the server (%d)\n", id);
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s [SERVER PORT] [CLIENT_COUNT]\n", argv[0]);
    return EXIT_FAILURE;
  }

  int num_clients = 1;

  if (argc >= 3) {
    num_clients = atoi(argv[2]);
  }

  std::vector<std::thread> clients;
  boost::asio::io_context io;
  for (int i = 0; i < num_clients; ++i) {
    clients.emplace_back(RunClient, &io, i, argv[1]);
  }

  for (std::thread& client : clients) {
    client.join();
  }

  return EXIT_SUCCESS;
}
