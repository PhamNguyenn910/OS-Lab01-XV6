#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc != 2){  // kiem tra xem co du 2 tham so hay khong
    fprintf(2, "Command: sleep <ticks>\n"); // Neu khong du 2 tham so, thong bao dinh dang cau lenh dung va ngung chuong trinh
    exit(1);
  }
  
  int ticks = atoi(argv[1]);
  
  printf("Sleeping for %d ticks...\n", ticks);
  sleep(ticks);
  printf("Woke up!\n");
  exit(0);
}