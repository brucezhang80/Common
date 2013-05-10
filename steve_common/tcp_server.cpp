#include "tcp_server.h"

CSockAcceptor::CSockAcceptor()
{
}

CSockAcceptor::CSockAcceptor(const char *ip, unsigned short port, int backlog /* = 5 */)
{
    Init(ip, port, backlog);
}

CSockAcceptor::~CSockAcceptor()
{
}

int CSockAcceptor::Init(const char *ip, unsigned short port, int backlog /* = 5 */)
{
	CreateTcp();
    if(IsValid())
    {
        SetReuseAddr();
        if( 0 != Bind(ip, port))
        {
            Close();
            return -1;
        }

        if( 0 != Listen(backlog))
        {
            Close();
            return -1;
        }

        return 0;
    }

    return -1;
}

int CSockAcceptor::Accept(struct sockaddr &addr, socklen_t &len, int timeout /* = -1 */)
{
    if(timeout < 0)
	{
        return accept(m_iSocket, &addr, &len);
    }

	if(0 == WaitRead(timeout))
	{
    	return accept(m_iSocket, &addr, &len);
	}

    return -2;
}

int CSockAcceptor::Accept(int timeout /* = -1 */)
{
	sockaddr  addr  = { 0 };
	socklen_t  nAddrSize = sizeof(addr);

    return Accept(addr, nAddrSize, timeout);
}

int CSockAcceptor::Listen(int backlog)
{
	return listen(m_iSocket, backlog);
}
