#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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

/******************************                   CHANNEL                   ******************************/

uint64 sys_channel_create(void)
{
  for (int i = 0; i < 10; i++) {
    acquire(&channels[i].lock);
  
    if (channels[i].creator_pid == -1 || channels[i].creator_pid== -2) {
      channels[i].creator_pid = myproc()->pid;
      release(&channels[i].lock);
      return i;
    }
    release(&channels[i].lock);
  }
  return -1;  // No Free channel
}

uint64 sys_channel_put(void)
{
  int cd, data;
  argint(0, &cd);
  argint(1, &data);
  
  if (cd < 0 || cd >= 10)
    return -1;

  acquire(&channels[cd].lock);

  while (channels[cd].isFree) {
    sleep(&channels[cd], &channels[cd].lock);
  }
  // if isFree changed to 0 from destroy and not from take , we want to exit with -1
  if(channels[cd].creator_pid==-2){
    release(&channels[cd].lock);
    return -1;
  }
  
  channels[cd].data = data;
  channels[cd].isFree = 1;
  wakeup(&channels[cd]);
  release(&channels[cd].lock);
  return 0;
}

uint64 sys_channel_take(void)
{
  int cd;
  uint64 add;
  argint(0, &cd);
  argaddr(1,&add);
  int * data = (int*)add;

  if (cd < 0 || cd >= 10)
    return -1;

  acquire(&channels[cd].lock);

  while (!channels[cd].isFree) {
    // if destroy changed isFree to 0 we want to exit with -1 , after being wokeup
    if(channels[cd].creator_pid==-2){
      release(&channels[cd].lock);
      return -1;
    }
    sleep(&channels[cd], &channels[cd].lock);
  }

  if (copyout(myproc()->pagetable, (uint64)data, (char*)&channels[cd].data, sizeof(int)) < 0) {
    release(&channels[cd].lock);
    return -1;
  }
  channels[cd].isFree = 0;
  wakeup(&channels[cd]);
  release(&channels[cd].lock);
  return 0;
}

uint64 sys_channel_destroy(void)
{
  int cd;
  argint(0, &cd);

  if (cd < 0 || cd >= 10)
    return -1;

  acquire(&channels[cd].lock);
  channels[cd].creator_pid = -2;
  channels[cd].isFree = 0;
  wakeup(&channels[cd]);
  release(&channels[cd].lock);
  // printf("kernel: destroyed!\n");
  return 0;
}