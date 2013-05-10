
#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "base_socket.h"


class CTcpClient : public CBaseSocket
{
public:
	//client side
	CTcpClient(const char *host, unsigned short port, int timeout = -1)
	{
		if( INVALID_SOCKET != CreateTcp())
		{
			if(0 != Connect(host, port, timeout))
			{
				Close();
			}
		}

	}

	//server side
	CTcpClient(int fd) 
	{
		Attach(fd, SOCK_STREAM);
	}


};



#endif


