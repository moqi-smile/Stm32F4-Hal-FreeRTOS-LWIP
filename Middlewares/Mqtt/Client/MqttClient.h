#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "MQTTPacket.h"

#include "transport.h"

#define ClientID                "0A0B0C0D|securemode=3,signmethod=hmacsha1,timestamp=1534728682167|"
#define UserName                "RilP0p5R80rdZgy38yMn&a1JREJMZPKu"
#define UserPassword            "defffff5132cb6110549f114244cb68415553f4a"
#define HOST_NAME				"a1JREJMZPKu.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define HOST_PORT				1883

#define ReleaseMsg              "/a1JREJMZPKu/RilP0p5R80rdZgy38yMn/update"
#define SubscribeMsg            "/a1JREJMZPKu/RilP0p5R80rdZgy38yMn/get"

enum QoS { QOS0, QOS1, QOS2 };
#define MSG_MAX_LEN    300
#define MSG_TOPIC_LEN  50

#define  KEEPLIVE_TIME 50

//数据交互结构体
typedef struct __MQTTMessage
{
	uint32_t qos;
	uint8_t  retained;
	uint8_t  dup;
	uint16_t id;
	uint8_t  type;
	void *payload;
	int payloadlen;
}MQTTMessage;

//用户接收消息结构体
typedef struct __MQTT_MSG
{
	uint8_t   msgqos;                 //消息质量
	uint8_t   msg[MSG_MAX_LEN];       //消息
	uint32_t msglenth;               //消息长度
	uint8_t   topic[MSG_TOPIC_LEN];   //主题    
	uint16_t packetid;               //消息ID
	uint8_t   valid;                  //标明消息是否有效
}MQTT_USER_MSG;

void UserMsgCtl(MQTT_USER_MSG  *msg);
void mqtt_pktype_ctl(uint8_t  packtype,uint8_t  *buf,uint32_t buflen);
int my_mqtt_send_pingreq(int sock);
uint16_t GetNextPackID(void);
int WaitForPacket(int sock,uint8_t packettype,uint8_t times);
int MQTTMsgPublish(int sock, char *topic, char qos, char retained,uint8_t * msg,uint32_t msg_len);
void deliverMessage(MQTTString  *TopicName,MQTTMessage *msg,MQTT_USER_MSG *mqtt_user_msg);
int ReadPacketTimeout(int sock,uint8_t  *buf,int buflen,uint32_t timeout);
int MQTTSubscribe(int sock,char *topic,enum QoS pos);
int MQTTClientInit(int sock);

#endif
