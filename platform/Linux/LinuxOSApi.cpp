#include "LinuxOSApi.h"
#include <unistd.h>

bool LinuxOSApi::OpenFileInDefaultApp(const std::string& filepath) {
    pid_t pid = fork();
    if (pid == 0) {
      execl("/usr/bin/xdg-open", "xdg-open", filepath.c_str(), (char *)0);
      exit(1);
    }
    return true;
}