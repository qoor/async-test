cc_binary(
  name = "async-test",

  srcs = [
    "async_test.cc",
    "server.cc",
    "server.h",
    "test_timer.h",
  ],
  deps = [
    "@boost//:asio"
  ],
  data = ["//data:1G.dummy"],
)
