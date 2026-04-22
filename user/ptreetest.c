#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/ptree.h"
#include "user/user.h"

static void
test_null_buf(void)
{
  int n = ptree((struct ptreeinfo *)0, 4);
  if(n != -1){
    printf("test_null_buf: FAIL (expected -1, got %d)\n", n);
    exit(1);
  }
  printf("test_null_buf: OK\n");
}

static void
test_bad_max(void)
{
  struct ptreeinfo buf[4];
  int n;

  n = ptree(buf, 0);
  if(n != -1){
    printf("test_bad_max(0): FAIL (expected -1, got %d)\n", n);
    exit(1);
  }

  n = ptree(buf, -1);
  if(n != -1){
    printf("test_bad_max(-1): FAIL (expected -1, got %d)\n", n);
    exit(1);
  }

  printf("test_bad_max: OK\n");
}

static void
test_small_max(void)
{
  struct ptreeinfo buf[2];
  int n = ptree(buf, 2);

  if(n < 0){
    printf("test_small_max: FAIL (ptree returned %d)\n", n);
    exit(1);
  }

  printf("test_small_max: returned=%d (max=2)\n", n);
  for(int i = 0; i < n; i++){
    printf("  [%d] pid=%d ppid=%d state=%d mem=%d name=%s\n",
           i, buf[i].pid, buf[i].ppid, buf[i].state,
           (int)buf[i].memsize, buf[i].name);
  }

  printf("test_small_max: OK\n");
}

static void
test_normal(void)
{
  struct ptreeinfo buf[64];
  int n = ptree(buf, 64);

  if(n <= 0){
    printf("test_normal: FAIL (expected > 0, got %d)\n", n);
    exit(1);
  }

  printf("test_normal: returned=%d\n", n);
  printf("test_normal: OK\n");
}

static void
test_parent_child_relation(void)
{
  struct ptreeinfo buf[64];
  int child = fork();

  if(child < 0){
    printf("test_parent_child_relation: FAIL (fork failed)\n");
    exit(1);
  }

  if(child == 0){
    sleep(50);
    exit(0);
  }

  sleep(5);

  int n = ptree(buf, 64);
  if(n < 0){
    printf("test_parent_child_relation: FAIL (ptree failed)\n");
    kill(child);
    wait(0);
    exit(1);
  }

  int found = 0;
  int me = getpid();

  for(int i = 0; i < n; i++){
    if(buf[i].pid == child && buf[i].ppid == me){
      found = 1;
      break;
    }
  }

  kill(child);
  wait(0);

  if(!found){
    printf("test_parent_child_relation: FAIL (child relation not found)\n");
    exit(1);
  }

  printf("test_parent_child_relation: OK\n");
}

int
main(void)
{
  printf("ptreetest: start\n");

  test_null_buf();
  test_bad_max();
  test_small_max();
  test_normal();
  test_parent_child_relation();

  printf("ptreetest: ALL OK\n");
  exit(0);
}