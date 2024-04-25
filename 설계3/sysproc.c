#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
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

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// date system call
int
sys_date(void)
{
	struct rtcdate *d; // new structure in "date.h"
	if(argptr(0, (void *)&d, sizeof(d)) < 0) // get pointer argument
		return -1; // and error checking

	cmostime(d); // get date and time info

	return 0;
}

// alarm system call
int
sys_alarm(void)
{
	int ticks; // a variable to get argument 'seconds' from alarm_test.c
	if(argint(0, &ticks) < 0) // get int type argument
		return -1; // error checking

	myproc()->alarmticks = ticks; // set alarmticks in proc structure as ticks
	myproc()->alarm_timer = 0; // set alarm_timer in proc struct as 0

	return 0;
}

// set_sche_info
int
sys_set_sche_info(void) {
    int priority, tick;

    if(argint(0, &priority) < 0 || argint(1, &tick) < 0)
	return -1;

    struct proc *p = myproc();
    p->priority = priority;
    p->cpu_used = tick;

    return 0;
}
