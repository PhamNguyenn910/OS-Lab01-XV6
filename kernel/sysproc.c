#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

#include "ptree.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int mask;

  // Lấy tham số (mask) từ user space truyền xuống
  argint(0, &mask);

  // Gán mask vào tiến trình hiện tại
  myproc()->traceID = mask;
  
  return 0;
}

uint64

sys_ptree(void)
{
  uint64 u_buf;
  int max;
  int count;
  struct ptreeinfo k_buf[NPROC];

  argaddr(0, &u_buf);
  argint(1, &max);

  if(u_buf == 0 || max <= 0)
    return -1;

  count = getprocs(k_buf, max);

  if(copyout(myproc()->pagetable, u_buf, (char *)k_buf,
             count * sizeof(struct ptreeinfo)) < 0)
    return -1;

  return count;
}

uint64
sys_sysinfo(void)
{
    struct sysinfo info;
    uint64 addr;
    struct ptreeinfo k_buf[NPROC];

    // Get user-space pointer argument
    /*if (argaddr(0, &addr) <0)
        return -1;*/
    argaddr(0, &addr);

    info.freemem = getfreemem();
    info.nproc   = getprocs(k_buf, NPROC);

    if (copyout(myproc()->pagetable, addr,
                (char *)&info, sizeof(info)) < 0)
        return -1;

    return 0;
}

uint64
sys_pgaccess(void)
{
  uint64 base;
  uint64 mask_addr;
  int npages;
  uint mask = 0;
  struct proc *p = myproc();

  argaddr(0, &base);
  argint(1, &npages);
  argaddr(2, &mask_addr);

  if(npages < 0 || npages > 32)
    return -1;

  for(int i = 0; i < npages; i++){
    uint64 va = base + i * PGSIZE;
    pte_t *pte = walk(p->pagetable, va, 0);

    if(pte == 0)
      continue;

    if((*pte & PTE_V) && (*pte & PTE_A)){
      mask |= (1U << i);
      *pte &= ~PTE_A;
    }
  }

  if(copyout(p->pagetable, mask_addr, (char *)&mask, sizeof(mask)) < 0)
    return -1;

  return 0;
}

