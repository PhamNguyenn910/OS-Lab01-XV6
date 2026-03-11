#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // 1. KHỞI TẠO PIPE (Phải làm TRƯỚC KHI gọi fork)
    // p1 dùng để Cha truyền dữ liệu cho Con.
    // p2 dùng để Con truyền dữ liệu ngược lại cho Cha.
    int p1[2]; 
    int p2[2]; 
    
    // Mảng buf 1 byte để chứa dữ liệu
    char buf[1]; 

    // Gọi syscall pipe() để nhờ Kernel tạo ống.
    // Nếu trả về giá trị < 0 nghĩa là hệ thống hết tài nguyên, phải báo lỗi và thoát ngay.
    if (pipe(p1) < 0 || pipe(p2) < 0) {
        fprintf(2, "pingpong: Khoi tao pipe that bai\n");
        exit(1);
    }

    // 2. NHÂN BẢN TIẾN TRÌNH
    // Syscall fork() tách chương trình làm 2 nhánh chạy song song.
    // Cả 2 nhánh đều kế thừa chung 2 cái ống p1 và p2 vừa tạo ở trên.
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "pingpong: fork that bai\n");
        exit(1);
    }

    // 3. ĐIỀU HƯỚNG LOGIC CHO TỪNG NHÁNH
    if (pid > 0) { 
        // ======================= TIẾN TRÌNH CHA =======================
        // Nhiệm vụ: Gửi 1 byte -> Đợi đọc 1 byte -> In "received pong"
        
        // Đóng các đầu ống mà Cha không bao giờ xài tới.
        // Cha KHÔNG ĐỌC từ ống 1 và KHÔNG GHI vào ống 2.
        close(p1[0]); 
        close(p2[1]); 

        // Hành động 1: Cha ghi 1 byte (ký tự 'x') vào đầu ghi của ống 1
        write(p1[1], "x", 1);
        
        // Rót dữ liệu xong, phải lập tức ĐÓNG VAN GHI ống 1 lại.
        // Việc này giúp báo cho tiến trình Con biết là "Đã gửi xong".
        close(p1[1]); 

        // Hành động 2: Cha gọi hàm read() ở đầu đọc ống 2 để đợi phản hồi từ Con.
        // Hàm read() sẽ tự động "block" (đứng chờ) cho đến khi Con thực sự gửi dữ liệu qua.
        if (read(p2[0], buf, 1) > 0) {
            // Dùng syscall getpid() để lấy ID của tiến trình Cha hiện tại và in ra.
            printf("%d: received pong\n", getpid());
        }
        
        // Đọc xong thì đóng nốt cái van đọc của ống 2.
        close(p2[0]);

        // Hành động 3: Dọn dẹp xác tiến trình Con.
        // Hàm wait(0) bắt Cha đứng im đợi cho đến khi Con chạy xong lệnh exit() ở dưới.
        // Tránh để lại Zombie Process gây rác bộ nhớ hệ thống.
        wait(0);
        
        // Cha kết thúc vòng đời.
        exit(0);

    } else if (pid == 0) { 
        // ======================= TIẾN TRÌNH CON =======================
        // Nhiệm vụ: Đợi đọc 1 byte -> In "received ping" -> Gửi 1 byte lại cho Cha
        
        // QUAN TRỌNG: Đóng các đầu ống mà Con không bao giờ xài tới.
        // Con KHÔNG GHI vào ống 1 và KHÔNG ĐỌC từ ống 2.
        close(p1[1]); 
        close(p2[0]); 

        // Hành động 1: Con gọi hàm read() ở đầu đọc ống 1 để đợi Cha gửi dữ liệu xuống.
        // Tương tự, read() sẽ block tiến trình Con cho đến khi có byte đi vào.
        if (read(p1[0], buf, 1) > 0) {
            // Dùng syscall getpid() để lấy ID của tiến trình Con hiện tại.
            printf("%d: received ping\n", getpid());
        }

        // Đọc xong phần của mình thì đóng van đọc ống 1 lại.
        close(p1[0]);

        // Hành động 2: Con ghi trả 1 byte (ký tự 'y') vào đầu ghi của ống 2 để báo hiệu cho Cha.
        write(p2[1], "y", 1);
        
        // Gửi xong thì đóng nốt van ghi ống 2.
        close(p2[1]);

        // Con hoàn thành nhiệm vụ và thoát chương trình.
        exit(0);
    }
}