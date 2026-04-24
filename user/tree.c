#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void tree(char *path, int level, int maxDepth, int onlyDir) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Mở file/thư mục
    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    // Lấy thông tin file/thư mục
    if(fstat(fd, &st) < 0){
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (level == 0) {
        printf("%s\n", path);
    }
    // Xử lý nếu đường dẫn gốc truyền vào là một file bình thường
    if(st.type == T_FILE){
        close(fd);
        return;
    }

    // Xử lý nếu là thư mục (T_DIR)
    if(st.type == T_DIR){
        if (maxDepth > 0 && level >= maxDepth) {
            close(fd);
            return;
        }

        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("tree: path too long\n");
            close(fd);
            return;
        }

        // Bắt đầu nối chuỗi: buf = path + '/'
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        // Đọc từng mục (entry) bên trong thư mục
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0) continue;
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            struct stat child_st;
            if(stat(buf, &child_st) < 0) continue;

            if(onlyDir && child_st.type == T_FILE) continue;


            for(int i = 0; i < level; i++){
                printf("|   "); 
            }
            char name[DIRSIZ + 1];
            memmove(name, de.name, DIRSIZ);
            name[DIRSIZ] = 0;  // Null terminator
            printf("|__ %s\n", name);

            // Gọi đệ quy cho thư mục con
            if(child_st.type == T_DIR){
                tree(buf, level + 1, maxDepth, onlyDir);
            }
        }
    }
    close(fd);
}
int main(int argc, char *argv[]) {
    char *path = "."; // Mặc định duyệt thư mục hiện tại
    int maxDepth = 0;
    int onlyDir = 0;

    // Vòng lặp phân tích các tham số dòng lệnh (flags)
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-d") == 0) {
            onlyDir = 1;
        } else if(strcmp(argv[i], "-L") == 0) {
            // Kiểm tra xem phía sau -L có số đi kèm không
            if(i + 1 < argc) {
                maxDepth = atoi(argv[i+1]);
                i++; // Nhảy cóc qua phần tử chứa số để vòng lặp không đọc lại
            } else {
                fprintf(2, "tree: missing depth for -L\n");
                exit(1);
            }
        } else {
            // Nếu không phải cờ, thì nó là đường dẫn thư mục
            path = argv[i];
        }
    }

    // Khởi chạy đệ quy từ level 0
    tree(path, 0, maxDepth, onlyDir);
    
    exit(0);
}