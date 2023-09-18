// Primes.

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#define PIPE_W 1
#define PIPE_R 0

void process(int *p)
{
    // 子进程
    close(p[PIPE_W]); // 子进程只需要用到管道读端
    int num;
    if (read(p[PIPE_R], &num, sizeof(num)) == 0) //管道没有数据可读了
    {
        close(p[PIPE_R]);
        exit(0); //该进程任务结束
    }
    printf("prime %d\n",num); //将管道中第一个读到的数打印

    int cp[2]; //需要一个新的管道给该进程的子进程
    pipe(cp);

    if(fork()==0) //当前进程的子进程
    {
        process(cp);
    }
    else //当前进程
    {
        close(cp[PIPE_R]); //关闭不用的fd
        int nnum;
        while(read(p[PIPE_R],&nnum,sizeof(nnum))) //把旧管道中的数都读出来，进行筛选
        {
            if(nnum%num!=0) //如果读出来的数不是num(质数)的倍数
            {
                write(cp[PIPE_W],&nnum,sizeof(nnum)); //将这些有可能是质数的数写入新管道
            }
        }
        close(cp[PIPE_W]); //关闭无用fd
        wait(0);
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    if (fork() == 0) // 子进程
    {
        process(p);
    }
    else // 父进程
    {
        close(p[PIPE_R]); // 关闭管道读端，防止超过xv6的系统fd上限

        for (int i = 2; i <= 35; i++)
        {
            write(p[PIPE_W], &i, sizeof(i)); //  将2到35全部写进管道
        }
        close(p[PIPE_W]);
        wait(0);
    }

    exit(0);
}