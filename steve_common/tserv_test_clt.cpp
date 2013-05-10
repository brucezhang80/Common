#include "general.h"
#include "hash.h"
#include "baselist.h"

/* Э��� */
#pragma pack(1)

typedef struct _tagTServProtoHead {
	unsigned int length; /* �����ȣ���ͷ+���� */
	unsigned int cmd; /* ������ */
	unsigned int sequence; /* ���к� */
} recTServProtoHead;

typedef struct _tagTServProtoPck {
	recTServProtoHead head; /* ����ͷ */
	char data[0]; /* ���� */
} recTServProtoPck;

#pragma pack()

typedef CGenericList<int> intList;

CHash g_hash(1024);
int g_print = 0;
intList g_list(1024);

int Add(int fd , int sock, int flag)
{
	if (fd == -1)
		return -1;

	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events = flag;

	int ret = epoll_ctl(fd, EPOLL_CTL_ADD, sock, &ev);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}

int Mod(int fd, int sock, int flag)
{
	if (fd == -1)
		return -1;

	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events = flag;

	int ret = epoll_ctl(fd, EPOLL_CTL_MOD, sock, &ev);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}

int Remove(int fd, int sock)
{
	if (fd == -1)
		return -1;

	struct epoll_event ev;

	ev.data.fd = sock;
	ev.events = 0;

	int ret = epoll_ctl(fd, EPOLL_CTL_DEL, sock, &ev);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}

int Recv(int sock, char * buf, int len)
{
	int left = len;
	char * ptr = buf;

	/* ���Ϊ������ */
	while (left > 0)
	{
		/* �������� */
		int ret = recv(sock, ptr, left, 0);
		if (ret <= 0)
		{
			if (errno == EAGAIN)
				/* �������� */
				break;

			if (errno == EINTR)
				/* ���ź��жϣ��������� */
				continue;

			return -1;
		}
		
		left -= ret;
		ptr += ret;
	}

	/* ����ʵ�ʶ�ȡ������ */
	return (len - left);
}

int Send(int sock, const char * buf, int len)
{
	/* �������� */
	int left = len;
	char * ptr = (char *)buf;

	while (left > 0)
	{
		int ret = send(sock, ptr, left, 0);
		if (ret <= 0)
		{
			if (errno == EAGAIN)
				/* �ӻ����� */
				break;

			if (errno == EINTR)
				/* ���ź��жϣ����ɷ� */
				continue;

			return -1;
		}

		left -= ret;
		ptr += ret;
	}

	/* ����ʵ�ʷ������ݴ�С */
	return (len - left);
}

int HandleIn(int fd, int sock)
{
	char buf[204800];

	/* ��HASH���в鿴�Ƿ�Ϊδ����ľ�� */
	int * obj = (int *)g_hash.Remove((size_t)sock);
	int left = ((obj == NULL) ? 0 : *obj);
	int ret = 0;
	int first = 0;
	if (left == 0)
	{
		/* �����ڣ����ճ��� */
		unsigned int length = 0;
		ret = Recv(sock, (char *)&length, sizeof(unsigned int));
		if (ret < 0 || ret != sizeof(unsigned int))
		{
			printf("recv head error.[%d: %s]\n", sock, strerror(errno));
			close(sock);
			return -1;
		}

		left = length - sizeof(unsigned int);

		if (g_print)
		{
			printf("length: %d\n", length);
		}

		first = 1;

		obj = g_list.RemoveHead();
	}

	ret = Recv(sock, buf, left);
	if (ret <= 0)
	{
		printf("recv body error.[%d: %s]\n", sock, strerror(errno));
		close(sock);
		return -1;
	}

	buf[ret] = 0x00;

	if (g_print)
	{
		if (first)
			printf("%s", buf+sizeof(recTServProtoHead)-sizeof(unsigned int));
		else
			printf("%s", buf);
	}

	left -= ret;

	*obj = left;

	/* ȷ���Ƿ�������� */
	if (left > 0)
	{
		/* ���ڵȴ��´ν��� */
		g_hash.Add((size_t)sock, obj);
		return 1;
	}

	g_list.AddTail(obj);

	Mod(fd, sock, EPOLLOUT | EPOLLERR | EPOLLHUP);

	return 0;
}

int HandleOut(int fd, int sock)
{
	char buf[1024];

	memset(buf, 0, sizeof(buf));

	recTServProtoPck * req = (recTServProtoPck *)buf;

	req->head.length = sizeof(recTServProtoPck) + strlen("request test.");
	req->head.cmd = 0xFFFFFFFF;
	req->head.sequence = 1;

	strcpy(req->data, "request test.");

	/* �������� */
	int ret = Send(sock, buf, req->head.length);
	if (ret <= 0)
	{
		printf("fail to send request.[%d][%d][%d][%s]\n", ret, sock, errno, strerror(errno));
		return -1;
	}

	Mod(fd, sock, EPOLLIN | EPOLLERR | EPOLLHUP);

	return 0;
}

int HandleErr(int fd, int sock)
{
	printf("error.[%d]\n", sock);
	close(sock);
	return 0;
}

int run(int fd, int total)
{
	struct epoll_event ev[1024]; /* epoll�¼��б� */
	int s = 0, r = 0;

	time_t t1, t2;

	t1 = t2 = time(NULL);

	while (total > r)
	{
		int nfds = epoll_wait(fd, ev, 1024, 100);
		for (int i=0; i<nfds; i++)
		{
			if (ev[i].events & EPOLLIN)
			{
				int ret = HandleIn(fd, ev[i].data.fd);
				if (ret == 0)
				{
					r++;
				}
			}
			else if (ev[i].events & EPOLLOUT)
			{
				if (s >= total)
					continue;

				int ret = HandleOut(fd, ev[i].data.fd);
				if (ret == 0)
				{
					s++;
				}
			}
			else if (ev[i].events & (EPOLLERR | EPOLLHUP))
			{
				HandleErr(fd, ev[i].data.fd);
			}
		}

		if ((time(NULL) - t2) >= 5)
		{
			printf("S: %d R: %d\n", s, r);
			t2 = time(NULL);
		}
	}

	printf("\n[S:%d R:%d]%u %u\n", s, r, (unsigned int)t1, (unsigned int)time(NULL));

	return 0;
}

int CreateClient(const char * ip, int port, int fd)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1)
	{
		printf("fail to create sock.\n");
		return -1;
	}

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	int ret = connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	if (ret == -1)
	{
		printf("fail to connect.\n");
		return -1;
	}

	/* ����Ϊ������ */
	int val = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, val | O_NONBLOCK | O_NDELAY);

	Add(fd, sock, EPOLLOUT | EPOLLERR | EPOLLHUP);

	return sock;
}

void Daemon()
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
	if ( (pid = fork()) != 0)
		exit(0);			/* 1st child terminates */

	/* 42nd child continues */

	//chdir("/");				/* change working directory */

	umask(0);				/* clear our file mode creation mask */
}

int main(int argc, char * argv[])
{
	if (argc != 3 && argc != 5)
	{
		printf("Usage: %s ip port [number total]\n", argv[0]);
		return -1;
	}

	char ip[16];
	strcpy(ip, argv[1]);
	int port = atoi(argv[2]);

	int fd = epoll_create(1024); 
	if (fd == -1)
	{
		printf("fail to create epoll.\n");
		return -1;
	}

	g_print = ((argc == 3) ? 1 : 0);
	int cltnum = ((argc > 3) ? atoi(argv[3]) : 1);
	int * sock = (int *)malloc(cltnum * sizeof(int));
	int * left = (int *)malloc(cltnum * sizeof(int));
	
	for (int i=0; i<cltnum; i++)
	{
		g_list.AddTail(&left[i]);
	}

	for (int i=0; i<cltnum; i++)
	{
		sock[i] = CreateClient(ip, port, fd);
		if (sock[i] == -1)
		{
			printf("fail to create client.\n");
			return -1;
		}
	}

	int total = ((argc > 4) ? atoi(argv[4]) : 1);

	run(fd, total);

	for (int i=0; i<cltnum; i++)
	{
		close(sock[i]);
	}

	free(left);

	free(sock);
	
	close(fd);

	printf("clt ok.\n");

	return 0;
}
