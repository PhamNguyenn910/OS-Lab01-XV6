#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

/*
  diff: So sánh 2 file theo dòng, in ra các dòng khác nhau.
  Cú pháp:
    diff file1 file2       (so sánh chi tiết)
    diff -q file1 file2    (chỉ báo khác nhau hay không)
*/

static int
readline(int fd, char *buf, int max)
{
  // Trả về:
  //  - số byte đọc được (>0) nếu đọc được ít nhất 1 ký tự
  //  - 0 nếu EOF ngay từ đầu
  //  - -1 nếu lỗi read
  int i = 0;
  char c;

  while (i + 1 < max) {
    int n = read(fd, &c, 1);
    if (n == 0) break;       // EOF
    if (n < 0) return -1;    // lỗi
    buf[i++] = c;
    if (c == '\n') break;    // kết thúc dòng
  }

  buf[i] = '\0';
  return i;
}

int
main(int argc, char *argv[])
{
  int fdsrc, fddst;
  int q_mode = 0;
  char *file1, *file2;

  // Parse arguments: diff [-q] file1 file2
  if (argc == 3) {
    file1 = argv[1];
    file2 = argv[2];
  } else if (argc == 4 && strcmp(argv[1], "-q") == 0) {
    q_mode = 1;
    file1 = argv[2];
    file2 = argv[3];
  } else {
    fprintf(2, "usage: diff [-q] file1 file2\n");
    exit(1);
  }

  // Mở file1 (read-only)
  fdsrc = open(file1, O_RDONLY);
  if (fdsrc < 0) {
    fprintf(2, "diff: cannot open %s\n", file1);
    exit(1);
  }

  // Mở file2 (read-only)
  fddst = open(file2, O_RDONLY);
  if (fddst < 0) {
    fprintf(2, "diff: cannot open %s\n", file2);
    close(fdsrc);
    exit(1);
  }

  char line1[256];
  char line2[256];
  int lineNo = 1;
  int different = 0;

  while (1) {
    int n = readline(fdsrc, line1, sizeof(line1));
    if (n < 0) {
      fprintf(2, "diff: read error on %s\n", file1);
      close(fdsrc); close(fddst);
      exit(1);
    }

    int m = readline(fddst, line2, sizeof(line2));
    if (m < 0) {
      fprintf(2, "diff: read error on %s\n", file2);
      close(fdsrc); close(fddst);
      exit(1);
    }

    // Cả 2 EOF, dừng
    if (n == 0 && m == 0) break;

    // file1 hết trước (EOF), file2 còn dòng
    if (n == 0) {
      different = 1;
      if (q_mode) {
        printf("diff: files differ\n");
        close(fdsrc); close(fddst);
        exit(0);
      }
      // In hết phần còn lại của file2
      do {
        printf("%s:%d: < EOF\n", file1, lineNo);
        printf("%s:%d: > %s", file2, lineNo, line2);
        if (line2[strlen(line2)-1] != '\n') printf("\n");
        lineNo++;
      } while ((m = readline(fddst, line2, sizeof(line2))) > 0);
      break;
    }

    // file2 hết trước (EOF), file1 còn dòng
    if (m == 0) {
      different = 1;
      if (q_mode) {
        printf("diff: files differ\n");
        close(fdsrc); close(fddst);
        exit(0);
      }
      // In hết phần còn lại của file1
      do {
        printf("%s:%d: < %s", file1, lineNo, line1);
        if (line1[strlen(line1)-1] != '\n') printf("\n");
        printf("%s:%d: > EOF\n", file2, lineNo);
        lineNo++;
      } while ((n = readline(fdsrc, line1, sizeof(line1))) > 0);
      break;
    }

    // Cả 2 đều có dòng
    if (strcmp(line1, line2) != 0) {
      different = 1;
      if (q_mode) {
        printf("diff: files differ\n");
        close(fdsrc); close(fddst);
        exit(0);
      }
      printf("%s:%d: < %s", file1, lineNo, line1);
      if (line1[strlen(line1)-1] != '\n') printf("\n");
      printf("%s:%d: > %s", file2, lineNo, line2);
      if (line2[strlen(line2)-1] != '\n') printf("\n");
    }

    lineNo++;
  }

  // Nếu giống hệt thì không in gì
  (void)different;

  close(fdsrc);
  close(fddst);
  exit(0);
}