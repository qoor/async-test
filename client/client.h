#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include <cstddef>
#include <string_view>

#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/write.hpp"

class Client {
 public:
  explicit Client(boost::asio::io_context& io) : socket_(io), resolver_(io) {}

  bool Connect(std::string_view ip, std::string_view port) {
    boost::system::error_code error;
    boost::asio::connect(socket_, resolver_.resolve(ip, port), error);
    if (error) {
      printf("Failed to connect to the server\n");
      return false;
    }
    return true;
  }

  size_t Write(std::string_view data) {
    boost::system::error_code error;
    size_t bytes =
        boost::asio::write(socket_, boost::asio::buffer(data), error);
    if (error) {
      printf("Failed to write data: %s\n", error.what().c_str());
      return 0;
    }

    return bytes;
  }

 private:
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver resolver_;
};

#endif  // CLIENT_CLIENT_H_
