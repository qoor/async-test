cc_binary(
  name = "async-test",

  srcs = [
    "async_test.cc", "test_timer.h"
  ],
  deps = [
    "@boost//:filesystem", "@boost//:asio"
  ],
  data = ["//data:1G.dummy"],
)
