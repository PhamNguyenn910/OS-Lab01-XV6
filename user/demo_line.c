#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"


/*
  demo_line: Minh hoạ đọc dữ liệu theo DÒNG (line-based I/O).
  Dùng để làm các bài cần xử lý theo dòng: diff, rgrep, xargs (đọc input từng dòng), ...

  Vì sao cần đọc theo dòng?
  - read(fd, buf, n) có thể trả về đoạn dữ liệu cắt ngang dòng.
  - Với bài cần "line number" hoặc so sánh theo dòng, ta cần gom đến '\n'.

  Cú pháp:
    demo_line          (đọc stdin)
    demo_line <file>   (đọc file)

  Cách làm đơn giản (phù hợp Lab01): read từng ký tự 1 byte cho đến '\n' hoặc EOF.
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

  while (i + 1 < max) {          // chừa 1 byte cho '\0'
    int n = read(fd, &c, 1);
    if (n == 0) {                // EOF
      break;
    }
    if (n < 0) {                 // lỗi
      return -1;
    }
    buf[i++] = c;
    if (c == '\n') {             // kết thúc dòng
      break;
    }
  }

  buf[i] = '\0';
  return i;
}

int
main(int argc, char *argv[])
{
  int fd = 0; // mặc định stdin

  if (argc == 2) {
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
      fprintf(2, "demo_line: cannot open %s\n", argv[1]);
      exit(1);
    }
  } else if (argc > 2) {
    fprintf(2, "usage: demo_line [file]\n");
    exit(1);
  }

  char line[256];
  int lineNo = 1;

  while (1) {
    int n = readline(fd, line, sizeof(line));
    if (n < 0) {
      fprintf(2, "demo_line: read error\n");
      if (fd != 0) close(fd);
      exit(1);
    }
    if (n == 0) {
      break; // EOF
    }

    // Demo: in số dòng + nội dung
    // Lưu ý: line đã có thể chứa '\n' cuối dòng. Nếu không có '\n', ta tự in thêm.
    printf("%d: %s", lineNo, line);
    if (line[n - 1] != '\n') printf("\n");
    lineNo++;
  }

  if (fd != 0) close(fd);
  exit(0);
}
