#ifndef PTREE_H
#define PTREE_H
#include "types.h"

struct ptreeinfo {
    int pid;          // Process ID
    int ppid;         // Parent Process ID
    int state;        // Trạng thái (RUNNING, SLEEPING, ...)
    uint64 memsize;   // Kích thước bộ nhớ (bytes)
    char name[16];    // Tên tiến trình
};

#endif