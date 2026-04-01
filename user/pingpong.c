#include "kernel/types.h"
#include "user/user.h"
int main() {
    int fd[2]; // Mảng lưu 2 file descriptors
    char buffer[20];
    // Tạo pipe
    pipe(fd);
    // Tạo tiến trình con
    if (fork() == 0) {
    // Tiến trình con
    close(fd[1]); // Đóng đầu ghi vì tiến trình con chỉ đọc
    read(fd[0], buffer, sizeof(buffer));
    printf("Child received: %s\n", buffer);
    close(fd[0]);
    } else {
    // Tiến trình cha
    close(fd[0]); // Đóng đầu đọc vì tiến trình cha chỉ ghi
    write(fd[1], "Hello from parent", 18);
    close(fd[1]);
    }
    exit(0);
}
