load(
  "//:copts/configure_copts.bzl",
  "ASYNC_TEST_DEFAULT_COPTS"
)

cc_binary(
  name = "test",

  srcs = [
    "main.cc",
    "server.cc",
    "server.h",
    "test_timer.h",
    "util.cc",
    "util.h",
  ],

  deps = [
    "@abseil-cpp//absl/strings:str_format",
    "@boost//:asio",
    "@libuv//:libuv",
  ],

  data = glob(["**/*.dummy"]),

  copts = ASYNC_TEST_DEFAULT_COPTS,
)
