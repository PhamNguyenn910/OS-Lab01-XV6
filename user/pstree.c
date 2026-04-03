#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/ptree.h"

static int
has_parent_in_buf(struct ptreeinfo *buf, int count, int ppid)
{
  for(int i = 0; i < count; i++){
    if(buf[i].pid == ppid)
      return 1;
  }
  return 0;
}

static void
print_children(struct ptreeinfo *buf, int count, int parent_pid, int indent)
{
  for(int i = 0; i < count; i++){
    if(buf[i].ppid == parent_pid){
      for(int j = 0; j < indent; j++)
        printf(" ");

      printf("%d %s state=%d mem=%d\n",
             buf[i].pid, buf[i].name, buf[i].state, (int)buf[i].memsize);

      print_children(buf, count, buf[i].pid, indent + 1);
    }
  }
}

int
main(void)
{
  struct ptreeinfo buf[64];
  int count = ptree(buf, 64);

  if(count < 0){
    printf("pstree: ptree failed\n");
    exit(1);
  }

  for(int i = 0; i < count; i++){
    if(buf[i].ppid == 0 || !has_parent_in_buf(buf, count, buf[i].ppid)){
      printf("%d %s state=%d mem=%d\n",
             buf[i].pid, buf[i].name, buf[i].state, (int)buf[i].memsize);
      print_children(buf, count, buf[i].pid, 1);
    }
  }

  exit(0);
}