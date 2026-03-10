#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

/*
  cp: Copy src -> dst
  Cú pháp: cp src dst

  Lưu ý hiệu năng:
  - KHÔNG đọc từng byte một (gọi system call 1 triệu lần cho file 1MB = rất chậm).
  - Dùng buffer 512 bytes: mỗi lần read múc hẳn 512 bytes, giảm số lần gọi system call
    xuống ~2000 lần thay vì 1,000,000 lần.
*/

#define BUF_SIZE 512

int
main(int argc, char *argv[])
{
  int fdsrc, fddst;
  char buf[BUF_SIZE];
  int n;

  // Kiểm tra đủ đối số
  if (argc != 3) {
    fprintf(2, "usage: cp src dst\n");
    exit(1);
  }

  // Mở file nguồn (read-only)
  fdsrc = open(argv[1], O_RDONLY);
  if (fdsrc < 0) {
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  // Mở file đích (tạo mới hoặc ghi đè)
  // O_WRONLY | O_CREATE | O_TRUNC: ghi, tạo nếu chưa có, xoá nội dung cũ nếu đã có
  fddst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC);
  if (fddst < 0) {
    fprintf(2, "cp: cannot open/create %s\n", argv[2]);
    close(fdsrc);
    exit(1);
  }

  // Copy theo buffer (512 bytes mỗi lần) - tránh bottleneck system call
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

  // Kiểm tra lỗi read
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