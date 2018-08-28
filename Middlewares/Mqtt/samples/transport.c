#include "transport.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "string.h"
#include "stdio.h"

static int mysock;

/************************************************************************
** ��������: transport_sendPacketBuffer									
** ��������: ��TCP��ʽ��������
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
int transport_sendPacketBuffer( uint8_t * buf, int buflen)
{
	return write(mysock, buf, buflen);
}

/************************************************************************
** ��������: transport_getdata									
** ��������: �������ķ�ʽ����TCP����
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int count�����ݳ���
** ���ڲ���: <=0��������ʧ��									
************************************************************************/
int transport_getdata(uint8_t * buf, int count)
{
	return recv(mysock, buf, count, 0);
}

/************************************************************************
** ��������: transport_open									
** ��������: ��һ���ӿڣ����Һͷ����� ��������
** ��ڲ���: char* servip:����������
**           int   port:�˿ں�
** ���ڲ���: <0������ʧ��										
************************************************************************/
int transport_open(char* servip, int port)
{
	int *sock = &mysock;
	int ret;
	//int opt;
	struct sockaddr_in addr;
	
	struct hostent* hptr;
	ip_addr_t rmtipaddr;

	
	if((hptr = gethostbyname(servip)) == NULL)
	{
		printf("gethostbyname error for host %s\n", servip);
	//	return -1;
	}

	ipaddr_aton(inet_ntoa(*((struct in_addr *)hptr->h_addr_list[0])), &rmtipaddr);

	printf("rmtipaddr %lX \r\n", rmtipaddr.addr);

	//��ʼ����������Ϣ
	memset(&addr,0,sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = AF_INET;
	//��д�������˿ں�
	addr.sin_port = PP_HTONS(port);
	//��д������IP��ַ
	addr.sin_addr.s_addr = rmtipaddr.addr;
	
	//����SOCK
	*sock = socket(AF_INET,SOCK_STREAM,0);
	//���ӷ����� 
	ret = connect(*sock,(struct sockaddr*)&addr,sizeof(addr));
	if(ret != 0)
	{
		 //�ر�����
		 close(*sock);
		 //����ʧ��
		 return -1;
	}
	//���ӳɹ�,���ó�ʱʱ��1000ms
	//opt = 1000;
	//setsockopt(*sock,SOL_SOCKET,SO_RCVTIMEO,&opt,sizeof(int));
	
	//�����׽���
	return *sock;
}


/************************************************************************
** ��������: transport_close									
** ��������: �ر��׽���
** ��ڲ���: unsigned char* buf�����ݻ�����
**           int buflen�����ݳ���
** ���ڲ���: <0��������ʧ��							
************************************************************************/
int transport_close(int sock)
{
	int rc;
	rc = close(mysock);
	return rc;
}
