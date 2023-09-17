// Pingpong.

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char **argv)
{
  //创建管道会得到一个长度为2的int数组
  //其中0为用于从管道读取数据的文件描述符，1为用于向管道写入数据的文件描述符
  int pp2c[2],pc2p[2];
  pipe(pp2c);//创建用于 父进程->子进程的管道
  pipe(pc2p);//创建用于 子进程->父进程的管道

  if(fork()!=0) {//父进程
    write(pp2c[1],"a",1);//1. 父进程首先向 父->子 管道发出该字节
    char buf;
    read(pc2p[0],&buf,1);//2. 父进程发送完成后，开始等待子进程的回复
    printf("%d: pong\n",getpid());//5. 子进程收到数据，read返回，输出pong
    wait(0);
  }else {//子进程
    char buf;
    read(pp2c[0],&buf,1);//3. 子进程读取管道，收到父进程发送的字节数据
    printf("%d: ping\n",getpid());
    write(pc2p[1],&buf,1);//4. 子进程通过 子->父 管道将字节送回父进程
  }
  exit(0);
}