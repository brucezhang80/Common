/*
 * @File: tfrm.cpp
 * @Desc: implement file of TFrame
 * @Author: stevetang
 * @History:
 *  2007-11-23   Create
 * @Copyright: Shenzhen Tecent Co.Ltd.
 */


#include "general.h"
#include "tfrm.h"

static CTFrm * g_TFrm = NULL; /* ϵͳ���� */

CTFrm::CTFrm(const char * name, const char * conf, unsigned char islock /* = 1 */) : CLThread("tfrm"), m_islock(islock)
{
    m_fd = -1;

    memset(m_name, 0, sizeof( m_name));
    strncpy(m_name, name, sizeof(m_name)-1);

    memset(m_fname, 0, sizeof( m_fname));
    strncpy(m_fname, conf, sizeof(m_fname)-1);

    m_daemon = 1; /* ȱʡΪ��̨���� */
}

CTFrm::~CTFrm()
{
}

/*
 * Function: DaemonInit
 * Desc: ����Ϊ��̨����
 * In: 
 *      none
 * Out: 
 *      none
 * Return code:
 *      none
 */
void CTFrm::DaemonInit()
{
	pid_t	pid;

	if ( (pid = fork()) != 0)
		exit(0);			/* parent terminates */

	/* 41st child continues */
	setsid();				/* become session leader */
	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	/* signal(SIGCHLD, SIG_IGN); */
	signal(SIGTERM, SIG_IGN);
    signal(SIGXFSZ, SIG_IGN);
    signal(SIGUSR1, CTFrm::HandleUSR1); /* USR1�������»��������ļ� */
    signal(SIGUSR2, CTFrm::HandleUSR2); /* USR2����ϵͳ�˳� */
	if ( (pid = fork()) != 0)
		exit(0);			/* 1st child terminates */

	/* 42nd child continues */

	//chdir("/");				/* change working directory */

	umask(0);				/* clear our file mode creation mask */
}

/*
 * Function: Usage
 * Desc: �÷�˵��
 * In: 
 *     const char *   name   ������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::Usage(const char * name)
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
    printf("\n");

    return 0;
}

/*
 * Function: ParseCmd
 * Desc: ������������
 * In: 
 *      int    argc    ������Ŀ
 *      char * argv[]  �������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::ParseCmd(int argc, char * argv[])
{
	int i = 2; /* ����0/1���� */

	while (i < argc)
	{
		if (strcmp(argv[i], "-m") == 0)
		{
			/* ����ģʽ */
			if (strcmp(argv[i+1], "nodaemon") == 0)
				m_daemon = 0; /* ǰ̨���� */
			else
				m_daemon = 1; /* ��̨���� */
		}

		i += 2;
	}

	return 0;
}

/*
 * Function: DispatchCmd
 * Desc: ��������
 * In: 
 *      int    argc    ������Ŀ
 *      char * argv[]  �������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::DispatchCmd(int argc, char * argv[])
{
    if (argc < 2)
    {
        Usage(argv[0]);
        return -1;
    }

    /* �������� */
    if (strcasecmp(argv[1], "help") == 0)
	{
        Usage(argv[0]);
	}
    else if (strcasecmp(argv[1], "start") == 0)
    {
		/* ȷ�Ϻ������� */
		ParseCmd(argc, argv);
        return 0;
    }
    else if (strcasecmp(argv[1], "stop") == 0)
    {
        /* ֹͣ���� */
        CmdStop(m_name);
    }
    else if (strcasecmp(argv[1], "restart") == 0)
    {
        /* �������� */
        /* ֹͣ���� */
        CmdStop(m_name);

        /* ȷ�������Ƿ����˳� */
        IsExit(m_name);

		/* ȷ�Ϻ������� */
		ParseCmd(argc, argv);

        return 0;
    }
    else if (strcasecmp(argv[1], "reload") == 0)
    {
        /* ���¼������� */
        CmdReload(m_name);
    }
    else
    {
        Usage(argv[0]);
    }

    return -1;
}

/*
 * Function: IsExit
 * Desc: �����Ƿ����˳�
 * In: 
 *     const char *   fname   ������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::IsExit(const char * fname)
{
    /* ��ȡ���̺� */
    char pid_file[1024];
        
    sprintf(pid_file, "%s.pid", fname);
        
    int pid_file_fd = open(pid_file, O_RDONLY , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(pid_file_fd < 0)
    {
        printf("Open pid file %s fail !\n", pid_file);
        return -1;
    }

    /* ȷ���ļ����Ƿ����ͷ� */
    while( flock(pid_file_fd, LOCK_EX | LOCK_NB) < 0 )
    {
        sleep(1);
    }

    flock(pid_file_fd, LOCK_UN);

    close(pid_file_fd);

    return 0;
}

/*
 * Function: CmdStop
 * Desc: ��������
 * In: 
 *     const char *   fname   ������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::CmdStop(const char * fname)
{
    /* ��ȡ���̺� */
    char pid_file[1024];
    char pid_buf[16];
        
    sprintf(pid_file, "%s.pid", fname);
        
    int pid_file_fd = open(pid_file, O_RDONLY , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(pid_file_fd < 0)
    {
        printf("Open pid file %s fail !\n", pid_file);
        return -1;
    }

    if( flock(pid_file_fd, LOCK_EX | LOCK_NB) < 0 )
    {
        /* ���̴��� */
        memset(pid_buf, 0, sizeof(pid_buf));
        if (read(pid_file_fd, pid_buf, sizeof(pid_buf)-1) <= 0)
        {
            printf("read pid file %s fail !\n", pid_file);
            return -1;
        }

        /* ����USR2�ź� */
        kill(atoi(pid_buf), SIGUSR2);
    }

    close(pid_file_fd);

    return 0;
}

/*
 * Function: CmdReload
 * Desc:���¼�������
 * In: 
 *     const char *   fname   ������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::CmdReload(const char * fname)
{
    /* ��ȡ���̺� */
    char pid_file[1024];
    char pid_buf[16];
        
    sprintf(pid_file, "%s.pid", fname);
        
    int pid_file_fd = open(pid_file, O_RDONLY , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(pid_file_fd < 0)
    {
        printf("Open pid file %s fail !\n", pid_file);
        return -1;
    }

    if( flock(pid_file_fd, LOCK_EX | LOCK_NB) < 0 )
    {
        /* ���̴��� */
        memset(pid_buf, 0, sizeof(pid_buf));
        if (read(pid_file_fd, pid_buf, sizeof(pid_buf)-1) <= 0)
        {
            printf("read pid file %s fail !\n", pid_file);
            return -1;
        }

        /* ����USR2�ź� */
        kill(atoi(pid_buf), SIGUSR1);
    }

    close(pid_file_fd);

    return 0;

}
    
/*
 * Function: LockServ
 * Desc: �������Ƿ�������
 * In: 
 *     const char *   fname   ������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::LockServ(const char * fname)
{
	char pid_file[1024];
	char pid_buf[16];
	
	sprintf(pid_file, "%s.pid", fname);
	
	int pid_file_fd = open(pid_file, O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(pid_file_fd < 0)
	{
		printf("Open pid file %s fail !\n", pid_file);
		return -1;
	}

	if( flock(pid_file_fd, LOCK_EX | LOCK_NB) < 0 )
	{
		printf("Server is already Running!\n");
		return -1;
	}

	if(ftruncate(pid_file_fd, 0) < 0)
	{
		printf("truncate pid file %s fail !\n", pid_file);
		return -1;
	}

	sprintf(pid_buf, "%u\n", (int)getpid());
	if(write(pid_file_fd, pid_buf, strlen(pid_buf)) != (int)strlen(pid_buf))
	{
		printf("write pid file %s fail !\n", pid_file);
		return -1;
	}

	int val = fcntl(pid_file_fd, F_GETFD, 0);
	if( val < 0)
	{
		printf("fcntl F_GETFD pid file %s fail !\n", pid_file);
		return -1;
	}

	val |= FD_CLOEXEC;

	if(fcntl(pid_file_fd, F_SETFD, val) < 0)
	{
		printf("fcntl F_SETFD pid file %s fail !\n", pid_file);
		return -1;
	}

    m_fd = pid_file_fd;
	
    return 0;
}

/*
 * Function: UnLockServ
 * Desc: �������Ƿ�������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::UnLockServ()
{
    if (m_fd == -1)
    {
        return -1;
    }

    /* �ͷ��ļ��� */
    flock(m_fd, LOCK_UN);

    /* �ر��ļ���Դ */
    close(m_fd);

    m_fd = -1;

    return 0;
}

/*
 * Function: HandleUSR1
 * Desc: SIGUSR1�źŴ�����
 * In: 
 *     int    sig    �ź�
 * Out: 
 *     none
 * Return code:
 *     none
 */
void CTFrm::HandleUSR1(int sig)
{
    if (g_TFrm == NULL)
    {
        return;
    }

    /* ֹͣ���� */
    g_TFrm->ReloadConf(g_TFrm->m_fname);
}

/*
 * Function: HandleUSR2
 * Desc: SIGUSR2�źŴ�����
 * In: 
 *     int    sig    �ź�
 * Out: 
 *     none
 * Return code:
 *     none
 */
void CTFrm::HandleUSR2(int sig)
{
    if (g_TFrm == NULL)
    {
        return;
    }

    /* ֹͣ���� */
    g_TFrm->Stop();
}

/*
 * Function: SetProcName
 * Desc: ���ý�������
 * In: 
 *      int    argc    ������Ŀ
 *      char * argv[]  �������
 * Out: 
 *     none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::SetProcName(int argc, char * argv[])
{
	printf("SetProcName...\n");

	return 0;
}

/*
 * Function: InitConf
 * Desc: ��ʼ�����ò���
 * In: 
 *      const char *   conf  �����ļ���
 * Out: 
 *      none
 * Return code:
 *      -1         ����
 *       0         �ɹ�
 */
int CTFrm::InitConf(const char * conf)
{
    printf("InitConf...\n");

    return 0;
}

/*
 * Function: ReloadConf
 * Desc: ���¼��������ļ�
 * In: 
 *     const char *   conf  �����ļ���
 * Out: 
 *     none
 * Return code:
 *     -1                   ����
 *      0                   �ɹ�
 */
int CTFrm::ReloadConf(const char *conf)
{
    printf("ReloadConf...\n");

    return 0;
}

/*
 * Function: InitFrame
 * Desc: ϵͳ��ʼ��
 * In: 
 *      none
 * Out: 
 *      none
 * Return code:
 *      -1      ����
 *       0      �ɹ�
 */
int CTFrm::InitFrame()
{
    printf("InitFrame...\n");

    return 0;
}

/*
 * Function: ReleaseFrame
 * Desc: ϵͳ��Դ����
 * In: 
 *      none
 * Out: 
 *      none
 * Return code:
 *      -1      ����
 *       0      �ɹ�
 */
int CTFrm::ReleaseFrame()
{
    printf("ReleaseFrame...\n");

    return 0;
}

/*
 * Function: HandleThread
 * Desc: �߳�ҵ����
 * In: 
 *      int    ibreak   �жϷ����־
 * Out: 
 *      int    ibreak   �жϷ����־
 * Return code:
 *      -1      ����
 *       0      �ɹ�
 */
int CTFrm::HandleThread(int& ibreak)
{
    printf("HandleThread:ibreak: %d...\n", ibreak);

    return 0;
}

/*
 * Function: Start
 * Desc: �������
 * In: 
 *      int    argc    ������Ŀ
 *      char * argv[]  �������
 * Out: 
 *      none
 * Return code:
 *      -1   ����
 *       0   �ɹ�
 */
int CTFrm::Start(int argc, char *argv[])
{
    /* �ж�������� */
    if (DispatchCmd(argc, argv))
    {
        return -1;
    }

    /* ����Ϊ��̨���� */
    if(m_daemon)
    {
        DaemonInit();
    }
	else
	{
		/* ���ô����ź� */
		signal(SIGUSR1, CTFrm::HandleUSR1); /* USR1�������»��������ļ� */
		signal(SIGUSR2, CTFrm::HandleUSR2); /* USR2����ϵͳ�˳� */
	}
    
    /* ��������Ƿ���� */
    if (m_islock)
    {
        if (LockServ(m_name))
        {
            return -1;
        }
    }

    /* ��ʼ�����ò��� */
    if (InitConf(m_fname))
    {
        printf("Config Init Failed, exit...\n");
        return -1;
    }

    /* ϵͳ��ʼ�� */
    if (InitFrame())
    {
        printf("Frame Init Failed, exit...\n");
        return -1;
    }

    /* �������� */
    if (CLThread::Start())
    {
        printf("Start Failed, exit...\n");
        return -1;
    }

    /* ���ý���ʵ�� */
    g_TFrm = this;

	/* ���ý����� */
	SetProcName(argc, argv);

    printf("%s is running...\n", m_name);

    /* �ȴ��˳� */
    m_event.Wait();

    /* ֹͣ���� */
    CLThread::Stop();

    /* �ͷ�ϵͳ��Դ */
    ReleaseFrame();

    /* �ͷ��ļ���Դ */
    if (m_islock)
    {
        UnLockServ();
    }

    printf("%s is stopped...\n", m_name);

    return 0;
}

/*
 * Function: Stop
 * Desc: �˳����
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      -1   ����
 *       0   �ɹ�
 */
int CTFrm::Stop()
{
    /* �����˳���־ */
    m_event.SetEvent();

    return 0;
}
