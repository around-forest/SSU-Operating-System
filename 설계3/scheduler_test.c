#include "types.h"
#include "stat.h"
#include "user.h"

#define PNUM 3 // 생성할 프로세스의 수

void child_process(int priority, int cpu_limit) {
  set_sche_info(priority, cpu_limit);  // 우선순위 및 CPU 사용 제한 설정

  int proc_tick = 0;
  while(proc_tick < cpu_limit) {
    proc_tick++;
    if(proc_tick % 30 == 0) {  // 매 30 tick마다 정보 출력
      printf(1, "PID: %d, Priority: %d, proc_tick: %d, CPU Used: %d\n", getpid(), priority, proc_tick, cpu_limit);
    }
  }

  // 프로세스 종료 정보 출력
  printf(1, "PID: %d terminated\n", getpid());
  exit();
}

void schedule_test(void) {
  int i, pid;

  printf(1, "Starting scheduler test...\n");

  for(i = 0; i < PNUM; i++) {
    pid = fork();
    if(pid < 0) {
      printf(1, "Fork failed\n");
      continue;
    }

    if(pid == 0) {
      // 자식 프로세스의 작업을 실행
      child_process(i * 10, 60); // 우선순위 및 CPU 사용 제한 설정
    }
  }

  for(i = 0; i < PNUM; i++) {
    wait();
  }

  printf(1, "Scheduler test completed.\n");
}

int main(void) {
  schedule_test();
  exit();
}

