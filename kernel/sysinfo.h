#include <stdint.h>

struct sysinfo {
    uint64_t freemem;   // bytes of free memory
    uint64_t nproc;     // number of non-UNUSED processes
};