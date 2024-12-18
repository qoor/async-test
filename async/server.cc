#include "async/server.h"

#include <unistd.h>

#include <cstdio>
#include <string>

bool LaunchClient(int num_clients) {
  pid_t pid = fork();
  if (pid == 0) {
    if (execlp("bazel-bin/client/tcp-client", "bazel-bin/client/tcp-client",
               "7696", std::to_string(num_clients).c_str(), nullptr) == -1) {
      perror("asdf");
      return false;
    }
  } else if (pid > 0) {
    // nothing
  } else {
    printf("fork() failed\n");
    return false;
  }
  return true;
}
