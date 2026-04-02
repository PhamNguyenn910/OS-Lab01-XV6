#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/ptree.h"

// Hàm đệ quy để in cây tiến trình
void print_tree(int ppid, int indent, struct ptreeinfo *buf, int count) {
    for (int i = 0; i < count; i++) {
        if (buf[i].ppid == ppid) {
            // Tạo khoảng thụt lề
            for (int j = 0; j < indent; j++) printf("  ");
            
printf("|-- %d %s (state=%d, mem=%d)\n", 
       buf[i].pid, buf[i].name, buf[i].state, (int)buf[i].memsize);
            
            // Tìm các con của tiến trình hiện tại
            print_tree(buf[i].pid, indent + 1, buf, count);
        }
    }
}

int main() {
    struct ptreeinfo buf[64];
    int count = ptree(buf, 64);
    
    if (count < 0) {
        printf("pstree: loi goi system call\n");
        exit(1);
    }

    printf("Cay tien trinh he thong:\n");
    // Bắt đầu in từ tiến trình gốc (ppid = 0)
    print_tree(0, 0, buf, count);
    
    exit(0);
}