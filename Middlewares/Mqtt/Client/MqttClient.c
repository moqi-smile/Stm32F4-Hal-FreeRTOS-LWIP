
#include "main.h"

#include "Json/cJson.h"
#include "malloc/malloc.h"

#include "lwip/tcpip.h" 
#include "lwip/sockets.h" 

#include "string.h"

#include "MqttClient.h"

//定义用户消息结构体
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

	//创建MQTT客户端连接参数
	connectData.willFlag = 0;
	//MQTT版本
	connectData.MQTTVersion = 4;
	//客户端ID--必须唯一
	connectData.clientID.cstring = ClientID;
	//保活间隔
	connectData.keepAliveInterval = KEEPLIVE_TIME;
	//用户名
	connectData.username.cstring = UserName;
	//用户密码
	connectData.password.cstring = UserPassword;
	//清除会话
	connectData.cleansession = 1;

	//串行化连接消息
	len = MQTTSerialize_connect(buffer, buflen, &connectData);
	//发送TCP数据
	if(transport_sendPacketBuffer(buffer, len) < 0)
	{
		rc = -1;
		goto exit;
	}

	//等待可读事件--等待超时
	if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
	{
		rc = -2;
		goto exit;
	}
	//有可读事件--没有可读事件
	if(FD_ISSET(sock,&readfd) == 0)
	{
		rc = -3;
		goto exit;
	}

	if(MQTTPacket_read(buffer, buflen, transport_getdata) != CONNACK)
		rc = -4;	
	//拆解连接回应包
	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buffer, buflen) != 1 || connack_rc != 0)
	{
		rc = -5;
		goto exit;
	}

	if(sessionPresent == 1)
	{
		rc = 1;//不需要重新订阅--服务器已经记住了客户端的状态
		goto exit;
	}
	else 
		rc = 0;//需要重新订阅
	exit:
	return rc;//需要重新订阅
}


/************************************************************************
** 函数名称: MQTTSubscribe								
** 函数功能: 订阅消息
** 入口参数: int sock：套接字
**           char *topic：主题
**           enum QoS pos：消息质量
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
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

	//复制主题
	topicString.cstring = (char *)topic;
	//订阅质量
	req_qos = pos;

	//串行化订阅消息
	len = MQTTSerialize_subscribe(buffer, buflen, 0, PacketID++, 1, &topicString, &req_qos);
	//发送TCP数据
	if(transport_sendPacketBuffer(buffer, len) < 0)
	{
		rc = -1;
		goto exit;
	}

	//等待可读事件--等待超时
	if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
	{
		rc = -2;
		goto exit;
	}
	//有可读事件--没有可读事件
	if(FD_ISSET(sock,&readfd) == 0)
	{
		rc = -3;
		goto exit;
	}

	//等待订阅返回--未收到订阅返回
	if(MQTTPacket_read(buffer, buflen, transport_getdata) != SUBACK)
	{
		rc = -4;
		goto exit;
	}

	//拆订阅回应包
	if(MQTTDeserialize_suback(&packetidbk,1, &conutbk, &qosbk, buffer, buflen) != 1)
	{
		rc = -5;
		goto exit;
	}


	//检测返回数据的正确性
	if((qosbk == 0x80)||(packetidbk != (PacketID-1)))
	{
		rc = -6;
		goto exit;
	}

	exit:

	//订阅成功
	return rc;
}

/************************************************************************
** 函数名称: ReadPacketTimeout					
** 函数功能: 阻塞读取MQTT数据
** 入口参数: int sock:网络描述符
**           uint8_t  *buf:数据缓存区
**           int buflen:缓冲区大小
**           uint32_t timeout:超时时间--0-表示直接查询，没有数据立即返回
** 出口参数: -1：错误,其他--包类型
** 备    注: 
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

		//等待可读事件--等待超时
		if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
				return -1;
		//有可读事件--没有可读事件
		if(FD_ISSET(sock,&readfd) == 0)
				return -1;
	}
	//读取TCP/IP事件
	return MQTTPacket_read(buf, buflen, transport_getdata);
}

/************************************************************************
** 函数名称: deliverMessage						
** 函数功能: 接受服务器发来的消息
** 入口参数: MQTTMessage *msg:MQTT消息结构体
**           MQTT_USER_MSG *mqtt_user_msg:用户接受结构体
**           MQTTString  *TopicName:主题
** 出口参数: 无
** 备    注: 
************************************************************************/
void deliverMessage(MQTTString  *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg)
{
	//消息质量
	mqtt_user_msg->msgqos = msg->qos;
	//保存消息
	memcpy(mqtt_user_msg->msg,msg->payload,msg->payloadlen);
	mqtt_user_msg->msg[msg->payloadlen] = 0;
	//保存消息长度
	mqtt_user_msg->msglenth = msg->payloadlen;
	//消息主题
	memcpy((char *)mqtt_user_msg->topic,TopicName->lenstring.data,TopicName->lenstring.len);
	mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
	//消息ID
	mqtt_user_msg->packetid = msg->id;
	//标明消息合法
	mqtt_user_msg->valid = 1;		
}

/************************************************************************
** 函数名称: UserMsgCtl						
** 函数功能: 用户消息处理函数
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: 无
** 备    注: 
************************************************************************/
void UserMsgCtl(MQTT_USER_MSG  *msg)
{
	//这里处理数据只是打印，用户可以在这里添加自己的处理方式
	printf("MQTT>>****收到客户端自己订阅的消息！！****\n");
	//返回后处理消息
	switch(msg->msgqos)
	{
		case 0:
			printf("MQTT>>消息质量：QoS0\n");
			break;
		case 1:
			printf("MQTT>>消息质量：QoS1\n");
			break;
		case 2:
			printf("MQTT>>消息质量：QoS2\n");
			break;
		default:
			printf("MQTT>>错误的消息质量\n");
			break;
	}
	printf("MQTT>>消息主题：%s\r\n",msg->topic);	
	printf("MQTT>>消息类容：%s\r\n",msg->msg);	
	printf("MQTT>>消息长度：%d\r\n",msg->msglenth);	 

	//处理完后销毁数据
	msg->valid  = 0;
}

/************************************************************************
** 函数名称: mqtt_pktype_ctl						
** 函数功能: 根据包类型进行处理
** 入口参数: uint8_t  packtype:包类型
** 出口参数: 无
** 备    注: 
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
			//拆析PUBLISH消息
			if(MQTTDeserialize_publish(&msg.dup,(int*)&msg.qos, &msg.retained, &msg.id, &receivedTopic,(unsigned char **)&msg.payload, &msg.payloadlen, buf, buflen) != 1)
			return;	
			//接受消息
			deliverMessage(&receivedTopic,&msg,&mqtt_user_msg);

			//消息质量不同，处理不同
			if(msg.qos == QOS0)
			{
				//QOS0-不需要ACK
				//直接处理数据
				UserMsgCtl(&mqtt_user_msg);
				return;
			}
			//发送PUBACK消息
			if(msg.qos == QOS1)
			{
				len =MQTTSerialize_puback(buf,buflen,mqtt_user_msg.packetid);
				if(len == 0)
					return;
				//发送返回
				if(transport_sendPacketBuffer(buf,len)<0)
					return;	
				//返回后处理消息
				UserMsgCtl(&mqtt_user_msg); 
					return;												
			}

			//对于质量2,只需要发送PUBREC就可以了
			if(msg.qos == QOS2)
			{
				len = MQTTSerialize_ack(buf, buflen, PUBREC, 0, mqtt_user_msg.packetid);			                
				if(len == 0)
					return;
				//发送返回
				transport_sendPacketBuffer(buf,len);	
			}
			break;
		case PUBREL:				           
			//解析包数据，必须包ID相同才可以
			rc = MQTTDeserialize_ack(&msg.type,&msg.dup, &msg.id, buf,buflen);
			if((rc != 1)||(msg.type != PUBREL)||(msg.id != mqtt_user_msg.packetid))
			return ;
			//收到PUBREL，需要处理并抛弃数据
			if(mqtt_user_msg.valid == 1)
			{
				//返回后处理消息
				UserMsgCtl(&mqtt_user_msg);
			}      
			//串行化PUBCMP消息
			len = MQTTSerialize_pubcomp(buf,buflen,msg.id);	                   	
			if(len == 0)
				return;									
			//发送返回--PUBCOMP
			transport_sendPacketBuffer(buf,len);										
			break;
		case PUBACK://等级1客户端推送数据后，服务器返回
			break;
		case PUBREC://等级2客户端推送数据后，服务器返回
			break;
		case PUBCOMP://等级2客户端推送PUBREL后，服务器返回
			break;
		default:
			break;
	}
}

/************************************************************************
** 函数名称: my_mqtt_send_pingreq								
** 函数功能: 发送MQTT心跳包
** 入口参数: 无
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
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

	//等待可读事件
	if(select(sock+1,&readfd,NULL,NULL,&tv) == 0)
	{
		rc = -1;
		goto exit;
	}

	//有可读事件
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
** 函数名称: GetNextPackID						
** 函数功能: 产生下一个数据包ID
** 入口参数: 无
** 出口参数: uint16_t packetid:产生的ID
** 备    注: 
************************************************************************/
uint16_t GetNextPackID(void)
{
	 static uint16_t pubpacketid = 0;
	 return pubpacketid++;
}

/************************************************************************
** 函数名称: WaitForPacket					
** 函数功能: 等待特定的数据包
** 入口参数: int sock:网络描述符
**           u8 packettype:包类型
**           u8 times:等待次数
** 出口参数: >=0:等到了特定的包 <0:没有等到特定的包
** 备    注: 
************************************************************************/
int WaitForPacket(int sock,uint8_t packettype,uint8_t times)
{
	int type;
	uint8_t buf[MSG_MAX_LEN];
	uint8_t n = 0;
	int buflen = sizeof(buf);
	do
	{
		//读取数据包
		type = ReadPacketTimeout(sock,buf,buflen,2);
		if(type != -1)
			mqtt_pktype_ctl(type,buf,buflen);
		n++;
	}while((type != packettype)&&(n < times));
	//收到期望的包
	if(type == packettype)
		return 0;
	else 
		return -1;
}

/************************************************************************
** 函数名称: mqtt_msg_publish						
** 函数功能: 用户推送消息
** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
** 出口参数: >=0:发送成功 <0:发送失败
** 备    注: 
************************************************************************/
int MQTTMsgPublish(int sock, char *topic, char qos, char retained,uint8_t * msg,uint32_t msg_len)
{
	uint8_t  buf[MSG_MAX_LEN];
	int buflen = sizeof(buf),len;
	MQTTString topicString = MQTTString_initializer;
	uint16_t packid = 0,packetidbk;

	//填充主题
	topicString.cstring = (char *)topic;

	//填充数据包ID
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

	//推送消息
	len = MQTTSerialize_publish(buf, buflen, 0, qos, retained, packid, topicString, (unsigned char*)msg, msg_len);
	if(len <= 0)
		return -1;
	if(transport_sendPacketBuffer(buf, len) < 0)	
		return -2;	

	//质量等级0，不需要返回
	if(qos == QOS0)
	{
		return 0;
	}

	//等级1
	if(qos == QOS1)
	{
		//等待PUBACK
		if(WaitForPacket(sock,PUBACK,5) < 0)
			return -3;
		return 1;

	}
	//等级2
	if(qos == QOS2)	
	{
		//等待PUBREC
		if(WaitForPacket(sock,PUBREC,5) < 0)
			return -3;
		//发送PUBREL
		len = MQTTSerialize_pubrel(buf, buflen,0, packetidbk);
		if(len == 0)
			return -4;
		if(transport_sendPacketBuffer(buf, len) < 0)	
			return -6;			
		//等待PUBCOMP
		if(WaitForPacket(sock,PUBREC,5) < 0)
			return -7;
		return 2;
	}
	//等级错误
	return -8;
}
