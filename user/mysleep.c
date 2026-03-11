#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  // Yêu cầu "nếu có 2 dòng" (2 tham số) thì mới chạy
  if (argc != 2) {
    fprintf(2, "Huong dan su dung: mysleep <so_ticks>\n");
    exit(1);
  }

  // ĐÂY LÀ DÒNG QUAN TRỌNG: Thay vì sleep(100), ta đọc từ tham số truyền vào
  // atoi: chuyển chuỗi ký tự (ví dụ "10") thành số nguyên (10)
  // argv[1]: là tham số thứ 2 (số ticks người dùng nhập)
  sleep(atoi(argv[1]));

  exit(0);
}