#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "date.h"

#define TIME_QUANTUM 30
#define PRIORITY_ADJUST_TIME 60

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

struct rtcdate *r;

extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;

// tick 함수
void
tick(void) {
    struct proc *p = myproc();

    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
	if(p->state == RUNNING && p->pid != 1 && p->pid != 2) {
	    p->proc_tick++;
	    p->priority_tick++;

	    if(p->proc_tick >= TIME_QUANTUM) {
		// Time quantum 종료. 프로세스 상태 변경
		p->state = RUNNABLE;
		p->proc_tick = 0;
		insert_proc(p); // 변경된 우선순위에 따라 runqueue에 다시 삽입
	    }

	    if(p->priority_tick >= PRIORITY_ADJUST_TIME) {
		adjust_priority(p); // 우선순위 조정
	    }

	    if(p->proc_tick >= p->cpu_used) {
		// 프로세스 종료
		remove_proc(p);
		p->state = ZOMBIE;
		wakeup1(p->parent);
	    }
	}
    }
    release(&ptable.lock);
}

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER: // when timer interrupt occered
    if(cpuid() == 0){
	tick(); // 타이머 인터럽트 발생 시 tick 함수 호출
    }
		lapiceoi();
		// alarm
		if(myproc() != 0 && myproc()->alarmticks != 0) // myproc exists and alarmticks is not 0
			myproc()->alarm_timer++; // increase alarm_timer 1
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }


  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

	// alarm : when alarm_timer is greater than alarmticks * 100
	if(myproc() != 0 && (tf->cs & 3) == 3 && tf->trapno == T_IRQ0 + IRQ_TIMER && myproc()->alarm_timer > (myproc()->alarmticks * 100)){
		cmostime(r); // get current time info
		myproc()->alarm_timer = 0; // reset the alarm_timer 0
		cprintf("SSU_Alarm!\n"); // print "SSU_Alarm!"
		cprintf("Current time : %d-%d-%d %d:%d:%d\n", r->year, r->month, r->day, r->hour, r->minute, r->second); // print current time info
		exit();
	}
}
