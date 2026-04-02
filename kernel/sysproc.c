#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

struct ptreeinfo {
  int pid;
  int ppid;
  int state;
  uint64 memsize;
  char name[16];
};

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
  uint64 u_buf; // Địa chỉ buffer từ User Space
  int max;      // Số lượng tối đa
  struct ptreeinfo k_buf[64]; // Mảng tạm trong Kernel (NPROC tối đa là 64)
  struct proc *p;
  int count = 0;

argaddr(0, &u_buf);
argint(1, &max);

  if(max <= 0 || u_buf == 0) return -1;

  // 2. Duyệt bảng tiến trình trong Kernel
  // Lưu ý: Biến 'proc' là mảng chứa tất cả tiến trình, định nghĩa trong proc.c
  extern struct proc proc[]; 

  for(int i = 0; i < 64 && count < max; i++){
    p = &proc[i];
    acquire(&p->lock); // Khóa tiến trình để đọc an toàn
    
    if(p->state != UNUSED){
      k_buf[count].pid = p->pid;
      k_buf[count].ppid = p->parent ? p->parent->pid : 0;
      k_buf[count].state = p->state;
      k_buf[count].memsize = p->sz;
      safestrcpy(k_buf[count].name, p->name, sizeof(p->name));
      count++;
    }
    
    release(&p->lock); // Giải phóng khóa
  }

  // 3. Chép dữ liệu từ Kernel ra User Space an toàn
  if(copyout(myproc()->pagetable, u_buf, (char *)k_buf, count * sizeof(struct ptreeinfo)) < 0)
    return -1;

  return count; // Trả về số lượng tiến trình đã thu thập
}

uint64
sys_sysinfo(void)
{
    struct sysinfo info;
    uint64 addr;

    // Get user-space pointer argument
    /*if (argaddr(0, &addr) <0)
        return -1;*/
    argaddr(0, &addr);

    info.freemem = getfreemem();
    info.nproc   = getnproc();

    if (copyout(myproc()->pagetable, addr,
                (char *)&info, sizeof(info)) < 0)
        return -1;

    return 0;
}