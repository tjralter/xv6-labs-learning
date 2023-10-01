#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  if(argint(0, &pid) < 0)
    return -1;
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

  if(argint(0,&mask)<0) //从a0寄存器中获取参数并赋值给mask
    return -1;
  
  myproc()->tmask=mask; //将mask存到proc结构体中的tmask
  return 0;
}

uint64
sys_sysinfo(void)
{
  uint64 addr;
  argaddr(0,&addr); // 获取当前进程下的页表下的trapframe的寄存器a0的虚拟地址

  struct sysinfo sinfo;

  // 根据计算好的数据把sinfo配置好
  sinfo.freemem=count_freemem();  //kernel/kalloc.c
  sinfo.nproc=count_liveproc(); //kernel/proc.c

  //使用copyout，结合当前进程的页表，获得进程传进来的指针（逻辑地址）对应的物理地址
  //然后将&sinfo中的数据复制到该指针所指位置，供用户进程使用
  if(copyout(myproc()->pagetable,addr,(char*)&sinfo,sizeof(sinfo))<0)
  {
    return -1;
  }
  return 0;
}