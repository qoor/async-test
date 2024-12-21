load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

load("@bazel_skylib//lib:selects.bzl", "selects")

config_setting(
  name = "clang_compiler",
  flag_values = {
    "@bazel_tools//tools/cpp:compiler": "clang",
  },
  visibility = [":__subpackages__"],
)

config_setting(
  name = "gcc_compiler",
  flag_values = {
    "@bazel_tools//tools/cpp:compiler": "gcc",
  },
  visibility = [":__subpackages__"],
)

config_setting(
  name = "mingw_unspecified_compiler",
  flag_values = {
    "@bazel_tools//tools/cpp:compiler": "mingw",
  },
  visibility = [":__subpackages__"],
)

config_setting(
  name = "mingw-gcc_compiler",
  flag_values = {
    "@bazel_tools//tools/cpp:compiler": "mingw-gcc",
  },
  visibility = [":__subpackages__"],
)

config_setting(
  name = "msvc_compiler",
  flag_values = {
    "@bazel_tools//tools/cpp:compiler": "msvc-cl",
  },
  visibility = [":__subpackages__"],
)

config_setting(
  name = "clang-cl_compiler",
  flag_values = {
    "@bazel_tools//tools/cpp:compiler": "clang-cl",
  },
  visibility = [":__subpackages__"],
)

config_setting(
  name = "osx",
  constraint_values = [
    "@platforms//os:osx",
  ],
)

config_setting(
  name = "ios",
  constraint_values = [
    "@platforms//os:ios",
  ],
)

config_setting(
  name = "ppc",
  constraint_values = [
    "@platforms//cpu:ppc",
  ],
  visibility = [":__subpackages__"],
)

config_setting(
  name = "platforms_wasm32",
  constraint_values = [
    "@platforms//cpu:wasm32",
  ],
  visibility = [":__subpackages__"],
)

config_setting(
  name = "platforms_wasm64",
  constraint_values = [
    "@platforms//cpu:wasm64",
  ],
  visibility = [":__subpackages__"],
)

selects.config_setting_group(
  name = "wasm",
  match_any = [
    ":platforms_wasm32",
    ":platforms_wasm64",
  ],
  visibility = [":__subpackages__"],
)

config_setting(
  name = "fuchsia",
  constraint_values = [
    "@platforms//os:fuchsia",
  ],
  visibility = [":__subpackages__"],
)

selects.config_setting_group(
  name = "mingw_compiler",
  match_any = [
    ":mingw_unspecified_compiler",
    ":mingw-gcc_compiler",
  ],
  visibility = [":__subpackages__"],
)

refresh_compile_commands(
  name = "refresh_compile_commands",
  targets = {
    "//...:all": "",
  },
)
