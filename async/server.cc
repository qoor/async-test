#include "async/server.h"

#include <unistd.h>

bool LaunchClient(int num_clients) {
  printf("Launching %d clients...\n", num_clients);

  pid_t pid = fork();
  if (pid == 0) {
    if (execl("bazel", "bazel", "run", "//client:tcp-client", 7696,
              num_clients) == -1) {
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
