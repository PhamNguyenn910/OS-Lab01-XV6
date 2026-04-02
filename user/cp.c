#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"


#define BUF_SIZE 512

int
main(int argc, char *argv[])
{
  int fdsrc, fddst;
  char buf[BUF_SIZE];
  int n;

  if (argc != 3) {
    fprintf(2, "usage: cp src dst\n");
    exit(1);
  }

  fdsrc = open(argv[1], O_RDONLY);
  if (fdsrc < 0) {
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  fddst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC);
  if (fddst < 0) {
    fprintf(2, "cp: cannot open/create %s\n", argv[2]);
    close(fdsrc);
    exit(1);
  }

  while ((n = read(fdsrc, buf, BUF_SIZE)) > 0) {
    int written = 0;
    while (written < n) {
      int w = write(fddst, buf + written, n - written);
      if (w < 0) {
        fprintf(2, "cp: write error\n");
        close(fdsrc);
        close(fddst);
        exit(1);
      }
      written += w;
    }
  }

  if (n < 0) {
    fprintf(2, "cp: read error\n");
    close(fdsrc);
    close(fddst);
    exit(1);
  }

  close(fdsrc);
  close(fddst);
  exit(0);
}