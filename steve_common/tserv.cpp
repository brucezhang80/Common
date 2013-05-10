#include "general.h"
#include "tservfrm.h"

int Usage(const char * name)
{
    printf("\n");
    printf("Usage: %s help|start|stop|restart|reload [options]\n", name);
    printf("\thelp: help.\n");
    printf("\tstart: start process.\n");
    printf("\tstop: stop process.\n");
    printf("\trestart: restart process.\n");
    printf("\treload: reload config file.\n");
	printf("\toptions:\n");
	printf("\t\t-m daemon|nodaemon\n");
	printf("\t\t-n name\n");
	printf("\t\t-c config-file-name\n");
    printf("\n");

    return 0;
}

int GetConfFileName(int argc, char * argv[], char * name)
{
	int i = 2; /* 跳过0/1参数 */

	while (i < argc)
	{
		if (strcmp(argv[i], "-c") == 0)
		{
			strcpy(name, argv[i+1]);
			break;
		}

		i += 2;
	}

	return 0;
}

int GetProcName(int argc, char * argv[], char * name)
{
	int i = 2; /* 跳过0/1参数 */

	while (i < argc)
	{
		if (strcmp(argv[i], "-n") == 0)
		{
			strcpy(name, argv[i+1]);
			break;
		}

		i += 2;
	}

	return 0;
}

int main(int argc, char * argv[])
{
	if (argc < 2 || strcmp(argv[1], "help") == 0)
	{
		Usage(argv[0]);
		return 0;
	}

	char name[512];

	strcpy(name, "tserv");

	GetProcName(argc, argv, name);

	char fname[512];

	strcpy(fname, "../conf/tserv.ini");

	GetConfFileName(argc, argv, fname);

    /* 启动进程 */
	CTServFrm g_tserv(name, fname);

    g_tserv.Start(argc, argv);

	return 0;
}
