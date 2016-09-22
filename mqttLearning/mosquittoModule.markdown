#mosquitto模块分析
#概述
__libmosquitto__ 作为 __mosquitto__ 开源代码的一部分，主要用来实现 __MQTT__ 协议栈和数据包通讯功能。
  
下面将对__libmosquitto__包含的部分模块进行分析。  

#mosquitto_internal.h
__mosquitto_internal.h__ 中定义了使用的数据结构。包含下列内容：
* enum枚举值
* struct结构体
* 函数指针
* 全局变量

```c
#ifdef WITH_BROKER
#  include "uthash.h"
struct mosquitto_client_msg;
#endif
```
* 宏定义
* 其他变量定义
* 头文件包含

  <br>
<font color="red" size="5">代码详细注解待添加</font>  
#mosquitto.h

__mosquitto.h__中定义了外部调用的接口，利用__mosquitto__进行开发时，需要包含__mosquitto.h__文件。 包含下列内容：  
* 枚举值
* 宏定义
* 头文件包含
* struct结构体
* 对外接口

###1、mosquitto_message
```c
struct mosquitto_message{
	int mid;
	char *topic;
	void *payload;
	int payloadlen;
	int qos;
	bool retain;
};
```
消息结构
  
name |description|
----|----|
mid | * |
topic|话题名称|
payload|负载内容|
payloadlen|负载长度|
qos|服务质量，value=0，1，2
retain|是否保留，value=0，1|

###3、版本接口
```c
int mosquitto_lib_version(int *major, int *minor, int *revision)
{
	if(major) *major = LIBMOSQUITTO_MAJOR;
	if(minor) *minor = LIBMOSQUITTO_MINOR;
	if(revision) *revision = LIBMOSQUITTO_REVISION;
	return LIBMOSQUITTO_VERSION_NUMBER;
}
```
* 返回版本号
* 参数说明

name|decription|
----|----------|
返回值|int,返回完整的版本号|
major|如果不为NULL，赋值为major version|
minor|如果不为NULL，赋值为ninor version|
revision|如果不为NULL，赋值为revision version|

###4、初始化
```c
int mosquitto_lib_init(void)
{
#ifdef WIN32
	srand(GetTickCount());
#else
	struct timeval tv;

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec*1000 + tv.tv_usec/1000);
#endif

	_mosquitto_net_init();

	return MOSQ_ERR_SUCCESS;
}
```
* 该方法必须在所有__mosquitto方法__之前调用，相当于初始化__mosquitto__使用环境
* 该方法不是__线程安全__的
* 始终返回__MOSQ_ERR_SUCCESS__

###5、资源回收
```c
int mosquitto_lib_cleanup(void)
{
	_mosquitto_net_cleanup();

	return MOSQ_ERR_SUCCESS;
}
```
* 释放分配的资源
* 始终返回__MOSQ_ERR_SUCCESS__

###6、创建客户端实例
```c
struct mosquitto *mosquitto_new(const char *id, bool clean_session, void *userdata)
{
	struct mosquitto *mosq = NULL;
	int rc;

	if(clean_session == false && id == NULL){
		errno = EINVAL;
		return NULL;
	}

#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	mosq = (struct mosquitto *)_mosquitto_calloc(1, sizeof(struct mosquitto));
	if(mosq){
		mosq->sock = INVALID_SOCKET;
		mosq->sockpairR = INVALID_SOCKET;
		mosq->sockpairW = INVALID_SOCKET;
#ifdef WITH_THREADING
		mosq->thread_id = pthread_self();
#endif
		rc = mosquitto_reinitialise(mosq, id, clean_session, userdata);
		if(rc){
			mosquitto_destroy(mosq);
			if(rc == MOSQ_ERR_INVAL){
				errno = EINVAL;
			}else if(rc == MOSQ_ERR_NOMEM){
				errno = ENOMEM;
			}
			return NULL;
		}
	}else{
		errno = ENOMEM;
	}
	return mosq;
}
```
* 创建一个新的客户端实例
* 参数说明

name|description|
----|-----------|
id|客户端id。如果为NULL，会生成一个随机id。如果为NULL，clean_session必须为 true|
clean_session|如果为true，通知broker在断开连接时清除所有的消息及订阅；如果为false，则通知broker在断开连接时保留消息及订阅。需要注意，客户端在断开时应该永远不丢弃所持有的输出消息 |
userdata|用户指针，传递给回调函数的变量|
返回值|如果成功，返回一个mosquitto结构体；如果失败，可以查询错误码来获得具体原因:ENOMEM，内存溢出；EINVAL，传入了无效的变量

###7、销毁客户端实例
```c
void mosquitto_destroy(struct mosquitto *mosq)
{
	if(!mosq) return;

	_mosquitto_destroy(mosq);
	_mosquitto_free(mosq);
}
```
* 释放客户端实例
* 参数说明

name|description|
---|------------|
mosq|要释放的实例

###8、重新初始化
```c
int mosquitto_reinitialise(struct mosquitto *mosq, const char *id, bool clean_session, void *userdata)
{
	int i;

	if(!mosq) return MOSQ_ERR_INVAL;

	if(clean_session == false && id == NULL){
		return MOSQ_ERR_INVAL;
	}

	_mosquitto_destroy(mosq);
	memset(mosq, 0, sizeof(struct mosquitto));

	if(userdata){
		mosq->userdata = userdata;
	}else{
		mosq->userdata = mosq;
	}
	mosq->protocol = mosq_p_mqtt31;
	mosq->sock = INVALID_SOCKET;
	mosq->sockpairR = INVALID_SOCKET;
	mosq->sockpairW = INVALID_SOCKET;
	mosq->keepalive = 60;
	mosq->message_retry = 20;
	mosq->last_retry_check = 0;
	mosq->clean_session = clean_session;
	if(id){
		if(STREMPTY(id)){
			return MOSQ_ERR_INVAL;
		}
		mosq->id = _mosquitto_strdup(id);
	}else{
		mosq->id = (char *)_mosquitto_calloc(24, sizeof(char));
		if(!mosq->id){
			return MOSQ_ERR_NOMEM;
		}
		mosq->id[0] = 'm';
		mosq->id[1] = 'o';
		mosq->id[2] = 's';
		mosq->id[3] = 'q';
		mosq->id[4] = '/';

		for(i=5; i<23; i++){
			mosq->id[i] = (rand()%73)+48;
		}
	}
	mosq->in_packet.payload = NULL;
	_mosquitto_packet_cleanup(&mosq->in_packet);
	mosq->out_packet = NULL;
	mosq->current_out_packet = NULL;
	mosq->last_msg_in = mosquitto_time();
	mosq->last_msg_out = mosquitto_time();
	mosq->ping_t = 0;
	mosq->last_mid = 0;
	mosq->state = mosq_cs_new;
	mosq->in_messages = NULL;
	mosq->in_messages_last = NULL;
	mosq->out_messages = NULL;
	mosq->out_messages_last = NULL;
	mosq->max_inflight_messages = 20;
	mosq->will = NULL;
	mosq->on_connect = NULL;
	mosq->on_publish = NULL;
	mosq->on_message = NULL;
	mosq->on_subscribe = NULL;
	mosq->on_unsubscribe = NULL;
	mosq->host = NULL;
	mosq->port = 1883;
	mosq->in_callback = false;
	mosq->in_queue_len = 0;
	mosq->out_queue_len = 0;
	mosq->reconnect_delay = 1;
	mosq->reconnect_delay_max = 1;
	mosq->reconnect_exponential_backoff = false;
	mosq->threaded = false;
#ifdef WITH_TLS
	mosq->ssl = NULL;
	mosq->tls_cert_reqs = SSL_VERIFY_PEER;
	mosq->tls_insecure = false;
	mosq->want_write = false;
#endif
#ifdef WITH_THREADING
	pthread_mutex_init(&mosq->callback_mutex, NULL);
	pthread_mutex_init(&mosq->log_callback_mutex, NULL);
	pthread_mutex_init(&mosq->state_mutex, NULL);
	pthread_mutex_init(&mosq->out_packet_mutex, NULL);
	pthread_mutex_init(&mosq->current_out_packet_mutex, NULL);
	pthread_mutex_init(&mosq->msgtime_mutex, NULL);
	pthread_mutex_init(&mosq->in_message_mutex, NULL);
	pthread_mutex_init(&mosq->out_message_mutex, NULL);
	pthread_mutex_init(&mosq->mid_mutex, NULL);
	mosq->thread_id = pthread_self();
#endif

	return MOSQ_ERR_SUCCESS;
}
```
* 复用已有的客户端实例，关闭当前客户端的连接，释放内存，使用新的参数来创建客户端实例，相当于用__mosquitto_new__方法重新的创建一个实例
* 参数说明

name|description|
---|------------|
mosq|有效的客户端实例|
id|参考 mosquitto_new方法
clean_session|参考 mosquitto_new方法
obj|参考 mosquitto_new方法
返回值|成功，返回MOSQ_ERR_SUCCESS；参数无效，返回MOSQ_ERR_INVAL；内存溢出，返回MOSQ_ERR_NOMEM|

###9、设置will
```c
int mosquitto_will_set(struct mosquitto *mosq, const char *topic, int payloadlen, const void *payload, int qos, bool retain)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	return _mosquitto_will_set(mosq, topic, payloadlen, payload, qos, retain);
}
```
* 为客户端设置will。默认情况，客户端没有will，必须在__mosquitto_connect__ 之前调用
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例
topic|发布will的话题
payloadlen|载荷长度，长度在0-268,435,455之间
payload|载荷，如果payloadlen长度大于0，则必须指向一块有效的内存
qos|用于will的服务质量，value=0，1，2
retain|true，令will是一个要保留的消息（？待确定）
返回值|成功，返回MOSQ_ERR_SUCCESS；输入参数无效，返回MOSQ_ERR_INVAL；内存溢出，返回MOSQ_ERR_NOMEM；payloadlen过大，返回MOSQ_ERR_PAYLOAD_SIZE


 <br>
<font color="red" size="5">……</font>  