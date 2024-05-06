#include "kernel/types.h"
#include "user/user.h"
 
void prime(int fd)
{   
    int num;
 
    if (read(fd, &num, 4) == 0)
    {
        close(fd);
        return;
    }
 
    printf("prime %d\n", num);
 
    int p[2];
 
    pipe(p);
 
    int pid = fork();
 
    if (pid > 0)
    {  
        close(p[0]);
        int n;
        while ((read(fd, &n, 4)) != 0)
        {
            if (n % num != 0)
            {
                write(p[1], &n, 4);
            }     
        }
        close(p[1]);
        wait(0);     
    }
    else
    {
        close(p[1]);
        prime(p[0]);
    }
}
 
int main(void)
{
    int p[2];
 
    pipe(p);
 
    int pid = fork();
 
    if (pid > 0)
    {
        close(p[0]);
        for (int i = 2; i <= 35; i++)
        {
            write(p[1], &i, 4);
        }
        close(p[1]);
    }
    else
    {
        close(p[1]);
        prime(p[0]);
    }
 
    exit(0);
}