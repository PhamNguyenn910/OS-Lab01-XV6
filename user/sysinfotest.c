#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

void
testcall(void)
{
    struct sysinfo info;
    if (sysinfo(&info) < 0) {
        printf("sysinfotest: sysinfo call FAILED\n");
        exit(1);
    }
    printf("testcall: OK (freemem=%lu, nproc=%lu)\n",
           info.freemem, info.nproc);
}

void
testmem(void)
{
    struct sysinfo before, after;
    sysinfo(&before);

    char *p = sbrk(4096);
    if (p == (char *)-1) {
        printf("testmem: sbrk FAILED\n");
        exit(1);
    }
    sysinfo(&after);

    if (after.freemem >= before.freemem) {
        printf("testmem: freemem did not decrease! before=%lu after=%lu\n",
               before.freemem, after.freemem);
        exit(1);
    }

    if (after.freemem < before.freemem - 4096) {
        printf("testmem: freemem decreased by more than 4096! before=%lu after=%lu\n",
               before.freemem, after.freemem);
        exit(1);
    }

    printf("testmem: after allocate memory  —  freemem: %lu\n",
           after.freemem);

    sbrk(-4096);
    sysinfo(&after);

    if (after.freemem != before.freemem) {
        printf("testmem: freemem not restored! before=%lu after=%lu\n",
               before.freemem, after.freemem);
        exit(1);
    }

    printf("testmem: after release memory  —  freemem: %lu\n",
           after.freemem);

    printf("testmem: OK\n");
}

void
testsysinfo(void)
{
    struct sysinfo before, after;
    int pid;

    sysinfo(&before);
    pid = fork();

    if (pid == 0) {
        char *p = sbrk(4096);
        (void)p;
        sleep(100);
        exit(0);
    }

    sleep(1);

    sysinfo(&after);

    if (after.nproc != before.nproc + 1) {
        printf("testsysinfo: nproc did not increase! before=%lu after=%lu\n",
               before.nproc, after.nproc);
        kill(pid);
        wait(0);
        exit(1);
    }

    if (after.freemem >= before.freemem) {
        printf("testsysinfo: freemem did not decrease after fork! before=%lu after=%lu\n",
               before.freemem, after.freemem);
        kill(pid);
        wait(0);
        exit(1);
    }

    printf("testsysinfo: after fork  — nproc: %lu, freemem: %lu\n",
           after.nproc, after.freemem);

    kill(pid);
    wait(0);

    sysinfo(&after);

    if (after.nproc != before.nproc) {
        printf("testsysinfo: nproc not restored! before=%lu after=%lu\n",
               before.nproc, after.nproc);
        exit(1);
    }

    if (after.freemem != before.freemem) {
        printf("testsysinfo: freemem not restored! before=%lu after=%lu\n",
               before.freemem, after.freemem);
        exit(1);
    }

    printf("testsysinfo: after kill — nproc: %lu, freemem: %lu\n",
           after.nproc, after.freemem);

    printf("testsysinfo: OK\n");
}

int
main(void)
{
    printf("sysinfotest: start\n");
    testcall();
    testmem();
    testsysinfo();
    printf("sysinfotest: OK\n");
    exit(0);
}