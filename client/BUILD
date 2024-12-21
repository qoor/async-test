load(
  "//:copts/configure_copts.bzl",
  "ASYNC_TEST_DEFAULT_COPTS"
)

cc_binary(
  name = "client",

  srcs = [
    "client.cc", "client.h"
  ],

  deps = [
    "@boost//:asio"
  ],

  data = ["//data:1G.dummy"],

  copts = ASYNC_TEST_DEFAULT_COPTS,
)
