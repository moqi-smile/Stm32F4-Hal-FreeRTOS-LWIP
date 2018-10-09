
#include "main.h"

#include "Json/cJson.h"
#include "malloc/malloc.h"

#include "lwip/tcpip.h" 
#include "lwip/sockets.h" 

#include "string.h"

#include "MqttClient.h"

//�����û���Ϣ�ṹ��
MQTT_USER_MSG  mqtt_user_msg;

int MQTTClientInit(int sock, uint8_t *buffer, int buflen)
{
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	int len = 0;
	int rc = 0;
	uint8_t sessionPresent,connack_rc;

	fd_set readfd;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	if (buffer == NULL)
	{
		rc = -6;
		goto exit;
	}
	memset(buffer, 0, buflen);

	FD_ZERO(&readfd);
	FD_SET(sock,&readfd);	

	//����MQTT�ͻ������Ӳ���
	connectData.willFlag = 0;
	//MQTT�汾
	connectData.MQTTVersion = 4;
	//�ͻ���ID--����Ψһ
	connectData.clientID.cstring = ClientID;
	//������
	connectData.keepAliveInterval = KEEPLIVE_TIME;
	//�û���
	connectData.username.cstring = UserName;
	//�û�����
	connectData.password.cstring = UserPassword;
	//����Ự
	connectData.cleansession = 1;

	//���л�������Ϣ
	len = MQTTSerialize_connect(buffer, buflen, &connectData);
	//����TCP����
	if(transport_sendPacketBuffer(buffer, len) < 0)
	{
		rc = -1;
		goto exit;
	}

	//�ȴ��ɶ��¼�--�ȴ���ʱ
	if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
	{
		rc = -2;
		goto exit;
	}
	//�пɶ��¼�--û�пɶ��¼�
	if(FD_ISSET(sock,&readfd) == 0)
	{
		rc = -3;
		goto exit;
	}

	if(MQTTPacket_read(buffer, buflen, transport_getdata) != CONNACK)
		rc = -4;	
	//������ӻ�Ӧ��
	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buffer, buflen) != 1 || connack_rc != 0)
	{
		rc = -5;
		goto exit;
	}

	if(sessionPresent == 1)
	{
		rc = 1;//����Ҫ���¶���--�������Ѿ���ס�˿ͻ��˵�״̬
		goto exit;
	}
	else 
		rc = 0;//��Ҫ���¶���
	exit:
	return rc;//��Ҫ���¶���
}


/************************************************************************
** ��������: MQTTSubscribe								
** ��������: ������Ϣ
** ��ڲ���: int sock���׽���
**           char *topic������
**           enum QoS pos����Ϣ����
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע: 
************************************************************************/
int MQTTSubscribe(int sock,char *topic,enum QoS pos, uint8_t *buffer, int buflen)
{
	static uint32_t PacketID = 0;
	uint16_t packetidbk = 0;
	int conutbk = 0;
	MQTTString topicString = MQTTString_initializer;  
	int len;
	int req_qos,qosbk;
	int rc = 0;

	fd_set readfd;
	struct timeval tv;

	if (buffer == NULL)
	{
		rc = -7;
		goto exit;
	}
	memset(buffer, 0, buflen);

	tv.tv_sec = 2;
	tv.tv_usec = 0;

	FD_ZERO(&readfd);
	FD_SET(sock,&readfd);		

	//��������
	topicString.cstring = (char *)topic;
	//��������
	req_qos = pos;

	//���л�������Ϣ
	len = MQTTSerialize_subscribe(buffer, buflen, 0, PacketID++, 1, &topicString, &req_qos);
	//����TCP����
	if(transport_sendPacketBuffer(buffer, len) < 0)
	{
		rc = -1;
		goto exit;
	}

	//�ȴ��ɶ��¼�--�ȴ���ʱ
	if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
	{
		rc = -2;
		goto exit;
	}
	//�пɶ��¼�--û�пɶ��¼�
	if(FD_ISSET(sock,&readfd) == 0)
	{
		rc = -3;
		goto exit;
	}

	//�ȴ����ķ���--δ�յ����ķ���
	if(MQTTPacket_read(buffer, buflen, transport_getdata) != SUBACK)
	{
		rc = -4;
		goto exit;
	}

	//���Ļ�Ӧ��
	if(MQTTDeserialize_suback(&packetidbk,1, &conutbk, &qosbk, buffer, buflen) != 1)
	{
		rc = -5;
		goto exit;
	}


	//��ⷵ�����ݵ���ȷ��
	if((qosbk == 0x80)||(packetidbk != (PacketID-1)))
	{
		rc = -6;
		goto exit;
	}

	exit:

	//���ĳɹ�
	return rc;
}

/************************************************************************
** ��������: ReadPacketTimeout					
** ��������: ������ȡMQTT����
** ��ڲ���: int sock:����������
**           uint8_t  *buf:���ݻ�����
**           int buflen:��������С
**           uint32_t timeout:��ʱʱ��--0-��ʾֱ�Ӳ�ѯ��û��������������
** ���ڲ���: -1������,����--������
** ��    ע: 
************************************************************************/
int ReadPacketTimeout(int sock,uint8_t  *buf,int buflen,uint32_t timeout)
{
	fd_set readfd;
	struct timeval tv;
	if(timeout != 0)
	{
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		FD_ZERO(&readfd);
		FD_SET(sock,&readfd); 

		//�ȴ��ɶ��¼�--�ȴ���ʱ
		if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
				return -1;
		//�пɶ��¼�--û�пɶ��¼�
		if(FD_ISSET(sock,&readfd) == 0)
				return -1;
	}
	//��ȡTCP/IP�¼�
	return MQTTPacket_read(buf, buflen, transport_getdata);
}

/************************************************************************
** ��������: deliverMessage						
** ��������: ���ܷ�������������Ϣ
** ��ڲ���: MQTTMessage *msg:MQTT��Ϣ�ṹ��
**           MQTT_USER_MSG *mqtt_user_msg:�û����ܽṹ��
**           MQTTString  *TopicName:����
** ���ڲ���: ��
** ��    ע: 
************************************************************************/
void deliverMessage(MQTTString  *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg)
{
	//��Ϣ����
	mqtt_user_msg->msgqos = msg->qos;
	//������Ϣ
	memcpy(mqtt_user_msg->msg,msg->payload,msg->payloadlen);
	mqtt_user_msg->msg[msg->payloadlen] = 0;
	//������Ϣ����
	mqtt_user_msg->msglenth = msg->payloadlen;
	//��Ϣ����
	memcpy((char *)mqtt_user_msg->topic,TopicName->lenstring.data,TopicName->lenstring.len);
	mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
	//��ϢID
	mqtt_user_msg->packetid = msg->id;
	//������Ϣ�Ϸ�
	mqtt_user_msg->valid = 1;		
}

/************************************************************************
** ��������: UserMsgCtl						
** ��������: �û���Ϣ������
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: ��
** ��    ע: 
************************************************************************/
void UserMsgCtl(MQTT_USER_MSG  *msg)
{
	//���ﴦ������ֻ�Ǵ�ӡ���û���������������Լ��Ĵ���ʽ
	printf("MQTT>>****�յ��ͻ����Լ����ĵ���Ϣ����****\n");
	//���غ�����Ϣ
	switch(msg->msgqos)
	{
		case 0:
			printf("MQTT>>��Ϣ������QoS0\n");
			break;
		case 1:
			printf("MQTT>>��Ϣ������QoS1\n");
			break;
		case 2:
			printf("MQTT>>��Ϣ������QoS2\n");
			break;
		default:
			printf("MQTT>>�������Ϣ����\n");
			break;
	}
	printf("MQTT>>��Ϣ���⣺%s\r\n",msg->topic);	
	printf("MQTT>>��Ϣ���ݣ�%s\r\n",msg->msg);	
	printf("MQTT>>��Ϣ���ȣ�%d\r\n",msg->msglenth);	 

	//���������������
	msg->valid  = 0;
}

/************************************************************************
** ��������: mqtt_pktype_ctl						
** ��������: ���ݰ����ͽ��д���
** ��ڲ���: uint8_t  packtype:������
** ���ڲ���: ��
** ��    ע: 
************************************************************************/
void mqtt_pktype_ctl(uint8_t  packtype,uint8_t  *buf,uint32_t buflen)
{
	MQTTMessage msg;
	int rc;
	MQTTString receivedTopic;
	uint32_t len;

	switch(packtype)
	{
		case PUBLISH:
			//����PUBLISH��Ϣ
			if(MQTTDeserialize_publish(&msg.dup,(int*)&msg.qos, &msg.retained, &msg.id, &receivedTopic,(unsigned char **)&msg.payload, &msg.payloadlen, buf, buflen) != 1)
			return;	
			//������Ϣ
			deliverMessage(&receivedTopic,&msg,&mqtt_user_msg);

			//��Ϣ������ͬ������ͬ
			if(msg.qos == QOS0)
			{
				//QOS0-����ҪACK
				//ֱ�Ӵ�������
				UserMsgCtl(&mqtt_user_msg);
				return;
			}
			//����PUBACK��Ϣ
			if(msg.qos == QOS1)
			{
				len =MQTTSerialize_puback(buf,buflen,mqtt_user_msg.packetid);
				if(len == 0)
					return;
				//���ͷ���
				if(transport_sendPacketBuffer(buf,len)<0)
					return;	
				//���غ�����Ϣ
				UserMsgCtl(&mqtt_user_msg); 
					return;												
			}

			//��������2,ֻ��Ҫ����PUBREC�Ϳ�����
			if(msg.qos == QOS2)
			{
				len = MQTTSerialize_ack(buf, buflen, PUBREC, 0, mqtt_user_msg.packetid);			                
				if(len == 0)
					return;
				//���ͷ���
				transport_sendPacketBuffer(buf,len);	
			}
			break;
		case PUBREL:				           
			//���������ݣ������ID��ͬ�ſ���
			rc = MQTTDeserialize_ack(&msg.type,&msg.dup, &msg.id, buf,buflen);
			if((rc != 1)||(msg.type != PUBREL)||(msg.id != mqtt_user_msg.packetid))
			return ;
			//�յ�PUBREL����Ҫ������������
			if(mqtt_user_msg.valid == 1)
			{
				//���غ�����Ϣ
				UserMsgCtl(&mqtt_user_msg);
			}      
			//���л�PUBCMP��Ϣ
			len = MQTTSerialize_pubcomp(buf,buflen,msg.id);	                   	
			if(len == 0)
				return;									
			//���ͷ���--PUBCOMP
			transport_sendPacketBuffer(buf,len);										
			break;
		case PUBACK://�ȼ�1�ͻ����������ݺ󣬷���������
			break;
		case PUBREC://�ȼ�2�ͻ����������ݺ󣬷���������
			break;
		case PUBCOMP://�ȼ�2�ͻ�������PUBREL�󣬷���������
			break;
		default:
			break;
	}
}

/************************************************************************
** ��������: my_mqtt_send_pingreq								
** ��������: ����MQTT������
** ��ڲ���: ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע: 
************************************************************************/
int my_mqtt_send_pingreq(int sock)
{
	int rc = 0;
	int len;
	uint8_t buf[200];
	int buflen = sizeof(buf);	 
	fd_set readfd;
	struct timeval tv;

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	FD_ZERO(&readfd);
	FD_SET(sock,&readfd);			

	len = MQTTSerialize_pingreq(buf, buflen);
	transport_sendPacketBuffer(buf, len);

	//�ȴ��ɶ��¼�
	if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
	{
		rc = -1;
		goto exit;
	}

	//�пɶ��¼�
	if(FD_ISSET(sock,&readfd) == 0)
	{
		rc =  -2;
		goto exit;
	}

	if(MQTTPacket_read(buf, buflen, transport_getdata) != PINGRESP)
	{
		rc = -3;
		goto exit;
	}

	exit:
	return rc;

}

/************************************************************************
** ��������: GetNextPackID						
** ��������: ������һ�����ݰ�ID
** ��ڲ���: ��
** ���ڲ���: uint16_t packetid:������ID
** ��    ע: 
************************************************************************/
uint16_t GetNextPackID(void)
{
	 static uint16_t pubpacketid = 0;
	 return pubpacketid++;
}

/************************************************************************
** ��������: WaitForPacket					
** ��������: �ȴ��ض������ݰ�
** ��ڲ���: int sock:����������
**           u8 packettype:������
**           u8 times:�ȴ�����
** ���ڲ���: >=0:�ȵ����ض��İ� <0:û�еȵ��ض��İ�
** ��    ע: 
************************************************************************/
int WaitForPacket(int sock,uint8_t packettype,uint8_t times)
{
	int type;
	uint8_t buf[MSG_MAX_LEN];
	uint8_t n = 0;
	int buflen = sizeof(buf);
	do
	{
		//��ȡ���ݰ�
		type = ReadPacketTimeout(sock,buf,buflen,2);
		if(type != -1)
			mqtt_pktype_ctl(type,buf,buflen);
		n++;
	}while((type != packettype)&&(n < times));
	//�յ������İ�
	if(type == packettype)
		return 0;
	else 
		return -1;
}

/************************************************************************
** ��������: mqtt_msg_publish						
** ��������: �û�������Ϣ
** ��ڲ���: MQTT_USER_MSG  *msg����Ϣ�ṹ��ָ��
** ���ڲ���: >=0:���ͳɹ� <0:����ʧ��
** ��    ע: 
************************************************************************/
int MQTTMsgPublish(int sock, char *topic, char qos, char retained,uint8_t * msg,uint32_t msg_len)
{
	uint8_t  buf[MSG_MAX_LEN];
	int buflen = sizeof(buf),len;
	MQTTString topicString = MQTTString_initializer;
	uint16_t packid = 0,packetidbk;

	//�������
	topicString.cstring = (char *)topic;

	//������ݰ�ID
	if((qos == QOS1)||(qos == QOS2))
	{ 
		packid = GetNextPackID();
	}
	else
	{
		qos = QOS0;
		retained = 0;
		packid = 0;
	}

	//������Ϣ
	len = MQTTSerialize_publish(buf, buflen, 0, qos, retained, packid, topicString, (unsigned char*)msg, msg_len);
	if(len <= 0)
		return -1;
	if(transport_sendPacketBuffer(buf, len) < 0)	
		return -2;	

	//�����ȼ�0������Ҫ����
	if(qos == QOS0)
	{
		return 0;
	}

	//�ȼ�1
	if(qos == QOS1)
	{
		//�ȴ�PUBACK
		if(WaitForPacket(sock,PUBACK,5) < 0)
			return -3;
		return 1;

	}
	//�ȼ�2
	if(qos == QOS2)	
	{
		//�ȴ�PUBREC
		if(WaitForPacket(sock,PUBREC,5) < 0)
			return -3;
		//����PUBREL
		len = MQTTSerialize_pubrel(buf, buflen,0, packetidbk);
		if(len == 0)
			return -4;
		if(transport_sendPacketBuffer(buf, len) < 0)	
			return -6;			
		//�ȴ�PUBCOMP
		if(WaitForPacket(sock,PUBREC,5) < 0)
			return -7;
		return 2;
	}
	//�ȼ�����
	return -8;
}
