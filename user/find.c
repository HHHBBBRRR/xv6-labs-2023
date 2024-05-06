#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/fs.h"
 
void find(char *path, char *target)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;
 
	if ((fd = open(path, O_RDONLY)) < 0)
	{
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}
 
	if (fstat(fd, &st) < 0)
	{
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}
 
	if (st.type == T_FILE || st.type == T_DEVICE)
	{
		for (p = path + strlen(path); p >= path && *p != '/'; p--)
			;
		p++;
 
		if (strcmp(p, target) == 0)
		{
			printf("%s\n", path);
		}
 
		close(fd);
		return;
	}
 
	strcpy(buf, path);
	p = buf + strlen(buf);
	*p++ = '/';
 
	while (read(fd, &de, sizeof(de)) == sizeof(de))
	{
		if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
		{
			continue;
		}
 
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
 
		find(buf, target);
	}
 
	close(fd);
}
 
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(2, "find: missing operand\n");
		exit(1);
	}
	else if (argc == 2)
	{
		find(".", argv[1]);
		exit(0);
	}
 
	find(argv[1], argv[2]);
 
	exit(0);
}