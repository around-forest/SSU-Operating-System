#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int create_page_table_entry(struct proc *p, uint va, uint n);

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

// ssualloc
int
sys_ssualloc(int)
{
	int n;
	uint addr;

	// 매개변수 조건 체크
	if(argint(0, &n) < 0 || n % 4096 != 0 || n <= 0)
		return -1;

	// 새로운 메모리 영역 할당
	addr = myproc()->sz;

	if(myproc()->sz + n > KERNBASE)
		return -1;

	// 페이지 테이블 항목 생성
	if(create_page_table_entry(myproc(), myproc()->sz, n) < 0){
		return -1;
	}

	// 주소공간 업데이트
	myproc()->sz += n;

	return addr;
}

// 페이지 테이블 엔트리 생성 함수
int
create_page_table_entry(struct proc *p, uint va, uint n)
{
	pte_t *pte;
	uint a;

	for(a = va; a < va + n; a += 4096){
		pte = walkpgdir(p->pgdir, (void *) a, 1);
		if(!pte)
			return -1;
		
		// 실제 물리 메모리 주소 설정 X, 권한 설정
		*pte = PTE_U | PTE_W;
	}

	return 0;
}

// getvp
int
sys_getvp(void)
{
	return myproc()->sz / 4096;
}

// getpp
int
sys_getpp(void)
{
	int count = 0;
	pde_t *pgdir = myproc()->pgdir;

	for(int i = 0; i< NPDENTRIES; i++){
		if(pgdir[i] & PTE_P){
			pte_t *pgtab = (pte_t*)P2V(PTE_ADDR(pgdir[i]));
			for(int j = 0; j < NPTENTRIES; j++){
				if(pgtab[j] & PTE_P){
					count++;
				}
			}
			break;
		}
	}

	return count;
}
