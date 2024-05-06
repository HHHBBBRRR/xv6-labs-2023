#include "kernel/types.h"
#include "user/user.h"
 
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(2, "sleep: missing operand\n");
		exit(1);
	}
 
    int ret = sleep(atoi(argv[1]));
 
    if (ret == -1)
    {
        exit(1);
    }
    
	exit(0);
}