


#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "tcp_client.h"
#include "tcp_server.h"


#ifdef _TEST_SOCKET_
void Server(const char *host, unsigned int port)
{
	CLfServer server(host, port, 16);
	//COtpcServer server(host, port);
	if( 0 != server.StartService(false))
	{
		printf("Cannot start server at %s:%u\n", host, port);
	}
}

int HandleInput(const char *buf, unsigned int len)
{
	return *buf;
}

void Client(const char *host, unsigned int port)
{
	CTcpClient client(host, port);
	if(client.IsValid())
	{
		dbg_out("client is ok,connect server ok.\n");
	}
	else
	{
		dbg_out("connect server error.\n");
		return;
	}

	int ret = 0;
	unsigned int req_len = 6;
	char req[32] = {0};
	req[0] = req_len;
	strcpy(req + 1,"12345");
	char rsp[32];
	unsigned int rsp_len = 31;
	while(1)
	{
		ret = client.SendRecvWithRetry(req, req_len, HandleInput, 
			1, rsp, rsp_len);
		if(ret <= 0)
		{
			printf("send recv error,ret = %d\n",ret);
			break;
		}
		rsp[ret] = 0;
		printf("recv packet, len = %d, content = %s\n", rsp[0], rsp + 1);
		sleep(10);
	}
}

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		printf("usage: %s c|s host port\n", argv[0]);
		return 0;
	}
	signal(SIGPIPE, SIG_IGN);
	if(argv[1][0] == 's')
	{
		Server(argv[2], atoi(argv[3]));
	}
	else
	{
		Client(argv[2], atoi(argv[3]));
	}
	return 0;
}

#endif
