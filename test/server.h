#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include <sys/socket.h>

#include <cstddef>
#include <cstdio>
#include <exception>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "boost/asio/buffer.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/read.hpp"

pid_t LaunchClient(int num_clients);

class Session {
 public:
  explicit Session(boost::asio::ip::tcp::socket session)
      : session_(std::move(session)) {}

  size_t Read(std::string& output) {
    constexpr const int kBufferSize = 65535;
    auto buffer = std::unique_ptr<char[]>(new char[kBufferSize]);

    output.clear();

    boost::system::error_code error;

    while (true) {
      session_.wait(boost::asio::ip::tcp::socket::wait_read, error);
      if (error) {
        printf("Failed to wait to read packet\n");
        break;
      }

      int bytes = boost::asio::read(
          session_, boost::asio::buffer(buffer.get(), kBufferSize), error);
      if (error == boost::asio::error::eof) {
        printf("Client closed the connection\n");
        session_.close();
        break;
      }

      if (error) {
        printf("An error occurred while reading client packet\n");
        break;
      }

      output.append(buffer.get(), bytes);
    }

    return output.size();
  }

 private:
  boost::asio::ip::tcp::socket session_;
};

class Server {
 public:
  Server(boost::asio::io_context& io, int port)
      : acceptor_(io),
        endpoint_(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {}

  std::vector<std::unique_ptr<Session>> AcceptNumClients(int clients) {
    std::vector<std::unique_ptr<Session>> sessions;
    std::thread thread([&] {
      for (int i = 0; i < clients; ++i) {
        printf("Waiting for client id: %d\n", i);
        sessions.emplace_back(Accept());
        printf("Success to accept client id: %d\n", i);
      }
    });

    if (LaunchClient(clients) <= 0) {
      printf("Failed to launch clients\n");
      sessions.clear();
    }

    try {
      thread.join();
    } catch (const std::exception& e) {
      // nothing
    }

    return sessions;
  }

  std::unique_ptr<Session> Accept() {
    boost::system::error_code error;

    if (error) {
      printf("Failed to listen port %d\n", endpoint_.port());
      return nullptr;
    }

    acceptor_.wait(boost::asio::socket_base::wait_read, error);
    auto socket = acceptor_.accept(error);

    if (error) {
      printf("Failed to accept client\n");
      return nullptr;
    }

    return std::make_unique<Session>(std::move(socket));
  }

  void Listen() {
    acceptor_.open(endpoint_.protocol());

    boost::asio::socket_base::reuse_address reuse_address(true);
    acceptor_.set_option(reuse_address);

    acceptor_.bind(endpoint_);
    acceptor_.listen();
  }

  void Close() { acceptor_.close(); }

 private:
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::endpoint endpoint_;
};

#endif  // TEST_SERVER_H_
