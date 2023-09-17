// Sleep.

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char **argv)
{
  if(argc < 2) {
    printf("need one argument\n");
    exit(1);
  } 
  //获取第二个参数
  int ticks=atoi(argv[1]);
  //调用系统函数sleep
  sleep(ticks);
  printf("(nothing happens for a little while)\n");
  exit(0);
}