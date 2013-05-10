/* 
 *       FILE:        
 *              base_socket.h
 *       DESCRIPTION:  
 *              socket������װ��TCP�ͻ���ͷ�����
 *       AUTHOR:
 *             jonaszhang   2006-05-16	QQ�ռ����ݿ�����
 *       MODIFIED:
 */
 

#ifndef _SOCKET_INTERFACE_H_
#define _SOCKET_INTERFACE_H_

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <netinet/in.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <string>
using namespace std;
#define INVALID_SOCKET  -1

/*
*���SOCKET�����Ļ���
*/
class CBaseSocket
{
public:
	/*���캯��*/
	CBaseSocket();
	/*��������*/
	virtual ~CBaseSocket(){Close();}	

	typedef int (*pfHandleInput)(const char *, unsigned int);
	
private:

	/*COPY���캯��*/
	CBaseSocket(const CBaseSocket &rhs);
	/*��ֵ����*/
	CBaseSocket &operator=(const CBaseSocket &rhs);	

public:

	/*
	*���ܣ�
			ȡSOCKET������
	*���������
			��
	*���������
			��
	*����ֵ��	
			
	*/
	int GetFD()const{return m_iSocket;}

	static char *ip_ntoa(unsigned int in, char *buf)
	{  
		in_addr _in = { in }; 
		return inet_ntoa_r(_in, buf);
	}

	static char *inet_ntoa_r(unsigned int in, char *buf)
	{
		in_addr _in = {in};
		return inet_ntoa_r(_in, buf);
	}
	
	static char *inet_ntoa_r(struct in_addr in, char *buf)
	{
		register char *p;  
		p = (char *)&in;
		#define UC(b)   (((int)b)&0xff)  
		sprintf(buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3])); 
		return buf;
	}

	int Reconnect(int timeout = -1);
	/*
	*���ܣ�
			�Ƿ���һ�����õ�SOCKET������
	*���������
			��
	*���������
			��
	*����ֵ��	
			true:����
			false:������
	*/
	bool IsValid(){return m_iSocket != INVALID_SOCKET;}

	inline int Type()const {return m_iType;}
	/*
	*���ܣ�
			ȡ�ϴβ����Ĵ�����
	*���������
			��
	*���������
			��
	*����ֵ��	
			�ϴβ����Ĵ�����
	*/
	inline int GetLastError()const{return errno;}

	
	/*
	*���ܣ�
			ȡ�ϴβ����Ĵ�����Ϣ
	*���������
			��
	*���������
			string &s:���صĴ�����Ϣ��������S��
	*����ֵ��	
			��
	*/
	const char *GetLastErrorMsg()const
	{
		return strerror(errno);
	}

	

	/*
	*���ܣ�
			�����µ�TCP SOCKET������
	*���������
			��
	*���������
			��
	*����ֵ��	
			INVALID_SOCKET:ʧ��
			��INVALID_SOCKET:���ɵ�SOCKET������
	*/
	int CreateTcp(){return Create(SOCK_STREAM);}

	/*
	*���ܣ�
			�����µ�UDP SOCKET������
	*���������
			��
	*���������
			��
	*����ֵ��	
			INVALID_SOCKET:ʧ��
			��INVALID_SOCKET:���ɵ�SOCKET������
	*/
	int CreateUdp(){return Create(SOCK_DGRAM);}


	/*
	*���ܣ�
			�������µ�SOCKET������
	*���������
			int iSocket:�µ�SOCKET������
	*���������
			��
	*����ֵ��	
			��
	*/
	void Attach(int iSocket, int iType);

	
	/*
	*���ܣ�
			ȡ������
	*���������
			��
	*���������
			��
	*����ֵ��	
			��
	*/
	void Dettach()
	{
		m_iSocket = INVALID_SOCKET;
		m_iType = -1;
		memset(&m_PeerAddr, 0, sizeof(m_PeerAddr));
		memset(&m_SockAddr,0, sizeof(m_SockAddr));
	}

	/*
	*����Զ˵�ַ
	*/
	int SavePeerAddr();

	/*
	*���汾�˵�ַ
	*/
	int SaveSockAddr();

	/*
	*�������˵�ַ
	*/
	int SaveAddr();

	
	/*
	*���ܣ�
			���ӵ�SERVER
	*���������
	*		const char *szServerIP:������IP
			unsigned short uServerPort���������˿�
			int timeout_usec: ���ӳ�ʱ�ĺ�����(1��=1000���룩
				   -1������ʽ����
				   0:��������ʱ�������أ����ȴ�
				   >0����������ʱ�ȴ�����ʱ

	*���������
			��
	*����ֵ��	
			0:�ɹ�
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
	*/
	int Connect(const char *szServerIP, unsigned short iServerPort, int timeout_usec = -1);
	int Connect(struct sockaddr_in *addr, int timeout_usec =-1);


	/*
	*���ܣ�
			���ӵ�SERVER
	*���������
	*		const struct sockaddr *serv_addr:��������ַ
			socklen_t addrlen����ַ�ṹ�ĳ���
			int timeout_usec: ���ӳ�ʱ�ĺ�����(1��=1000���룩
				   -1������ʽ����
				   0:��������ʱ�������أ����ȴ�
				   >0����������ʱ�ȴ�����ʱ

	*���������
			��
	*����ֵ��	
			0:�ɹ�
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
	*/
	
	int  Connect(unsigned int ip, unsigned short port,int timeout_usec = -1);
	

	
	/*
	*���ܣ�
			�ر�SOCKET������
	*���������
			��
	*���������
			��
	*����ֵ��	
			0:�ɹ�
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
	*/
	int Close();

	/*
	*���ܣ�
			�ر�SOCKET������
	*���������
			��
	*���������
			��
	*����ֵ��	
			0:�ɹ�
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
	*/
	int Shutdown(int how = SHUT_RDWR);

	/*
	*���ܣ�
			������
	*���������
			unsigned int nSize:�������ĳ���
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1������ʽ����
			 0:������ʱ�������أ����ȴ�
			 >0��������ʱ�ȴ�����ʱ
	*���������
			char *chBuffer:������
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�Է��ѹر�
			>0:�յ������ݳ���
	*/
	int Recv(char *chBuffer, unsigned int nSize, int timeout = -1, int flags = 0);
	

	/*
	*���ܣ�
			������
	*���������
			const void *chBuff�������͵�����
			unsigned int nSize:�������ĳ���
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1������ʽ����
			 0:���ɷ�ʱ�������أ����ȴ�
			 >0�����ɷ�ʱ�ȴ�����ʱ
			int flag:���ͱ�־
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			>=0:�ѷ��͵����ݳ���
	*/
	int Send(const void* chBuff, unsigned int nSize, int timeout = -1, int flags = 0);

	/*
	*���ܣ�
			������(һֱ����nBytes�ֽ�Ϊֹ�����߳���Ϊֹ�����߳�ʱΪֹ��
	*���������
			unsigned int &nBytes:�������ĳ���
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1������ʽ����
			 0:������ʱ�������أ����ȴ�
			 >0��������ʱ�ȴ�����ʱ
			int flags:��־
	*���������
			unsigned int &nBytes:�������յ����ֽ���
			char *chBuffer:�������а������յ�������
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�Է��ѹر�
			>0:�յ������ݳ���
	*/
	int RecvN(unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);

	int ReadN(unsigned int &nread, void *vptr, unsigned int n);

	/*
	*���ܣ�
			��һ���� 
	*���������
			pfHandleInput pf:�����ȼ��ĺ���
				(����ֵ:0��Ҫ���������ݲ���ȷ����
					-1:���ݳ���
					>0:����ȫ��)
			unsigned int nBytes:�������ĳ���		
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1������ʽ����
			 0:������ʱ�������أ����ȴ�
			 >0��������ʱ�ȴ�����ʱ
			int flags:��־
	*���������
			unsigned int &nRead:�������յ����ֽ���
			char *pBuffer:�������а������յ�������
	
	����ֵ: >0:read ok, pack size is return, actually read size is nread, nread>=����ֵ
	0:closed by peer
	-1:error
	-2:timeout
	-3:data invalid
	*/
	int RecvPack(pfHandleInput pf, unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);

	/*
	*���ܣ�
			��һ���� 
	*���������
			pfHandleInput pf:�����ȼ��ĺ���
				(����ֵ:0��Ҫ���������ݲ���ȷ����
					-1:���ݳ���
					>0:����ȫ��)
			unsigned int hlen:��ͷ�ĳ���
			unsigned int nBytes:�������ĳ���		
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1������ʽ����
			 0:������ʱ�������أ����ȴ�
			 >0��������ʱ�ȴ�����ʱ
			int flags:��־
	*���������
			unsigned int &nRead:�������յ����ֽ���
			char *pBuffer:�������а������յ�������
	
	����ֵ: >0:read ok, pack size is return, actually read size is nread, nread>=����ֵ
	0:closed by peer
	-1:error
	-2:timeout
	-3:data invalid
	*/
	int RecvPack(pfHandleInput pf, unsigned int hlen, unsigned int &nread, void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);


	/*
	*���ܣ�
			������(һֱ����nBytes�ֽ�Ϊֹ�����߳���Ϊֹ�����߳�ʱΪֹ��
	*���������
			const void *pBuffer:�����͵Ļ�����
			unsigned int &nBytes:�������ĳ���
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1������ʽ����
			 0:���ɷ�ʱ�������أ����ȴ�
			 >0�����ɷ�ʱ�ȴ�����ʱ
			int flags:��־
	*���������
			unsigned int &nBytes:�����ѷ��͵��ֽ���
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�Է��ѹر�
			>0:�յ������ݳ���
	*/
	int SendN(unsigned int &nwrite, const void *pBuffer, unsigned int nBytes,int timeout = -1, int flags = 0);
	
	int WriteN(unsigned int &nwrite, const void *pBuffer, unsigned int nBytes);

	
	
	/*
	*���ܣ�
			�󶨱��ص�ַ
	*���������
			const char *pszBindIP, const ���󶨵�IP
			unsigned short iBindPort	���󶨵Ķ˿�
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int Bind(const char *pszBindIP, const unsigned short iBindPort);

	/*
	*���ܣ�
			ȡSOCKETѡ��
	*���������
			int level:����
			int optname:ѡ������
	*���������
			void *optval:ѡ��ֵ
			socklen_t *optlen:ѡ��ֵ�ĳ���
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int GetSockOpt(int level, int optname, void *optval, socklen_t *optlen);

	/*
	*���ܣ�
			����SOCKETѡ��
	*���������
			int level:����
			int optname:ѡ������
			void *optval:ѡ��ֵ
			socklen_t *optlen:ѡ��ֵ�ĳ���

	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int SetSockOpt(int level, int optname, const void *optval, socklen_t optlen);



	/*
	*���ܣ�
			ȡSOCKET�ı��˵�ַ
	*���������
			socklen_t *namelen:��ʼ��Ϊname�ṹ��Ĵ�С
	*���������
			struct sockaddr *name�����صı��˵�ַ
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int GetSockName(struct sockaddr *name, socklen_t *namelen);

	
	/*
	*���ܣ�
			ȡ���˵�IP��ַ
	*���������
			��
	*���������
			sring &strHost�����صı���IP��ַ
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int GetSockIP(string &strHost)
	{
		if(SaveSockAddr() != 0)
		{
			return -1;
		}
		char buf[32] = {0};
		strHost = inet_ntoa_r(m_SockAddr.sin_addr, buf);
		return 0;
	}

	int GetSockIP(char * ip, int len)
	{
        string s;

        GetSockIP(s);

        strncpy(ip, s.c_str(), len);

		return 0;
	}

	/*
	*���ܣ�
			ȡ���˵Ķ� ��
	*���������
			��
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			>=0:���˵Ķ˿ں�
	*/
	int GetSockPort()
	{
		if( 0 != SaveSockAddr())
		{
			return -1;
		}
		return ntohs(m_SockAddr.sin_port);
	}

	
	
	/*
	*���ܣ�
			ȡSOCKET�ĶԶ˵�ַ
	*���������
			socklen_t *namelen:��ʼ��Ϊname�ṹ��Ĵ�С
	*���������
			struct sockaddr *name�����صı��˵�ַ
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int GetPeerName(struct sockaddr *name, socklen_t *namelen);

	/*
	*���ܣ�
			ȡ�Զ˵�IP��ַ
	*���������
			��
	*���������
			sring &strHost�����صĶԶ�IP��ַ
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int GetPeerIP(string &strHost)
	{
		if( SavePeerAddr() != 0)
		{
			return -1;
		}
		char buf[32] = {0};
		strHost = inet_ntoa_r(m_PeerAddr.sin_addr, buf);
		return 0;
	}

	int GetPeerIP(char * ip, int len)
	{
        string s;

        GetPeerIP(s);

        strncpy(ip, s.c_str(), len);

		return 0;
	}

	/*
	*���ܣ�
			ȡ�Զ˵Ķ� ��
	*���������
			��
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			>��0:�Զ˵Ķ˿ں�
	*/
	int GetPeerPort()
	{
		if( 0 != SaveSockAddr())
		{
			return -1;
		}
		return ntohs(m_PeerAddr.sin_port);
	}
	
	/*
	*���ܣ�
			���÷�����ѡ��
	*���������
			bool flag:�Ƿ�Ϊ������
				true:��
				false:����
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɹ�
	*/
	int SetNonBlockOption(bool flag = true);

	static int SetNonBlockOption(int sock, bool flag = true);


    	bool SetReuseAddr(int on = 1)
      {
                   return setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(int)) != -1;
      }

	
	/*
	*���ܣ�
			�ȴ��ɶ�
	*���������
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1:һֱ�ȴ�
			 0:���ȴ�
			 >0�����ȴ�timeout����
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:�ɶ�
	*/
	int WaitRead(int timeout = -1);

	/*
	*���ܣ�
			�ȴ���д
	*���������
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1:һֱ�ȴ�
			 0:���ȴ�
			 >0�����ȴ�timeout����
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			0:��д
	*/
	int WaitWrite(int timeout = -1);
	
	/*
	*���ܣ�
			�ȴ���д��ɶ�
	*���������
			int timeout: ��ʱ�ĺ�����(1��=1000���룩
			 -1:һֱ�ȴ�
			 0:���ȴ�
			 >0�����ȴ�timeout����
	*���������
			��
	*����ֵ��	
			-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
			1:��д
			2:�ɶ�
			3:����д���ֿɶ�
	*/
	int WaitRdWr(int timeout = -1);	

	int SendRecvWithRetry(
		const char *req,
    		unsigned int req_len, 
		pfHandleInput pf,
		unsigned int hlen,    		
    		char *rsp, 
    		unsigned int rsp_len,  
    		int timeout = -1,
    		int flags = 0)
	{
		int n = SendRecv(req, req_len, pf, hlen, rsp, rsp_len, timeout, flags) ;
		if( n <= 0)
		{
			if( Reconnect(timeout) != 0)
			{
				return n;
			}
			n = SendRecv(req, req_len, pf, hlen, rsp, rsp_len, timeout, flags); 
		}
		return n;
	}

	/*
	*	-1:recv error
		-2:recv timeout
		-3:recv invalid data
		-4:send error
		-5:send timeout		
		0:closed
		>0:ok, ret the pack len.
	*/
	int SendRecv(
		const char *req,
    		unsigned int req_len, 
		pfHandleInput pf,
		unsigned int hlen,    		
    		char *rsp, 
    		unsigned int rsp_len,  
    		int timeout = -1,
    		int flags = 0);
	
	
protected:
	/*
	*���ܣ�
			�����µ�SOCKET������
	*���������
			int nSocketType:SOCKET���ͣ�SOCK_DGRAM:UDP,SOCK_STREAM:TCP)
	*���������
			��
	*����ֵ��	
			INVALID_SOCKET:ʧ��
			��INVALID_SOCKET:���ɵ�SOCKET������
	*/
	int Create(int nSocketType);

	
	int m_iSocket;	//SOCKET������
	int m_iType;
	
	struct sockaddr_in m_SockAddr;	//���˵�ַ
	struct sockaddr_in m_PeerAddr;	//�Զ˵�ַ

};


class CUdpSocket : public CBaseSocket
{
public:
/*
*���ܣ�
		�����ݣ���Ҫ����UDP��
*���������
		void *pBuffer:���ջ�����
		unsigned int nBytes:���ջ������Ĵ�С
		int iFlags:��־
*���������
		void *pBuffer:���ջ�����
		struct sockaddr *pFromAddr:�Է��ĵ�ַ 
		socklen_t* iAddrLen����ַ�ĳ���
*����ֵ��	
		-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
		0:�Է��ѹر�
		>0:�յ������ݳ���
*/
int Recvfrom(void *pBuffer, unsigned int nBytes, struct sockaddr *pFromAddr, socklen_t* iAddrLen, int iFlags /*= 0*/)
{
	return recvfrom(m_iSocket, pBuffer, nBytes,iFlags, pFromAddr, iAddrLen);
}

/*
*���ܣ�
		�����ݣ���Ҫ����UDP��
*���������
		void *pBuffer�����ͻ�����
		unsigned int nBytes:���ͻ������Ĵ�С
		int iFlags:��־
		struct sockaddr *pFromAddr:�Է��ĵ�ַ 
		socklen_t iAddrLen����ַ�ĳ���

*���������
		��
*����ֵ��	
		-1��ʧ�ܣ�����������ͨ��GetLastError()��ȡ
		>=0:�ѷ��͵����ݳ���
*/
int Sendto(void *pBuffer, unsigned int nBytes, struct sockaddr *pToAddr, socklen_t iAddrLen,int iFlags /*= 0*/ )
{
	return sendto(m_iSocket, pBuffer, nBytes,iFlags, pToAddr, iAddrLen);
}


};


#endif // _CSOCKET_H_


