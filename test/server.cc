#include "test/server.h"

#include <unistd.h>

#include <cstdio>
#include <string>

pid_t LaunchClient(int num_clients) {
  pid_t pid = fork();
  if (pid == 0) {
    if (execlp("bazel-bin/client/client", "bazel-bin/client/client", "7696",
               std::to_string(num_clients).c_str(), nullptr) == -1) {
      perror("asdf");
      return -1;
    }
  } else if (pid < 0) {
    printf("fork() failed\n");
    return -1;
  }
  return pid;
}
