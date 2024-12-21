load(
  "//:copts/compiler_copts.bzl",
  "ASYNC_TEST_GCC_FLAGS"
)

ASYNC_TEST_DEFAULT_COPTS = select({
  "//conditions:default": ASYNC_TEST_GCC_FLAGS,
})
