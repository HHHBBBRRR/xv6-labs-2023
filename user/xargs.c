#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/param.h"
 
#define BUFFR_SIZE 512
 
int main(int argc, char *argv[])
{
    int n;
    char buffer[BUFFR_SIZE];
    char *exec_argv[MAXARG];
 
    if (argc < 2)
    {
        exec_argv[0] = "echo";
    }
    else
    {
        exec_argv[0] = argv[1];
    }
 
    for (int i = 2; i < argc; i++)
    {
        exec_argv[i - 1] = argv[i];
    }
 
    while ((n = read(0, buffer, sizeof(buffer))) > 0)
    {
        char args[512] = {'\0'};
        char *end;
        int length;
 
        while (n > 0)
        {           
            end = strchr(buffer, '\n');
            length = end - buffer;
            memmove(args, buffer, end - buffer);
 
            int pid = fork();
 
            if (pid < 0)
            {
                fprintf(2, "Error: fork failed\n");
                exit(1);
            }
            else if (pid == 0)
            {
                exec_argv[argc - 1] = args;
                exec(exec_argv[0], exec_argv);
            }
 
            wait(0);
            memmove(buffer, buffer + length + 1, length + 1);
            n -= (length + 1);
        }
    }
 
    if (n < 0)
    {
        fprintf(2, "xargs: read error\n");
        exit(1);
    }
 
    exit(0);
}