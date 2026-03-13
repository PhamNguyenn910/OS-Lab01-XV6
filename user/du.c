#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

long du(char *path, int flag_a, int flag_s) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    long total_size = 0;


    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "du: cannot open %s\n", path);
        return 0;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "du: cannot stat %s\n", path);
        close(fd);
        return 0;
    }

    // 1. Nếu là FILE
    if(st.type == T_FILE){
        // Cờ -a yêu cầu in cả file (và không bị cờ -s ghi đè)
        if(flag_a && !flag_s) {
            printf("%d\t%s\n", (int)st.size, path);
        }
        close(fd);
        return st.size; // Trả về size của file
    }

    // 2. Nếu là THƯ MỤC
    if(st.type == T_DIR){
        // Bắt đầu đếm từ 0
        total_size = 0; 

        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("du: path too long\n");
            close(fd);
            return 0;
        }

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        // Đọc từng file/thư mục con
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0) continue;

            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            //Dệ quy: Cộng dồn dung lượng của mục con vào tổng của cha
            total_size += du(buf, flag_a, flag_s);
        }

        // In ra màn hình thư mục hiện tại sau khi đã duyệt xong tất cả các con bên trong
        if(!flag_s) {
            printf("%d\t%s\n", (int)total_size, path); // Format: <bytes>\t<path>
        }

        close(fd);
        return total_size;
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    char *path = ".";
    int flag_a = 0;
    int flag_s = 0;

    // Phân tích tham số đầu vào
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-a") == 0) {
            flag_a = 1;
        } else if(strcmp(argv[i], "-s") == 0) {
            flag_s = 1;
        } else {
            // Nếu không phải cờ, nó là đường dẫn
            path = argv[i];
        }
    }

    // Khởi chạy đệ quy
    long total = du(path, flag_a, flag_s);

    // -s: Chỉ in ra duy nhất 1 con số tổng cộng ở bước cuối cùng
    if(flag_s) {
        printf("%d\n", (int)total); // Output chỉ có số byte
    }

    exit(0);
}