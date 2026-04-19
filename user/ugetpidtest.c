#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"


//Viết chương trình user để kiểm tra
int main(int argc, char *argv[]) {
  // Gọi system call bình thường (tốn thời gian switch context)
  int normal_pid = getpid();
  
  // Gọi hàm "nhanh" đọc trực tiếp từ vùng nhớ chia sẻ
  int fast_pid = ugetpid();

  // In ra hai giá trị để kiểm tra 
  printf("Normal getpid() returned: %d\n", normal_pid);
  printf("Fast ugetpid() returned: %d\n", fast_pid);

  // So sánh kết quả
  if (normal_pid == fast_pid) {
    printf("=> SUCCESS: Vung nho chia se hoat dong tot!\n");
  } else {
    printf("=> FAILED: PIDs khong khop.\n");
  }

  exit(0);
}