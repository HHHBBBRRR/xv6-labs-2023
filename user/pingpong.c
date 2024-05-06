#include "kernel/types.h"
#include "user/user.h"
 
int main(void)
{
    int fd1[2];
    int fd2[2];
    int pid;
    char buf[1] = {'a'};
 
    pipe(fd1);
    pipe(fd2);
 
    pid = fork();
 
    if (pid == 0)
    {
        close(fd1[0]);
        close(fd2[1]);
        write(fd1[1],buf,sizeof(buf));
        printf("%d: received ping\n",getpid());
        read(fd2[0],buf,sizeof(buf));
    }
    else
    {
        close(fd1[1]);
        close(fd2[0]);
        read(fd1[0],buf,sizeof(buf));
        printf("%d: received pong\n",getpid());
        write(fd2[1],buf,sizeof(buf));
    }
    
	exit(0);
}