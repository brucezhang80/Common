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

static CTFrm * g_TFrm = NULL; /* 系统对象 */

CTFrm::CTFrm(const char * name, const char * conf, unsigned char islock /* = 1 */) : CLThread("tfrm"), m_islock(islock)
{
    m_fd = -1;

    memset(m_name, 0, sizeof( m_name));
    strncpy(m_name, name, sizeof(m_name)-1);

    memset(m_fname, 0, sizeof( m_fname));
    strncpy(m_fname, conf, sizeof(m_fname)-1);

    m_daemon = 1; /* 缺省为后台运行 */
}

CTFrm::~CTFrm()
{
}

/*
 * Function: DaemonInit
 * Desc: 设置为后台进程
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
    signal(SIGUSR1, CTFrm::HandleUSR1); /* USR1用于重新回载配置文件 */
    signal(SIGUSR2, CTFrm::HandleUSR2); /* USR2用于系统退出 */
	if ( (pid = fork()) != 0)
		exit(0);			/* 1st child terminates */

	/* 42nd child continues */

	//chdir("/");				/* change working directory */

	umask(0);				/* clear our file mode creation mask */
}

/*
 * Function: Usage
 * Desc: 用法说明
 * In: 
 *     const char *   name   进程名
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
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
 * Desc: 解析后续命令
 * In: 
 *      int    argc    参数数目
 *      char * argv[]  命令参数
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::ParseCmd(int argc, char * argv[])
{
	int i = 2; /* 跳过0/1参数 */

	while (i < argc)
	{
		if (strcmp(argv[i], "-m") == 0)
		{
			/* 启动模式 */
			if (strcmp(argv[i+1], "nodaemon") == 0)
				m_daemon = 0; /* 前台启动 */
			else
				m_daemon = 1; /* 后台启动 */
		}

		i += 2;
	}

	return 0;
}

/*
 * Function: DispatchCmd
 * Desc: 进程命令
 * In: 
 *      int    argc    参数数目
 *      char * argv[]  命令参数
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::DispatchCmd(int argc, char * argv[])
{
    if (argc < 2)
    {
        Usage(argv[0]);
        return -1;
    }

    /* 启动进程 */
    if (strcasecmp(argv[1], "help") == 0)
	{
        Usage(argv[0]);
	}
    else if (strcasecmp(argv[1], "start") == 0)
    {
		/* 确认后续命令 */
		ParseCmd(argc, argv);
        return 0;
    }
    else if (strcasecmp(argv[1], "stop") == 0)
    {
        /* 停止进程 */
        CmdStop(m_name);
    }
    else if (strcasecmp(argv[1], "restart") == 0)
    {
        /* 重启进程 */
        /* 停止进程 */
        CmdStop(m_name);

        /* 确定进程是否已退出 */
        IsExit(m_name);

		/* 确认后续命令 */
		ParseCmd(argc, argv);

        return 0;
    }
    else if (strcasecmp(argv[1], "reload") == 0)
    {
        /* 重新加载配置 */
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
 * Desc: 进程是否已退出
 * In: 
 *     const char *   fname   进程名
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::IsExit(const char * fname)
{
    /* 获取进程号 */
    char pid_file[1024];
        
    sprintf(pid_file, "%s.pid", fname);
        
    int pid_file_fd = open(pid_file, O_RDONLY , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(pid_file_fd < 0)
    {
        printf("Open pid file %s fail !\n", pid_file);
        return -1;
    }

    /* 确认文件锁是否已释放 */
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
 * Desc: 进程命令
 * In: 
 *     const char *   fname   进程名
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::CmdStop(const char * fname)
{
    /* 获取进程号 */
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
        /* 进程存在 */
        memset(pid_buf, 0, sizeof(pid_buf));
        if (read(pid_file_fd, pid_buf, sizeof(pid_buf)-1) <= 0)
        {
            printf("read pid file %s fail !\n", pid_file);
            return -1;
        }

        /* 发送USR2信号 */
        kill(atoi(pid_buf), SIGUSR2);
    }

    close(pid_file_fd);

    return 0;
}

/*
 * Function: CmdReload
 * Desc:重新加载配置
 * In: 
 *     const char *   fname   进程名
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::CmdReload(const char * fname)
{
    /* 获取进程号 */
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
        /* 进程存在 */
        memset(pid_buf, 0, sizeof(pid_buf));
        if (read(pid_file_fd, pid_buf, sizeof(pid_buf)-1) <= 0)
        {
            printf("read pid file %s fail !\n", pid_file);
            return -1;
        }

        /* 发送USR2信号 */
        kill(atoi(pid_buf), SIGUSR1);
    }

    close(pid_file_fd);

    return 0;

}
    
/*
 * Function: LockServ
 * Desc: 检查进程是否已启动
 * In: 
 *     const char *   fname   进程名
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
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
 * Desc: 检查进程是否已启动
 * In: 
 *     none
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::UnLockServ()
{
    if (m_fd == -1)
    {
        return -1;
    }

    /* 释放文件锁 */
    flock(m_fd, LOCK_UN);

    /* 关闭文件资源 */
    close(m_fd);

    m_fd = -1;

    return 0;
}

/*
 * Function: HandleUSR1
 * Desc: SIGUSR1信号处理函数
 * In: 
 *     int    sig    信号
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

    /* 停止服务 */
    g_TFrm->ReloadConf(g_TFrm->m_fname);
}

/*
 * Function: HandleUSR2
 * Desc: SIGUSR2信号处理函数
 * In: 
 *     int    sig    信号
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

    /* 停止服务 */
    g_TFrm->Stop();
}

/*
 * Function: SetProcName
 * Desc: 设置进程名字
 * In: 
 *      int    argc    参数数目
 *      char * argv[]  命令参数
 * Out: 
 *     none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::SetProcName(int argc, char * argv[])
{
	printf("SetProcName...\n");

	return 0;
}

/*
 * Function: InitConf
 * Desc: 初始化配置参数
 * In: 
 *      const char *   conf  配置文件名
 * Out: 
 *      none
 * Return code:
 *      -1         出错
 *       0         成功
 */
int CTFrm::InitConf(const char * conf)
{
    printf("InitConf...\n");

    return 0;
}

/*
 * Function: ReloadConf
 * Desc: 重新加载配置文件
 * In: 
 *     const char *   conf  配置文件名
 * Out: 
 *     none
 * Return code:
 *     -1                   出错
 *      0                   成功
 */
int CTFrm::ReloadConf(const char *conf)
{
    printf("ReloadConf...\n");

    return 0;
}

/*
 * Function: InitFrame
 * Desc: 系统初始化
 * In: 
 *      none
 * Out: 
 *      none
 * Return code:
 *      -1      出错
 *       0      成功
 */
int CTFrm::InitFrame()
{
    printf("InitFrame...\n");

    return 0;
}

/*
 * Function: ReleaseFrame
 * Desc: 系统资源销毁
 * In: 
 *      none
 * Out: 
 *      none
 * Return code:
 *      -1      出错
 *       0      成功
 */
int CTFrm::ReleaseFrame()
{
    printf("ReleaseFrame...\n");

    return 0;
}

/*
 * Function: HandleThread
 * Desc: 线程业务处理
 * In: 
 *      int    ibreak   中断服务标志
 * Out: 
 *      int    ibreak   中断服务标志
 * Return code:
 *      -1      出错
 *       0      成功
 */
int CTFrm::HandleThread(int& ibreak)
{
    printf("HandleThread:ibreak: %d...\n", ibreak);

    return 0;
}

/*
 * Function: Start
 * Desc: 启动框架
 * In: 
 *      int    argc    参数数目
 *      char * argv[]  命令参数
 * Out: 
 *      none
 * Return code:
 *      -1   出错
 *       0   成功
 */
int CTFrm::Start(int argc, char *argv[])
{
    /* 判断命令参数 */
    if (DispatchCmd(argc, argv))
    {
        return -1;
    }

    /* 设置为后台进程 */
    if(m_daemon)
    {
        DaemonInit();
    }
	else
	{
		/* 设置处理信号 */
		signal(SIGUSR1, CTFrm::HandleUSR1); /* USR1用于重新回载配置文件 */
		signal(SIGUSR2, CTFrm::HandleUSR2); /* USR2用于系统退出 */
	}
    
    /* 进查进程是否存在 */
    if (m_islock)
    {
        if (LockServ(m_name))
        {
            return -1;
        }
    }

    /* 初始化配置参数 */
    if (InitConf(m_fname))
    {
        printf("Config Init Failed, exit...\n");
        return -1;
    }

    /* 系统初始化 */
    if (InitFrame())
    {
        printf("Frame Init Failed, exit...\n");
        return -1;
    }

    /* 启动服务 */
    if (CLThread::Start())
    {
        printf("Start Failed, exit...\n");
        return -1;
    }

    /* 设置进程实例 */
    g_TFrm = this;

	/* 设置进程名 */
	SetProcName(argc, argv);

    printf("%s is running...\n", m_name);

    /* 等待退出 */
    m_event.Wait();

    /* 停止服务 */
    CLThread::Stop();

    /* 释放系统资源 */
    ReleaseFrame();

    /* 释放文件资源 */
    if (m_islock)
    {
        UnLockServ();
    }

    printf("%s is stopped...\n", m_name);

    return 0;
}

/*
 * Function: Stop
 * Desc: 退出框架
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      -1   出错
 *       0   成功
 */
int CTFrm::Stop()
{
    /* 发送退出标志 */
    m_event.SetEvent();

    return 0;
}
