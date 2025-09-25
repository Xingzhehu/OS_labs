#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

extern struct proc proc[NPROC];  // 引入全局进程表

uint64 sys_yield(void) {
    struct proc *p = myproc();  // 获取当前进程
    // 打印当前进程的上下文保存地址范围
    printf("Save the context of the process to the memory region from address %p to %p\n", 
           (void *)&p->context, (void *)&p->context + sizeof(p->context));

    // 打印当前进程的PID和用户态PC值（从trapframe获取）
    printf("Current running process pid is %d and user pc is %p\n", 
           p->pid, (void *)(p->trapframe->epc));

    // 寻找下一个可运行的进程
    struct proc *next_proc = 0;
    for (int i = 0; i < NPROC; i++) {
        if (proc[i].state == RUNNABLE) {
            next_proc = &proc[i];
            break;
        }
    }

    if (next_proc) {
        // 打印下一个可运行进程的PID和用户态PC值
        printf("Next runnable process pid is %d and user pc is %p\n", 
               next_proc->pid, (void *)(next_proc->trapframe->epc));
    }

    yield();  // 调用内核中的yield函数进行调度

    return 0;  // 返回值可以根据需求修改
}



uint64 sys_exit(void) {
  int n;
  if (argint(0, &n) < 0) return -1;
  exit(n);
  return 0;  // not reached
}

uint64 sys_getpid(void) { return myproc()->pid; }

uint64 sys_fork(void) { return fork(); }

uint64 sys_wait(void) {
  uint64 addr;
  int flags;

  if (argaddr(0, &addr) < 0 || argint(1, &flags) < 0) {
    return -1;
  }
  return wait(addr, flags);
}

uint64 sys_sbrk(void) {
  int addr;
  int n;

  if (argint(0, &n) < 0) return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0) return -1;
  return addr;
}

uint64 sys_sleep(void) {
  int n;
  uint ticks0;

  if (argint(0, &n) < 0) return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (myproc()->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64 sys_kill(void) {
  int pid;

  if (argint(0, &pid) < 0) return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_rename(void) {
  char name[16];
  int len = argstr(0, name, MAXPATH);
  if (len < 0) {
    return -1;
  }
  struct proc *p = myproc();
  memmove(p->name, name, len);
  p->name[len] = '\0';
  return 0;
}
