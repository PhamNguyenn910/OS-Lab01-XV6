#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

// Hàm test yêu cầu 1: Tăng tốc System call (ugetpid)
void
ugetpid_test()
{
  int i;

  printf("ugetpid_test starting\n");
  for (i = 0; i < 64; i++) {
    int ret = fork();
    if (ret != 0) {
      wait(&ret);
      if (ret != 0)
        exit(1);
    } else {
      int pid = getpid();
      int upid = ugetpid(); // Gọi hàm ugetpid để đọc từ trang USYSCALL
      if (pid != upid) {
        printf("pid %d != upid %d\n", pid, upid);
        exit(1);
      }
      exit(0);
    }
  }
  printf("ugetpid_test: OK\n");
}

int
main(int argc, char *argv[])
{
  ugetpid_test();

  printf("pgtbltest: all tests succeed\n");
  exit(0);
}