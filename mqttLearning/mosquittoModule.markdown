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
major|如果不为NULL，赋值为major version|
minor|如果不为NULL，赋值为ninor version|
revision|如果不为NULL，赋值为revision version|
返回值|int,返回完整的版本号|

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
返回值|成功，返回一个mosquitto结构体；<br>内存溢出，ENOMEM；<br>无效的变量，返回EINVAL，

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
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>内存溢出，返回MOSQ_ERR_NOMEM|

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
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>输入参数无效，返回MOSQ_ERR_INVAL；<br>内存溢出，返回MOSQ_ERR_NOMEM；<br>payloadlen过大，返回MOSQ_ERR_PAYLOAD_SIZE

###10、清除will
```c
int mosquitto_will_clear(struct mosquitto *mosq)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	return _mosquitto_will_clear(mosq);
}
```
* 清除设置的will，必须在__mosquitto_connect__之前调用
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL

###11、用户名、密码设置
```c
int mosquitto_username_pw_set(struct mosquitto *mosq, const char *username, const char *password)
{
	if(!mosq) return MOSQ_ERR_INVAL;

	if(mosq->username){
		_mosquitto_free(mosq->username);
		mosq->username = NULL;
	}
	if(mosq->password){
		_mosquitto_free(mosq->password);
		mosq->password = NULL;
	}

	if(username){
		mosq->username = _mosquitto_strdup(username);
		if(!mosq->username) return MOSQ_ERR_NOMEM;
		if(password){
			mosq->password = _mosquitto_strdup(password);
			if(!mosq->password){
				_mosquitto_free(mosq->username);
				mosq->username = NULL;
				return MOSQ_ERR_NOMEM;
			}
		}
	}
	return MOSQ_ERR_SUCCESS;
}
```
* 为一个客户端实例配置用户名和密码，仅在broker实现MQTT V3.1协议时使用。默认情况下，不会发送用户名和密码。必须在__mosquitto_connect__之前调用
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例
username|字符串形式的用户名，如果为空，则忽略password，关闭身份验证
password|字符串形式的密码，当置为NULL，并且用户名有效时，仅发送用户名
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>内存溢出，返回MOSQ_ERR_NOMEM

###12、连接
```c
int mosquitto_connect(struct mosquitto *mosq, const char *host, int port, int keepalive)
{
	return mosquitto_connect_bind(mosq, host, port, keepalive, NULL);
}
```
* 连接到broker
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
host|broker的主机名或者ip地址
port|broker的网络端口，通常为1883
keepalive|当没有数据交互时，broker发送PING message的时间间隔
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###13、连接（扩展）
```c
int mosquitto_connect_bind(struct mosquitto *mosq, const char *host, int port, int keepalive, const char *bind_address)
{
	int rc;
	rc = _mosquitto_connect_init(mosq, host, port, keepalive, bind_address);
	if(rc) return rc;

	pthread_mutex_lock(&mosq->state_mutex);
	mosq->state = mosq_cs_new;
	pthread_mutex_unlock(&mosq->state_mutex);

	return _mosquitto_reconnect(mosq, true);
}
```
* 连接到broker，通过添加(bind _ address)变量扩展了__mosquitto_connect__函数。当需要通过特殊的接口来限制网络连接时，使用此函数。
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
host|broker的主机名或者ip地址
port|broker的网络端口，通常为1883
keepalive|当没有数据交互时，broker发送PING message的时间间隔
bind_address|本地网络提供绑定的主机名或ip地址
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###14、异步连接
```c
int mosquitto_connect_async(struct mosquitto *mosq, const char *host, int port, int keepalive)
{
	return mosquitto_connect_bind_async(mosq, host, port, keepalive, NULL);
}
```
* 非阻塞形式连接broker，如果通过异步方式连接broker，客户端必须使用__mosquitto_loop_start__。如果要是用__mosquitto_loop__，则必须使用__mosquitto_connect__
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
host|broker的主机名或者ip地址
port|broker的网络端口，通常为1883
keepalive|当没有数据交互时，broker发送PING message的时间间隔
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###15、异步连接（扩展）
```c
int mosquitto_connect_bind_async(struct mosquitto *mosq, const char *host, int port, int keepalive, const char *bind_address)
{
	int rc = _mosquitto_connect_init(mosq, host, port, keepalive, bind_address);
	if(rc) return rc;

	pthread_mutex_lock(&mosq->state_mutex);
	mosq->state = mosq_cs_connect_async;
	pthread_mutex_unlock(&mosq->state_mutex);

	return _mosquitto_reconnect(mosq, false);
}
```
* 非阻塞形式连接broker，如果通过异步方式连接broker，客户端必须使用__mosquitto_loop_start__。如果要是用__mosquitto_loop__，则必须使用__mosquitto_connect__
* 通过添加(bind _ address)变量扩展了__mosquitto_connect_async__函数。当需要通过特殊的接口来限制网络连接时，使用此函数
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
host|broker的主机名或者ip地址
port|broker的网络端口，通常为1883
keepalive|当没有数据交互时，broker发送PING message的时间间隔
bind_address|本地网络提供绑定的主机名或ip地址
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###16、重新连接
```c
int mosquitto_reconnect(struct mosquitto *mosq)
{
	return _mosquitto_reconnect(mosq, true);
}
```
* 重新连接broker
* 提供了一种简单的方式在连接断开后重新连接broker的函数，使用在__mosquitto_connect__中提供的参数进行连接，不允许在__mosquitto_connect__之前调用
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###17、异步重新连接
```c
int mosquitto_reconnect_async(struct mosquitto *mosq)
{
	return _mosquitto_reconnect(mosq, false);
}
```
* 异步方式重新连接broker
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###18、断开连接
```c
int mosquitto_disconnect(struct mosquitto *mosq)
{
	if(!mosq) return MOSQ_ERR_INVAL;

	pthread_mutex_lock(&mosq->state_mutex);
	mosq->state = mosq_cs_disconnecting;
	pthread_mutex_unlock(&mosq->state_mutex);

	if(mosq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;
	return _mosquitto_send_disconnect(mosq);
}
```
* 断开与broker的连接
* 参数说明


name|description|
---|------------|
mosq|一个有效的客户端实例|
返回值|成功，返回MOSQ_ERR_SUCCESS；<br>参数无效，返回MOSQ_ERR_INVAL；<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###19、发布消息
```c
int mosquitto_publish(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain)
{
	struct mosquitto_message_all *message;
	uint16_t local_mid;
	int queue_status;

	if(!mosq || !topic || qos<0 || qos>2) return MOSQ_ERR_INVAL;
	if(STREMPTY(topic)) return MOSQ_ERR_INVAL;
	if(payloadlen < 0 || payloadlen > MQTT_MAX_PAYLOAD) return MOSQ_ERR_PAYLOAD_SIZE;

	if(mosquitto_pub_topic_check(topic) != MOSQ_ERR_SUCCESS){
		return MOSQ_ERR_INVAL;
	}

	local_mid = _mosquitto_mid_generate(mosq);
	if(mid){
		*mid = local_mid;
	}

	if(qos == 0){
		return _mosquitto_send_publish(mosq, local_mid, topic, payloadlen, payload, qos, retain, false);
	}else{
		message = _mosquitto_calloc(1, sizeof(struct mosquitto_message_all));
		if(!message) return MOSQ_ERR_NOMEM;

		message->next = NULL;
		message->timestamp = mosquitto_time();
		message->msg.mid = local_mid;
		message->msg.topic = _mosquitto_strdup(topic);
		if(!message->msg.topic){
			_mosquitto_message_cleanup(&message);
			return MOSQ_ERR_NOMEM;
		}
		if(payloadlen){
			message->msg.payloadlen = payloadlen;
			message->msg.payload = _mosquitto_malloc(payloadlen*sizeof(uint8_t));
			if(!message->msg.payload){
				_mosquitto_message_cleanup(&message);
				return MOSQ_ERR_NOMEM;
			}
			memcpy(message->msg.payload, payload, payloadlen*sizeof(uint8_t));
		}else{
			message->msg.payloadlen = 0;
			message->msg.payload = NULL;
		}
		message->msg.qos = qos;
		message->msg.retain = retain;
		message->dup = false;

		pthread_mutex_lock(&mosq->out_message_mutex);
		queue_status = _mosquitto_message_queue(mosq, message, mosq_md_out);
		if(queue_status == 0){
			if(qos == 1){
				message->state = mosq_ms_wait_for_puback;
			}else if(qos == 2){
				message->state = mosq_ms_wait_for_pubrec;
			}
			pthread_mutex_unlock(&mosq->out_message_mutex);
			return _mosquitto_send_publish(mosq, message->msg.mid, message->msg.topic, message->msg.payloadlen, message->msg.payload, message->msg.qos, message->msg.retain, message->dup);
		}else{
			message->state = mosq_ms_invalid;
			pthread_mutex_unlock(&mosq->out_message_mutex);
			return MOSQ_ERR_SUCCESS;
		}
	}
}
```
* 在给定topic上发布消息
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
mid|int指针。<br>如果为NULL，函数将其设定给消息的message id。<br>可以与消息发布后的回调函数一起使用<br>需要注意，虽然MQTT协议没有在QoS=0的消息中使用message id，但libmosquitto为这些消息指定了message id，通过这些参数可以跟踪消息
topic|发布消息的topic
payloadlen|载荷长度，长度在0-268,435,455之间
payload|载荷，如果payloadlen长度大于0，则必须指向一块有效的内存
qos|用服务质量，value=0，1，2
retain|true，保留消息（？待确定）
返回值|成功，返回MOSQ_ERR_SUCCESS <br>参数无效，返回MOSQ_ERR_INVAL <br>内存溢出，MOSQ_ERR_NOMEM <br>客户端并没有连接到broker，返回MOSQ_ERR_NO_CONN <br>与broker的通信中协议错误，返回MOSQ_ERR_PROTOCOL <br>payloadlen过大，返回MOSQ_ERR_PAYLOAD_SIZE

###20、订阅消息
```c
int mosquitto_subscribe(struct mosquitto *mosq, int *mid, const char *sub, int qos)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	if(mosq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;

	if(mosquitto_sub_topic_check(sub)) return MOSQ_ERR_INVAL;

	return _mosquitto_send_subscribe(mosq, mid, sub, qos);
}
```
* 订阅一个topic
* 参数说明

name|description|
---|------------|
mosq|一个有效的客户端实例|
mid|int指针。<br>如果为NULL，函数将其设定给消息的message id。<br>可以与消息发布后的回调函数一起使用<br>需要注意，虽然MQTT协议没有在QoS=0的消息中使用message id，但libmosquitto为这些消息指定了message id，通过这些参数可以跟踪消息
topic|发布消息的topic
sub|订阅模式
qos|订阅要求的服务质量
返回值|成功，返回MOSQ_ERR_SUCCESS <br>参数无效，返回MOSQ_ERR_INVAL <br>内存溢出，MOSQ_ERR_NOMEM <br>客户端并没有连接到broker，返回MOSQ_ERR_NO_CONN 
 

 <br>
<font color="red" size="5">……</font>  