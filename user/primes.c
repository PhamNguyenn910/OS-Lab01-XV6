#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Sử dụng __attribute__((noreturn)) để tránh cảnh báo đệ quy vô hạn từ trình biên dịch
__attribute__((noreturn))
void sieve(int left_pipe[2]) {
    int p;
    
    // Tiến trình hiện tại chỉ đọc dữ liệu từ left_pipe, do đó cần đóng ngay đầu ghi.
    close(left_pipe[1]);

    // Đọc số đầu tiên từ pipe. Theo thuật toán, số đầu tiên luôn là số nguyên tố.
    // Nếu read trả về 0 (EOF), nghĩa là tiến trình cha đã đóng pipe, không còn dữ liệu -> thoát.
    if (read(left_pipe[0], &p, sizeof(p)) == 0) {
        close(left_pipe[0]);
        exit(0);
    }
    
    // In ra số nguyên tố tìm được
    printf("prime %d\n", p);

    // Khởi tạo pipe mới để truyền các số không phải là bội của p sang tiến trình con
    int right_pipe[2];
    if (pipe(right_pipe) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid > 0) {
        // ================= TIẾN TRÌNH CHA =================
        // Cha chỉ ghi dữ liệu vào right_pipe, không đọc từ right_pipe
        close(right_pipe[0]);
        
        int n;
        // Liên tục đọc các số còn lại từ left_pipe do tiến trình trước đó truyền tới
        while (read(left_pipe[0], &n, sizeof(n)) > 0) {
            // Nếu n không chia hết cho p, truyền n sang cho tiến trình con xử lý
            if (n % p != 0) {
                write(right_pipe[1], &n, sizeof(n));
            }
        }
        
        // Dọn dẹp tài nguyên sau khi đã truyền xong dữ liệu:
        // 1. Đóng đầu đọc của left_pipe (đã dùng xong).
        close(left_pipe[0]);
        // 2. Đóng đầu ghi của right_pipe để gửi tín hiệu EOF (End of File) cho tiến trình con.
        close(right_pipe[1]);
        
        // Chờ tiến trình con kết thúc để tránh tạo ra Zombie process
        wait(0);
        exit(0);

    } else {
        // ================= TIẾN TRÌNH CON =================
        // ĐÂY LÀ BƯỚC QUAN TRỌNG ĐỂ TRÁNH RÒ RỈ FILE DESCRIPTOR:
        // Con thừa kế các FDs từ cha, nhưng nó không cần dùng đến left_pipe của cha
        close(left_pipe[0]);
        
        // Con cũng không ghi vào right_pipe (việc ghi là do cha làm).
        // Nếu không đóng, đầu đọc của con sẽ bị treo (deadlock) vì pipe này không bao giờ báo EOF.
        close(right_pipe[1]);
        
        // Tiến hành đệ quy: right_pipe của cha bây giờ sẽ trở thành left_pipe của con
        sieve(right_pipe); 
    }
}

int main(int argc, char *argv[]) {
    int initial_pipe[2];
    if (pipe(initial_pipe) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid > 0) {
        // ================= TIẾN TRÌNH MAIN (GỐC) =================
        // Main chỉ làm nhiệm vụ tạo nguồn số, không đọc
        close(initial_pipe[0]); 
        
        // Truyền các số nguyên từ 2 đến 280 (mỗi số 4-byte) vào pipe đầu tiên
        for (int i = 2; i <= 280; i++) {
            write(initial_pipe[1], &i, sizeof(i));
        }
        
        // Rất quan trọng: Phải đóng đầu ghi sau khi truyền xong để kích hoạt tín hiệu EOF
        close(initial_pipe[1]);
        
        // Đợi toàn bộ pipeline (chuỗi các tiến trình con, cháu) kết thúc
        wait(0);
        exit(0);

    } else {
        // ================= TIẾN TRÌNH CON ĐẦU TIÊN =================
        // Tiến trình này chỉ nhận dữ liệu từ Main, không ghi ngược lại
        close(initial_pipe[1]);
        
        // Bắt đầu chuỗi đệ quy xử lý Sàng nguyên tố
        sieve(initial_pipe);
    }
    
    return 0;
}