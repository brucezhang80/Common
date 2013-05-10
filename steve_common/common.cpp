/*
 * @File: common.cpp
 * @Desc: implement file of Common
 * @Author: stevetang
 * @History:
 *      2007-11-26   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include <sys/ioctl.h>
#include <net/if.h>
#include "general.h"
#include "common.h"

/*
 * Function: FlatStr
 * Desc: ����ַ�������β�ո�ͻس�����
 * In: 
 *     char *   buf   �ַ���
 * Out: 
 *     char *   buf   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int FlatStr(char * buf)
{
    char * ptr = buf + strlen(buf) - 1;

    /* ȥ��β���ո� \r \n */
    while (*ptr == ' ' || *ptr == '\r' || *ptr == '\n' || * ptr == '\t')
    {
        *ptr = 0x00;
        ptr--;
    }

    /* ȥ���ַ����׶˿ո� */
    ptr = buf;
    while ((*ptr == ' ' || * ptr == '\t') && *ptr)
        ptr++;

    /* ���������������� */
    strcpy(buf, ptr);

    return 0;
}

/*
 * Function: TrimStr
 * Desc: �����ַ���,ȥ���س�
 * In: 
 *     char *   buf   �ַ���
 * Out: 
 *     char *   buf   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int TrimStr(char * buf)
{
    FlatStr(buf);

    char * ptr = buf;

    while (*ptr)
    {
        if (*ptr != '\r' && *ptr != '\n')
        {
            ptr++;
            continue;
        }

        strcpy(ptr, ptr+1);
    }

    return 0;
}

/*
 * Function: CheckHalfChinese
 * Desc: ȥ������İ������
 * In: 
 *     char *   buf   �ַ���
 * Out: 
 *     char *   buf   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CheckHalfChinese(char * buf)
{
	char * s = buf + strlen(buf) - 1;
	int n = 0;

	while (s >= buf && ((unsigned char)*s > 127)) //0x7F
	{
		s--;
		n++;
	}

	if (n%2) /* ���� */
	{
		*(buf + strlen(buf) - 1) = 0x00;
	}

    return 0;
}

/*
 * Function: GetLocalIP
 * Desc: ȡ����IP��ַ
 * In: 
 *     char *   ip   �ַ���
 * Out: 
 *     char *   ip   �ַ���
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int GetLocalIP(char * ip)
{
    char tmp[100];

    system("/sbin/ifconfig eth1 | grep '172\\.' | awk '{print $2}' | awk -F : '{print $2}' > local.tmp");

    FILE * fp = fopen("local.tmp", "r");
    if (fp)
    {
        memset(tmp, 0, sizeof(tmp));
        fgets(tmp, sizeof(tmp)-1, fp);

        fclose(fp);

        FlatStr(tmp);

        strcpy(ip, tmp);
    }

    return 0;
}

// ����if��ȡ������Ӧ��ip
int GetIpByIf(char *ifname, char *pszip) 
{
	register int fd, ret;
	struct ifreq buf;

	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	strcpy(buf.ifr_name, ifname);

	ret = ioctl (fd, SIOCGIFADDR, (char *) &buf);
	close(fd);

	if (ret)
		return -1;

	strncpy(pszip, inet_ntoa(((struct sockaddr_in*)(&buf.ifr_addr))->sin_addr), 15);
	pszip[15] = 0;
	return 0;
}

//����·��
int CreateDirectory(const char * path)
{
    char* pBegin = (char *)path;
    char* pEnd = (char *)path;
    char  szPath[256] = {'\0'};
    int   iReturn = -1;

    while (1)
    {
        pEnd = strstr (pBegin, "/");

        if (0 == pEnd)
        {
            break;
        }

        memset (szPath, 0x00, sizeof(szPath));
        memcpy (szPath, path, (((pEnd - path) == 0) ? 1 : (pEnd - path)));

        iReturn = mkdir(szPath, 0777);

        if ((0 != iReturn) && (EEXIST != errno))
        {
            return -1;
        }
        
        pBegin = pEnd + 1;
    }

    if (strlen(pBegin) > 0)
    {
        mkdir (path, 0777);
    }

	return 0;
}

//��ȡ�ļ���
int GetFileName(const char * path, char * fname)
{
	char * ptr = (char *)(path + strlen(path) -1);

	while (*ptr != '/' && *ptr != '\\' && ptr >= path)
		ptr--;

	ptr++;

	if ((unsigned int)(ptr - path) < strlen(path))
	{
		strcpy(fname, ptr);
	}

	return 0;
}

//��ȡ·����
int GetPath(const char * fname, char * path)
{
	char * ptr = (char *)(fname + strlen(fname) -1);

	while (*ptr != '/' && *ptr != '\\' && ptr >= fname)
		ptr--;

	ptr++;

	if ((unsigned int)(ptr - fname) < strlen(fname))
	{
		strncpy(path, fname, ptr-fname);
		path[ptr-fname] = 0x00;
	}

	return 0;
}

//��ȡ�ļ�������չ��
int GetFileExt(const char * path, char * fname, char * ext)
{
	char tmp[256];

	GetFileName(path, tmp);

	char * ptr = strstr(tmp, ".");
	if (ptr == NULL)
	{
		strcpy(fname, tmp);
		*ext = 0x00;
	}
	else
	{
		strncpy(fname, ptr, ptr-tmp);
		fname[ptr-tmp] = 0x00;
		ptr++;
		strcpy(ext, ptr);
	}

	return 0;
}
