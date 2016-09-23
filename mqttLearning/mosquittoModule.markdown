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
mid | 消息id |
topic|话题名称|
payload|负载内容|
payloadlen|负载长度|
qos|服务质量，value=0，1，2
retain|是否保留，value=0，1|

###3、mosquitto_lib_version（版本接口）
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

###4、mosquitto_lib_init（初始化）
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

###5、mosquitto_lib_cleanup（资源回收）
```c
int mosquitto_lib_cleanup(void)
{
	_mosquitto_net_cleanup();

	return MOSQ_ERR_SUCCESS;
}
```
* 释放分配的资源
* 始终返回__MOSQ_ERR_SUCCESS__

###6、mosquitto_new（创建客户端实例）
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

###7、mosquitto_destroy（销毁客户端实例）
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

###8、mosquitto_reinitialise（重新初始化）
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

###9、mosquitto_will_set（设置will）
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

###10、mosquitto_will_clear（清除will）
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

###11、mosquitto_username_pw_set（用户名、密码设置）
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

###12、mosquitto_connect（连接）
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

###13、mosquitto_connect_bind（连接（扩展））
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

###14、mosquitto_connect_async（异步连接）
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

###15、mosquitto_connect_bind_async（异步连接（扩展））
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

###16、mosquitto_reconnect（重新连接）
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

###17、mosquitto_reconnect_async（异步重新连接）
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

###18、mosquitto_disconnect（断开连接）
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

###19、mosquitto_publish（发布消息）
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

###20、mosquitto_subscribe（订阅消息）
```c
int mosquitto_subscribe(struct mosquitto *mosqq, int *mid, const char *sub, int qos)
{
	if(!mosqq) return MOSQ_ERR_INVAL;
	if(mosqq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;

	if(mosquitto_sub_topic_check(sub)) return MOSQ_ERR_INVAL;

	return _mosquitto_send_subscribe(mosqq, mid, sub, qos);
}
```
* 订阅一个topic
* 参数说明

name|description|
---|------------|
mosqq |一个有效的客户端实例|
mid|int指针。<br>如果为NULL，函数将其设定给消息的message id。<br>可以与消息发布后的回调函数一起使用<br>需要注意，虽然MQTT协议没有在QoS=0的消息中使用message id，但libmosquitto为这些消息指定了message id，通过这些参数可以跟踪消息
topic|发布消息的topic
sub |订阅模式
qos |订阅要求的服务质量
返回值|成功，返回MOSQ_ERR_SUCCESS <br>参数无效，返回MOSQ_ERR_INVAL <br>内存溢出，MOSQ_ERR_NOMEM <br>客户端并没有连接到broker，返回MOSQ_ERR_NO_CONN 
 
###21、mosquitto_unsubscribe（取消订阅）
```c
int mosquitto_unsubscribe(struct mosquitto *mosq, int *mid, const char *sub)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	if(mosq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;

	if(mosquitto_sub_topic_check(sub)) return MOSQ_ERR_INVAL;

	return _mosquitto_send_unsubscribe(mosq, mid, sub);
}
```
* 取消订阅
* 参数说明

name|description|
---|------------|
mosq |一个有效的客户端实例|
mid|int指针。<br>如果为NULL，函数将其设定给消息的message id。<br>可以与取消订阅后的回调函数一起使用<br>需要注意，虽然MQTT协议没有在QoS=0的消息中使用message id，但libmosquitto为这些消息指定了message id，通过这些参数可以跟踪消息|
sub|取消定于的模式
返回值|成功，返回MOSQ_ERR_SUCCESS <br>参数无效，返回MOSQ_ERR_INVAL <br>内存溢出，MOSQ_ERR_NOMEM <br>客户端并没有连接到broker，返回MOSQ_ERR_NO_CONN

###22、mosquitto_message_copy（消息拷贝(在message_mosq.c中实现)）
```c
int mosquitto_message_copy(struct mosquitto_message *dst, const struct mosquitto_message *src)
{
	if(!dst || !src) return MOSQ_ERR_INVAL;

	dst->mid = src->mid;
	dst->topic = _mosquitto_strdup(src->topic);
	if(!dst->topic) return MOSQ_ERR_NOMEM;
	dst->qos = src->qos;
	dst->retain = src->retain;
	if(src->payloadlen){
		dst->payload = _mosquitto_malloc(src->payloadlen);
		if(!dst->payload){
			_mosquitto_free(dst->topic);
			return MOSQ_ERR_NOMEM;
		}
		memcpy(dst->payload, src->payload, src->payloadlen);
		dst->payloadlen = src->payloadlen;
	}else{
		dst->payloadlen = 0;
		dst->payload = NULL;
	}
	return MOSQ_ERR_SUCCESS;
}
```
* 拷贝一条消息内容到另一条消息中。可以用于在 __on_message()__ 回调中保持消息。
* 参数说明

name|description|
---|------------|
dst|目的消息
src|源消息
返回值|成功，返回MOSQ_ERR_SUCCESS <br>参数无效，返回MOSQ_ERR_INVAL <br>内存溢出，MOSQ_ERR_NOMEM

###23、mosquitto_message_free（释放消息(在message_mosq.c中实现)）
```c
void mosquitto_message_free(struct mosquitto_message **message)
{
	struct mosquitto_message *msg;

	if(!message || !*message) return;

	msg = *message;

	if(msg->topic) _mosquitto_free(msg->topic);
	if(msg->payload) _mosquitto_free(msg->payload);
	_mosquitto_free(msg);
}
```
* 释放一条mosquitto_message消息
* 参数说明

name|description|
---|------------|
message|指向一条待释放的消息

###24、mosquitto_loop（启动循环）
```c
int mosquitto_loop(struct mosquitto *mosq, int timeout, int max_packets)
{
#ifdef HAVE_PSELECT
	struct timespec local_timeout;
#else
	struct timeval local_timeout;
#endif
	fd_set readfds, writefds;
	int fdcount;
	int rc;
	char pairbuf;
	int maxfd = 0;

	if(!mosq || max_packets < 1) return MOSQ_ERR_INVAL;
#ifndef WIN32
	if(mosq->sock >= FD_SETSIZE || mosq->sockpairR >= FD_SETSIZE){
		return MOSQ_ERR_INVAL;
	}
#endif

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	if(mosq->sock != INVALID_SOCKET){
		maxfd = mosq->sock;
		FD_SET(mosq->sock, &readfds);
		pthread_mutex_lock(&mosq->current_out_packet_mutex);
		pthread_mutex_lock(&mosq->out_packet_mutex);
		if(mosq->out_packet || mosq->current_out_packet){
			FD_SET(mosq->sock, &writefds);
		}
#ifdef WITH_TLS
		if(mosq->ssl){
			if(mosq->want_write){
				FD_SET(mosq->sock, &writefds);
			}else if(mosq->want_connect){
				/* Remove possible FD_SET from above, we don't want to check
				 * for writing if we are still connecting, unless want_write is
				 * definitely set. The presence of outgoing packets does not
				 * matter yet. */
				FD_CLR(mosq->sock, &writefds);
			}
		}
#endif
		pthread_mutex_unlock(&mosq->out_packet_mutex);
		pthread_mutex_unlock(&mosq->current_out_packet_mutex);
	}else{
#ifdef WITH_SRV
		if(mosq->achan){
			pthread_mutex_lock(&mosq->state_mutex);
			if(mosq->state == mosq_cs_connect_srv){
				rc = ares_fds(mosq->achan, &readfds, &writefds);
				if(rc > maxfd){
					maxfd = rc;
				}
			}else{
				pthread_mutex_unlock(&mosq->state_mutex);
				return MOSQ_ERR_NO_CONN;
			}
			pthread_mutex_unlock(&mosq->state_mutex);
		}
#else
		return MOSQ_ERR_NO_CONN;
#endif
	}
	if(mosq->sockpairR != INVALID_SOCKET){
		/* sockpairR is used to break out of select() before the timeout, on a
		 * call to publish() etc. */
		FD_SET(mosq->sockpairR, &readfds);
		if(mosq->sockpairR > maxfd){
			maxfd = mosq->sockpairR;
		}
	}

	if(timeout >= 0){
		local_timeout.tv_sec = timeout/1000;
#ifdef HAVE_PSELECT
		local_timeout.tv_nsec = (timeout-local_timeout.tv_sec*1000)*1e6;
#else
		local_timeout.tv_usec = (timeout-local_timeout.tv_sec*1000)*1000;
#endif
	}else{
		local_timeout.tv_sec = 1;
#ifdef HAVE_PSELECT
		local_timeout.tv_nsec = 0;
#else
		local_timeout.tv_usec = 0;
#endif
	}

#ifdef HAVE_PSELECT
	fdcount = pselect(maxfd+1, &readfds, &writefds, NULL, &local_timeout, NULL);
#else
	fdcount = select(maxfd+1, &readfds, &writefds, NULL, &local_timeout);
#endif
	if(fdcount == -1){
#ifdef WIN32
		errno = WSAGetLastError();
#endif
		if(errno == EINTR){
			return MOSQ_ERR_SUCCESS;
		}else{
			return MOSQ_ERR_ERRNO;
		}
	}else{
		if(mosq->sock != INVALID_SOCKET){
			if(FD_ISSET(mosq->sock, &readfds)){
#ifdef WITH_TLS
				if(mosq->want_connect){
					rc = mosquitto__socket_connect_tls(mosq);
					if(rc) return rc;
				}else
#endif
				{
					do{
						rc = mosquitto_loop_read(mosq, max_packets);
						if(rc || mosq->sock == INVALID_SOCKET){
							return rc;
						}
					}while(SSL_DATA_PENDING(mosq));
				}
			}
			if(mosq->sockpairR != INVALID_SOCKET && FD_ISSET(mosq->sockpairR, &readfds)){
#ifndef WIN32
				if(read(mosq->sockpairR, &pairbuf, 1) == 0){
				}
#else
				recv(mosq->sockpairR, &pairbuf, 1, 0);
#endif
				/* Fake write possible, to stimulate output write even though
				 * we didn't ask for it, because at that point the publish or
				 * other command wasn't present. */
				FD_SET(mosq->sock, &writefds);
			}
			if(FD_ISSET(mosq->sock, &writefds)){
#ifdef WITH_TLS
				if(mosq->want_connect){
					rc = mosquitto__socket_connect_tls(mosq);
					if(rc) return rc;
				}else
#endif
				{
					rc = mosquitto_loop_write(mosq, max_packets);
					if(rc || mosq->sock == INVALID_SOCKET){
						return rc;
					}
				}
			}
		}
#ifdef WITH_SRV
		if(mosq->achan){
			ares_process(mosq->achan, &readfds, &writefds);
		}
#endif
	}
	return mosquitto_loop_misc(mosq);
}
```
* 客户端的网络工作主循环。必须周期性的调用此函数来保证客户端和broker之间的通信。如果流入数据已经就绪，将会被处理。通常会在调用发布消息时立刻将消息发出。此函数会尝试发送任何已经保留的输出消息，包括一部分用于QoS>0的消息流中的命令（？待确定）
* 可以在客户端所拥有的线程中开启循环，使用的方法是__mosquitto_loop_start__
* 此方法使用select方式进行轮询监控socket，如果想要整合mosquitto客户端和自己的select方法，可以是用__mosquitto_socket__，__mosquitto_loop_read__，__mosquitto_loop_write__和__mosquitto_loop_misc__
* 参数说明

name|description|
---|------------|
mosq|客户端实例
timeout|select的超时时间，如果为0，则立刻调用，默认为__1000毫秒__
max_packets|未使用，为了兼容性应该设置为__1__
返回值|成功，返回MOSQ_ERR_SUCCESS<br>参数无效，返回MOSQ_ERR_INVAL<br>内存溢出，返回MOSQ_ERR_NOMEM<br>与broker未连接，返回MOSQ_ERR_NO_CONN<br>连接失效，返回MOSQ_ERR_CONN_LOST<br>与broker通信中，协议错误，返回MOSQ_ERR_PROTOCOL<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###25、mosquitto_loop_forever（启动循环（永久））
```c
int mosquitto_loop_forever(struct mosquitto *mosq, int timeout, int max_packets)
{
	int run = 1;
	int rc;
	unsigned int reconnects = 0;
	unsigned long reconnect_delay;

	if(!mosq) return MOSQ_ERR_INVAL;

	if(mosq->state == mosq_cs_connect_async){
		mosquitto_reconnect(mosq);
	}

	while(run){
		do{
			rc = mosquitto_loop(mosq, timeout, max_packets);
			if (reconnects !=0 && rc == MOSQ_ERR_SUCCESS){
				reconnects = 0;
			}
		}while(run && rc == MOSQ_ERR_SUCCESS);
		/* Quit after fatal errors. */
		switch(rc){
			case MOSQ_ERR_NOMEM:
			case MOSQ_ERR_PROTOCOL:
			case MOSQ_ERR_INVAL:
			case MOSQ_ERR_NOT_FOUND:
			case MOSQ_ERR_TLS:
			case MOSQ_ERR_PAYLOAD_SIZE:
			case MOSQ_ERR_NOT_SUPPORTED:
			case MOSQ_ERR_AUTH:
			case MOSQ_ERR_ACL_DENIED:
			case MOSQ_ERR_UNKNOWN:
			case MOSQ_ERR_EAI:
			case MOSQ_ERR_PROXY:
				return rc;
			case MOSQ_ERR_ERRNO:
				break;
		}
		if(errno == EPROTO){
			return rc;
		}
		do{
			rc = MOSQ_ERR_SUCCESS;
			pthread_mutex_lock(&mosq->state_mutex);
			if(mosq->state == mosq_cs_disconnecting){
				run = 0;
				pthread_mutex_unlock(&mosq->state_mutex);
			}else{
				pthread_mutex_unlock(&mosq->state_mutex);

				if(mosq->reconnect_delay > 0 && mosq->reconnect_exponential_backoff){
					reconnect_delay = mosq->reconnect_delay*reconnects*reconnects;
				}else{
					reconnect_delay = mosq->reconnect_delay;
				}

				if(reconnect_delay > mosq->reconnect_delay_max){
					reconnect_delay = mosq->reconnect_delay_max;
				}else{
					reconnects++;
				}

#ifdef WIN32
				Sleep(reconnect_delay*1000);
#else
				sleep(reconnect_delay);
#endif

				pthread_mutex_lock(&mosq->state_mutex);
				if(mosq->state == mosq_cs_disconnecting){
					run = 0;
					pthread_mutex_unlock(&mosq->state_mutex);
				}else{
					pthread_mutex_unlock(&mosq->state_mutex);
					rc = mosquitto_reconnect(mosq);
				}
			}
		}while(run && rc != MOSQ_ERR_SUCCESS);
	}
	return rc;
}
```
* 当在程序中，仅仅需要启动MQTT客户端时，可以使用此方法。启动一个永久阻塞的循环
* 当连接失效时，会重新连接，调用__mosquitto_disconnect()__可以结束并返回
* 参数说明

name|description|
---|------------|
mosq|客户端实例
timeout|select的超时时间，如果为0，则立刻调用，默认为__1000毫秒__
max_packets|未使用，为了兼容性应该设置为__1__
返回值|成功，返回MOSQ_ERR_SUCCESS<br>参数无效，返回MOSQ_ERR_INVAL<br>内存溢出，返回MOSQ_ERR_NOMEM<br>与broker未连接，返回MOSQ_ERR_NO_CONN<br>连接失效，返回MOSQ_ERR_CONN_LOST<br>与broker通信中，协议错误，返回MOSQ_ERR_PROTOCOL<br>系统调用失败，返回MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###26、mosquitto_loop_start（启动循环（线程，在thread_mosq.c中实现））
```c
int mosquitto_loop_start(struct mosquitto *mosq)
{
#ifdef WITH_THREADING
	if(!mosq || mosq->threaded) return MOSQ_ERR_INVAL;

	mosq->threaded = true;
	pthread_create(&mosq->thread_id, NULL, _mosquitto_thread_main, mosq);
	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;
#endif
}
```
* 每调用一次此函数，将开启一个新的线程去处理网络业务。为用户提供一种重复调用__mosquitto_loop__的替代方法
* 参数说明

name|description|
---|------------|
mosq|客户端实例
返回值|成功，返回MOSQ_ERR_SUCCESS<br>参数无效，返回MOSQ_ERR_INVAL<br>线程不支持，返回MOSQ_ERR_NOT_SUPPORTED

###27、mosquitto_loop_stop（停止循环（在thread_mosq.c中实现））
```c
int mosquitto_loop_stop(struct mosquitto *mosq, bool force)
{
#ifdef WITH_THREADING
#  ifndef WITH_BROKER
	char sockpair_data = 0;
#  endif

	if(!mosq || !mosq->threaded) return MOSQ_ERR_INVAL;


	/* Write a single byte to sockpairW (connected to sockpairR) to break out
	 * of select() if in threaded mode. */
	if(mosq->sockpairW != INVALID_SOCKET){
#ifndef WIN32
		if(write(mosq->sockpairW, &sockpair_data, 1)){
		}
#else
		send(mosq->sockpairW, &sockpair_data, 1, 0);
#endif
	}
	
	if(force){
		pthread_cancel(mosq->thread_id);
	}
	pthread_join(mosq->thread_id, NULL);
	mosq->thread_id = pthread_self();
	mosq->threaded = false;

	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;
#endif
}
```
* 停止使用__mosquitto_loop_start__ 启动的的网络线程。这个函数会阻塞，直到网络线程停止。为了是网络线程终止，必须先调用__mosquitto_disconnect__或者将参数__force__设置为__true__
* 参数说明

name|description|
---|------------|
mosq|客户端实例
force|true，强行结束线程<br>false，必须先调用__mosquitto_disconnect__
返回值|成功，返回MOSQ_ERR_SUCCESS<br>参数无效，返回MOSQ_ERR_INVAL<br>线程不支持，返回MOSQ_ERR_NOT_SUPPORTED

###28、mosquitto_socket
```c
int mosquitto_socket(struct mosquitto *mosq)
{
	if(!mosq) return INVALID_SOCKET;
	return mosq->sock;
}
```
* 返回一个mosquitto实例的socket句柄。如果希望在select调用中包含一个mosquitto实例的话，这个方法很有用
* 参数说明

name|description|
---|------------|
mosq|客户端实例
返回值|成功，返回socket<br>失败，返回-1

###29、mosquitto_loop_read（循环：读）
```c
int mosquitto_loop_read(struct mosquitto *mosq, int max_packets)
{
	int rc;
	int i;
	if(max_packets < 1) return MOSQ_ERR_INVAL;

	pthread_mutex_lock(&mosq->out_message_mutex);
	max_packets = mosq->out_queue_len;
	pthread_mutex_unlock(&mosq->out_message_mutex);

	pthread_mutex_lock(&mosq->in_message_mutex);
	max_packets += mosq->in_queue_len;
	pthread_mutex_unlock(&mosq->in_message_mutex);

	if(max_packets < 1) max_packets = 1;
	/* Queue len here tells us how many messages are awaiting processing and
	 * have QoS > 0. We should try to deal with that many in this loop in order
	 * to keep up. */
	for(i=0; i<max_packets; i++){
#ifdef WITH_SOCKS
		if(mosq->socks5_host){
			rc = mosquitto__socks5_read(mosq);
		}else
#endif
		{
			rc = _mosquitto_packet_read(mosq);
		}
		if(rc || errno == EAGAIN || errno == COMPAT_EWOULDBLOCK){
			return _mosquitto_loop_rc_handle(mosq, rc);
		}
	}
	return rc;
}
```
* 执行网络读的动作
* 仅仅用于不使用__mosquitto_loop()__，并且自己监控客户端socket时
* 参数说明

name|description|
---|------------|
mosq|客户端实例
max_packets|未使用，为了兼容性应该设置为__1__
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>参数无效，返回 MOSQ_ERR_INVAL<br>内存溢出，返回 MOSQ_ERR_NOMEM<br>客户端没有连接broker，返回 MOSQ_ERR_NO_CONN<br>与broker的连接失效，返回 MOSQ_ERR_CONN_LOST<br>与broker的通信中，协议错误，返回 MOSQ_ERR_PROTOCOL<br>系统调用失败，返回 MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###30、mosquitto_loop_write（循环：写）
```c
int mosquitto_loop_write(struct mosquitto *mosq, int max_packets)
{
	int rc;
	int i;
	if(max_packets < 1) return MOSQ_ERR_INVAL;

	pthread_mutex_lock(&mosq->out_message_mutex);
	max_packets = mosq->out_queue_len;
	pthread_mutex_unlock(&mosq->out_message_mutex);

	pthread_mutex_lock(&mosq->in_message_mutex);
	max_packets += mosq->in_queue_len;
	pthread_mutex_unlock(&mosq->in_message_mutex);

	if(max_packets < 1) max_packets = 1;
	/* Queue len here tells us how many messages are awaiting processing and
	 * have QoS > 0. We should try to deal with that many in this loop in order
	 * to keep up. */
	for(i=0; i<max_packets; i++){
		rc = _mosquitto_packet_write(mosq);
		if(rc || errno == EAGAIN || errno == COMPAT_EWOULDBLOCK){
			return _mosquitto_loop_rc_handle(mosq, rc);
		}
	}
	return rc;
}
```
* 执行网络的写操作
* 仅仅用于不使用__mosquitto_loop()__，并且自己监控客户端socket时
* 参数说明

name|description|
---|------------|
mosq|客户端实例
max_packets|未使用，为了兼容性应该设置为__1__
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>参数无效，返回 MOSQ_ERR_INVAL<br>内存溢出，返回 MOSQ_ERR_NOMEM<br>客户端没有连接broker，返回 MOSQ_ERR_NO_CONN<br>与broker的连接失效，返回 MOSQ_ERR_CONN_LOST<br>与broker的通信中，协议错误，返回 MOSQ_ERR_PROTOCOL<br>系统调用失败，返回 MOSQ_ERR_ERRNO，可以查看详细的错误码获得错误信息

###31、mosquitto_loop_misc（循环（混合））
```c
int mosquitto_loop_misc(struct mosquitto *mosq)
{
	time_t now;
	int rc;

	if(!mosq) return MOSQ_ERR_INVAL;
	if(mosq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;

	_mosquitto_check_keepalive(mosq);
	now = mosquitto_time();
	if(mosq->last_retry_check+1 < now){
		_mosquitto_message_retry_check(mosq);
		mosq->last_retry_check = now;
	}
	if(mosq->ping_t && now - mosq->ping_t >= mosq->keepalive){
		/* mosq->ping_t != 0 means we are waiting for a pingresp.
		 * This hasn't happened in the keepalive time so we should disconnect.
		 */
		_mosquitto_socket_close(mosq);
		pthread_mutex_lock(&mosq->state_mutex);
		if(mosq->state == mosq_cs_disconnecting){
			rc = MOSQ_ERR_SUCCESS;
		}else{
			rc = 1;
		}
		pthread_mutex_unlock(&mosq->state_mutex);
		pthread_mutex_lock(&mosq->callback_mutex);
		if(mosq->on_disconnect){
			mosq->in_callback = true;
			mosq->on_disconnect(mosq, mosq->userdata, rc);
			mosq->in_callback = false;
		}
		pthread_mutex_unlock(&mosq->callback_mutex);
		return MOSQ_ERR_CONN_LOST;
	}
	return MOSQ_ERR_SUCCESS;
}
```
* 执行网络需要的各种各样的操作
* 这个函数会处理__PINGs__，检测消息是否需要重发，所以调用很频繁
* 参数说明

name|description|
---|------------|
mosq|客户端实例
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>参数无效，返回 MOSQ_ERR_INVAL<br>客户端没有连接broker，返回 MOSQ_ERR_NO_CONN

###32、mosquitto_want_write
```c
bool mosquitto_want_write(struct mosquitto *mosq)
{
	if(mosq->out_packet || mosq->current_out_packet){
		return true;
#ifdef WITH_TLS
	}else if(mosq->ssl && mosq->want_write){
		return true;
#endif
	}else{
		return false;
	}
}
```
* 如果有数据已经就绪，可以写入socket，返回true
* 参数说明

name|description|
---|------------|
mosq|客户端实例
返回值|如果有数据已经就绪，可以写入socket，返回true

###33、mosquitto_threaded_set（在thread_mosq.c中实现）
```c
int mosquitto_threaded_set(struct mosquitto *mosq, bool threaded)
{
	if(!mosq) return MOSQ_ERR_INVAL;

	mosq->threaded = threaded;

	return MOSQ_ERR_SUCCESS;
}
```
* 通知库使用了多线程，但不是使用__mosquitto_loop_start__开启的
* 如果你自己管理线程，没有是有此函数，会因为资源竞争导致crash
* 参数说明

name|description|
---|------------|
mosq|客户端实例
threaded|如果你的程序使用了多线程，设置为__true__，否则为__false__
返回值|*

###34、mosquitto_opts_set（设置参数）
```c
int mosquitto_opts_set(struct mosquitto *mosq, enum mosq_opt_t option, void *value)
{
	int ival;

	if(!mosq || !value) return MOSQ_ERR_INVAL;

	switch(option){
		case MOSQ_OPT_PROTOCOL_VERSION:
			ival = *((int *)value);
			if(ival == MQTT_PROTOCOL_V31){
				mosq->protocol = mosq_p_mqtt31;
			}else if(ival == MQTT_PROTOCOL_V311){
				mosq->protocol = mosq_p_mqtt311;
			}else{
				return MOSQ_ERR_INVAL;
			}
			break;
		default:
			return MOSQ_ERR_INVAL;
	}
	return MOSQ_ERR_SUCCESS;
}
```
* 设置客户端参数，必须在客户端连接前设置，
* 参数说明

name|description|
---|------------|
mosq|客户端实例
option|设置的参数。必须是int，MQTT_PROTOCOL_V31 或者 MQTT_PROTOCOL_V311<br>默认值为 MQTT_PROTOCOL_V31
value|option指定的值

###35、mosquitto_tls_set
```c
int mosquitto_tls_set(struct mosquitto *mosq, const char *cafile, const char *capath, const char *certfile, const char *keyfile, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata))
{
#ifdef WITH_TLS
	FILE *fptr;

	if(!mosq || (!cafile && !capath) || (certfile && !keyfile) || (!certfile && keyfile)) return MOSQ_ERR_INVAL;

	if(cafile){
		fptr = _mosquitto_fopen(cafile, "rt");
		if(fptr){
			fclose(fptr);
		}else{
			return MOSQ_ERR_INVAL;
		}
		mosq->tls_cafile = _mosquitto_strdup(cafile);

		if(!mosq->tls_cafile){
			return MOSQ_ERR_NOMEM;
		}
	}else if(mosq->tls_cafile){
		_mosquitto_free(mosq->tls_cafile);
		mosq->tls_cafile = NULL;
	}

	if(capath){
		mosq->tls_capath = _mosquitto_strdup(capath);
		if(!mosq->tls_capath){
			return MOSQ_ERR_NOMEM;
		}
	}else if(mosq->tls_capath){
		_mosquitto_free(mosq->tls_capath);
		mosq->tls_capath = NULL;
	}

	if(certfile){
		fptr = _mosquitto_fopen(certfile, "rt");
		if(fptr){
			fclose(fptr);
		}else{
			if(mosq->tls_cafile){
				_mosquitto_free(mosq->tls_cafile);
				mosq->tls_cafile = NULL;
			}
			if(mosq->tls_capath){
				_mosquitto_free(mosq->tls_capath);
				mosq->tls_capath = NULL;
			}
			return MOSQ_ERR_INVAL;
		}
		mosq->tls_certfile = _mosquitto_strdup(certfile);
		if(!mosq->tls_certfile){
			return MOSQ_ERR_NOMEM;
		}
	}else{
		if(mosq->tls_certfile) _mosquitto_free(mosq->tls_certfile);
		mosq->tls_certfile = NULL;
	}

	if(keyfile){
		fptr = _mosquitto_fopen(keyfile, "rt");
		if(fptr){
			fclose(fptr);
		}else{
			if(mosq->tls_cafile){
				_mosquitto_free(mosq->tls_cafile);
				mosq->tls_cafile = NULL;
			}
			if(mosq->tls_capath){
				_mosquitto_free(mosq->tls_capath);
				mosq->tls_capath = NULL;
			}
			if(mosq->tls_certfile){
				_mosquitto_free(mosq->tls_certfile);
				mosq->tls_certfile = NULL;
			}
			return MOSQ_ERR_INVAL;
		}
		mosq->tls_keyfile = _mosquitto_strdup(keyfile);
		if(!mosq->tls_keyfile){
			return MOSQ_ERR_NOMEM;
		}
	}else{
		if(mosq->tls_keyfile) _mosquitto_free(mosq->tls_keyfile);
		mosq->tls_keyfile = NULL;
	}

	mosq->tls_pw_callback = pw_callback;


	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;

#endif
}
```
* 待添加
* 待添加
* 参数说明

name|description|
---|------------|
mosq|客户端实例
cafile|
capath|
certfile|
keyfile|
pw_callback
返回值|

###36、mosquitto_tls_insecure_set
```c
int mosquitto_tls_insecure_set(struct mosquitto *mosq, bool value)
{
#ifdef WITH_TLS
	if(!mosq) return MOSQ_ERR_INVAL;
	mosq->tls_insecure = value;
	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;
#endif
}
```
* 待添加
* 参数说明

name|description|
---|------------|
mosq|客户端实例
value|
返回值|

###37、mosquitto_tls_opts_set
```c
int mosquitto_tls_opts_set(struct mosquitto *mosq, int cert_reqs, const char *tls_version, const char *ciphers)
{
#ifdef WITH_TLS
	if(!mosq) return MOSQ_ERR_INVAL;

	mosq->tls_cert_reqs = cert_reqs;
	if(tls_version){
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
		if(!strcasecmp(tls_version, "tlsv1.2")
				|| !strcasecmp(tls_version, "tlsv1.1")
				|| !strcasecmp(tls_version, "tlsv1")){

			mosq->tls_version = _mosquitto_strdup(tls_version);
			if(!mosq->tls_version) return MOSQ_ERR_NOMEM;
		}else{
			return MOSQ_ERR_INVAL;
		}
#else
		if(!strcasecmp(tls_version, "tlsv1")){
			mosq->tls_version = _mosquitto_strdup(tls_version);
			if(!mosq->tls_version) return MOSQ_ERR_NOMEM;
		}else{
			return MOSQ_ERR_INVAL;
		}
#endif
	}else{
#if OPENSSL_VERSION_NUMBER >= 0x10001000L
		mosq->tls_version = _mosquitto_strdup("tlsv1.2");
#else
		mosq->tls_version = _mosquitto_strdup("tlsv1");
#endif
		if(!mosq->tls_version) return MOSQ_ERR_NOMEM;
	}
	if(ciphers){
		mosq->tls_ciphers = _mosquitto_strdup(ciphers);
		if(!mosq->tls_ciphers) return MOSQ_ERR_NOMEM;
	}else{
		mosq->tls_ciphers = NULL;
	}


	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;

#endif
}
```
* 待添加
* 参数说明

name|description|
---|------------|
mosq|客户端实例
cert_reqs|
tls_version|
ciphers|
返回值|

###38、mosquitto_tls_psk_set
```c
int mosquitto_tls_psk_set(struct mosquitto *mosq, const char *psk, const char *identity, const char *ciphers)
{
#ifdef REAL_WITH_TLS_PSK
	if(!mosq || !psk || !identity) return MOSQ_ERR_INVAL;

	/* Check for hex only digits */
	if(strspn(psk, "0123456789abcdefABCDEF") < strlen(psk)){
		return MOSQ_ERR_INVAL;
	}
	mosq->tls_psk = _mosquitto_strdup(psk);
	if(!mosq->tls_psk) return MOSQ_ERR_NOMEM;

	mosq->tls_psk_identity = _mosquitto_strdup(identity);
	if(!mosq->tls_psk_identity){
		_mosquitto_free(mosq->tls_psk);
		return MOSQ_ERR_NOMEM;
	}
	if(ciphers){
		mosq->tls_ciphers = _mosquitto_strdup(ciphers);
		if(!mosq->tls_ciphers) return MOSQ_ERR_NOMEM;
	}else{
		mosq->tls_ciphers = NULL;
	}

	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;
#endif
}
```
* 待添加
* 参数说明

name|description|
---|------------|
mosq|客户端实例
psk|
identity|
ciphers|
返回值|

###39、mosquitto_connect_callback_set
```c
void mosquitto_connect_callback_set(struct mosquitto *mosq, void (*on_connect)(struct mosquitto *, void *, int))
{
	pthread_mutex_lock(&mosq->callback_mutex);
	mosq->on_connect = on_connect;
	pthread_mutex_unlock(&mosq->callback_mutex);
}
```
* 设置连接回调函数，当broker发送CONNACK作为连接响应时调用
* 参数说明

name|description|
---|------------|
mosq|客户端实例
on_connect|回调函数

* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
rc|连接响应的返回码<br>0 - success<br> 1 - 连接拒绝，因为版本不被接受<br> 2 - 连接拒绝，标识符拒绝<br>3 - 连接拒绝，broker不可用 <br> 4-255 保留

###40、mosquitto_disconnect_callback_set
```c
void mosquitto_disconnect_callback_set(struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int))
{
	pthread_mutex_lock(&mosq->callback_mutex);
	mosq->on_disconnect = on_disconnect;
	pthread_mutex_unlock(&mosq->callback_mutex);
}
```
* 设置断开连接时的回调
* 当broker收到__DISCONNECT__命令，断开客户端时调用
* 参数说明

name|description|
---|------------|
mosq|客户端实例
on_disconnect|回调函数
* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
rc|整型，表明断开的原因。0，表示客户端调用 mosquitto_disconnect <br>其他数值，未知原因的断开

###41、mosquitto_publish_callback_set
```c
void mosquitto_publish_callback_set(struct mosquitto *mosq, void (*on_publish)(struct mosquitto *, void *, int))
{
	pthread_mutex_lock(&mosq->callback_mutex);
	mosq->on_publish = on_publish;
	pthread_mutex_unlock(&mosq->callback_mutex);
}
```
* 设置发布回调函数
* 当消息成功发送到broker后调用
* 参数说明

name|description|
---|------------|
mosq|客户端实例
on_publish|回调函数
* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
mid|发送消息的消息id


###42、mosquitto_message_callback_set
```c
void mosquitto_message_callback_set(struct mosquitto *mosq, void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *))
{
	pthread_mutex_lock(&mosq->callback_mutex);
	mosq->on_message = on_message;
	pthread_mutex_unlock(&mosq->callback_mutex);
}
```
* 设置接收到broker消息时的回调函数
* 参数说明

name|description|
---|------------|
mosq|客户端实例
on_message|回调函数
* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
message|消息数据，在回调函数结束后，这个消息所绑定的内存将被释放。客户端可以根据需要拷贝消息数据

###43、mosquitto_subscribe_callback_set
```c
void mosquitto_subscribe_callback_set(struct mosquitto *mosq, void (*on_subscribe)(struct mosquitto *, void *, int, int, const int *))
{
	pthread_mutex_lock(&mosq->callback_mutex);
	mosq->on_subscribe = on_subscribe;
	pthread_mutex_unlock(&mosq->callback_mutex);
}
```
* 设置订阅回调。当broker对订阅要求响应时触发
* 参数说明

name|description|
---|------------|
mosq|客户端实例
on_subscribe|回调函数
* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
mid|订阅消息的消息id
qos_count|granted_qos的大小，准许订阅的数量
granted_qos|整型数组，表明每条订阅的准许的服务质量

###44、mosquitto_unsubscribe_callback_set
```c
void mosquitto_unsubscribe_callback_set(struct mosquitto *mosq, void (*on_unsubscribe)(struct mosquitto *, void *, int))
{
	pthread_mutex_lock(&mosq->callback_mutex);
	mosq->on_unsubscribe = on_unsubscribe;
	pthread_mutex_unlock(&mosq->callback_mutex);
}
```
* 设置取消订阅的回调函数。当接收到broker对于取消订阅的响应时调用
* 参数说明

name|description|
---|------------|
mosq|mosquitto实例
on_unsubscribe|回调函数
* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
mid|发送消息的消息id

###45、mosquitto_log_callback_set
```c
void mosquitto_log_callback_set(struct mosquitto *mosq, void (*on_log)(struct mosquitto *, void *, int, const char *))
{
	pthread_mutex_lock(&mosq->log_callback_mutex);
	mosq->on_log = on_log;
	pthread_mutex_unlock(&mosq->log_callback_mutex);
}
```
* 设置日志回调函数。当你需要从客户端库中获取日志消息时使用
* 参数说明

name|description|
---|------------|
mosq|mosquitto实例
on_log|回调函数
* 回调函数参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户数据，mosquitto_new 中提供的数据
level|日志级别：<br>MOSQ_LOG_INFO<br>MOSQ_LOG_NOTICE<br>MOSQ_LOG_WARNING<br>MOSQ_LOG_ERR<br>MOSQ_LOG_DEBUG
str|消息内容

###46、mosquitto_reconnect_delay_set
```c
int mosquitto_reconnect_delay_set(struct mosquitto *mosq, unsigned int reconnect_delay, unsigned int reconnect_delay_max, bool reconnect_exponential_backoff)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	
	mosq->reconnect_delay = reconnect_delay;
	mosq->reconnect_delay_max = reconnect_delay_max;
	mosq->reconnect_exponential_backoff = reconnect_exponential_backoff;
	
	return MOSQ_ERR_SUCCESS;
	
}
```
* 当客户端在__mosquitto_loop_forever__中，或者调用__mosquitto_loop_start__后，因为未知的原因断开连接后，会尝试重新连接。默认情况下，每个1秒会尝试连接1次，直到连接成功
* 使用此函数，可以设置尝试连接的时间间隔，可以开启指数补偿来设置尝试连接的延迟以及上限值。例如，

```
Example 1:
	delay=2, delay_max=10, exponential_backoff=False
	Delays would be: 2, 4, 6, 8, 10, 10, ...

Example 2:
	delay=3, delay_max=30, exponential_backoff=True
	Delays would be: 3, 6, 12, 24, 30, 30, ...
```
* 参数说明

name|description|
---|------------|
mosq|mosquitto实例
reconnect_delay|重新连接的间隔时间，单位：秒
reconnect_delay_max|尝试连接的上限时间间隔
reconnect_exponential_backoff|是否开启指数补偿，<br>false，参考Example1<br>true，参考Example2
返回值|成功，返回MOSQ_ERR_SUCCESS<br>参数无效，返回MOSQ_ERR_INVAL

###46、mosquitto_max_inflight_messages_set（在message_mosq.c中实现）
```c
int mosquitto_max_inflight_messages_set(struct mosquitto *mosq, unsigned int max_inflight_messages)
{
	if(!mosq) return MOSQ_ERR_INVAL;

	mosq->max_inflight_messages = max_inflight_messages;

	return MOSQ_ERR_SUCCESS;
}
```
* 表示允许多大数量的QoS为1或2消息被同时进行传输处理。这些消息包括正在进行握手的消息和进行重新发送的消息。默认为20个，
如果设置为0，表示不设限制；如果为1，则会确保消息被顺序处理。
* 参数说明

name|description|
---|------------|
mosq|mosquitto实例
max_inflight_messages|最大并发数量，默认20
返回值|成功，返回 MOSQ_ERR_SUCCESS<br> 参数无效，返回 MOSQ_ERR_INVAL

###47、mosquitto_message_retry_set
```c
void mosquitto_message_retry_set(struct mosquitto *mosq, unsigned int message_retry)
{
	assert(mosq);
	if(mosq) mosq->message_retry = message_retry;
}
```
* 设置重复消息的时间间隔，用于QoS>0的消息。可以在任何时候调用。
* 参数说明

name|description|
---|------------|
mosq|mosquitto实例
message_retry|等待响应的时间，默认是20秒

###48、mosquitto_user_data_set
```c
void mosquitto_user_data_set(struct mosquitto *mosq, void *userdata)
{
	if(mosq){
		mosq->userdata = userdata;
	}
}
```
* 当调用__mosquitto_new__时，会传递参数“obj”，这个指针将会作为用户数据传递给回调函数使用。
* 此函数可以在任何时间更新用户数据
* 此函数不会修改当前用户数据的内存，如果想要动态的分配内存，需要用户自己释放
* 参数说明

name|description|
---|------------|
mosq|mosquitto实例
obj|用户指针，会作为参数传递给任意一个回调函数使用

###49、mosquitto_socks5_set（在socks_mosq.c中实现）
```c
int mosquitto_socks5_set(struct mosquitto *mosq, const char *host, int port, const char *username, const char *password)
{
#ifdef WITH_SOCKS
	if(!mosq) return MOSQ_ERR_INVAL;
	if(!host || strlen(host) > 256) return MOSQ_ERR_INVAL;
	if(port < 1 || port > 65535) return MOSQ_ERR_INVAL;

	if(mosq->socks5_host){
		_mosquitto_free(mosq->socks5_host);
	}

	mosq->socks5_host = _mosquitto_strdup(host);
	if(!mosq->socks5_host){
		return MOSQ_ERR_NOMEM;
	}

	mosq->socks5_port = port;

	if(mosq->socks5_username){
		_mosquitto_free(mosq->socks5_username);
	}
	if(mosq->socks5_password){
		_mosquitto_free(mosq->socks5_password);
	}

	if(username){
		mosq->socks5_username = _mosquitto_strdup(username);
		if(!mosq->socks5_username){
			return MOSQ_ERR_NOMEM;
		}

		if(password){
			mosq->socks5_password = _mosquitto_strdup(password);
			if(!mosq->socks5_password){
				_mosquitto_free(mosq->socks5_username);
				return MOSQ_ERR_NOMEM;
			}
		}
	}

	return MOSQ_ERR_SUCCESS;
#else
	return MOSQ_ERR_NOT_SUPPORTED;
#endif
}
```

* 待添加

###50、mosquitto_strerror
```c
const char *mosquitto_strerror(int mosq_errno)
{
	switch(mosq_errno){
		case MOSQ_ERR_CONN_PENDING:
			return "Connection pending.";
		case MOSQ_ERR_SUCCESS:
			return "No error.";
		case MOSQ_ERR_NOMEM:
			return "Out of memory.";
		case MOSQ_ERR_PROTOCOL:
			return "A network protocol error occurred when communicating with the broker.";
		case MOSQ_ERR_INVAL:
			return "Invalid function arguments provided.";
		case MOSQ_ERR_NO_CONN:
			return "The client is not currently connected.";
		case MOSQ_ERR_CONN_REFUSED:
			return "The connection was refused.";
		case MOSQ_ERR_NOT_FOUND:
			return "Message not found (internal error).";
		case MOSQ_ERR_CONN_LOST:
			return "The connection was lost.";
		case MOSQ_ERR_TLS:
			return "A TLS error occurred.";
		case MOSQ_ERR_PAYLOAD_SIZE:
			return "Payload too large.";
		case MOSQ_ERR_NOT_SUPPORTED:
			return "This feature is not supported.";
		case MOSQ_ERR_AUTH:
			return "Authorisation failed.";
		case MOSQ_ERR_ACL_DENIED:
			return "Access denied by ACL.";
		case MOSQ_ERR_UNKNOWN:
			return "Unknown error.";
		case MOSQ_ERR_ERRNO:
			return strerror(errno);
		case MOSQ_ERR_EAI:
			return "Lookup error.";
		case MOSQ_ERR_PROXY:
			return "Proxy error.";
		default:
			return "Unknown error.";
	}
}
```
* 获取mosquitto错误码
* 参数说明

name|description|
---|------------|
mosq_errno|mosquitto错误码
返回值|描述错误的字符串

###51、mosquitto_connack_string
```c
const char *mosquitto_connack_string(int connack_code)
{
	switch(connack_code){
		case 0:
			return "Connection Accepted.";
		case 1:
			return "Connection Refused: unacceptable protocol version.";
		case 2:
			return "Connection Refused: identifier rejected.";
		case 3:
			return "Connection Refused: broker unavailable.";
		case 4:
			return "Connection Refused: bad user name or password.";
		case 5:
			return "Connection Refused: not authorised.";
		default:
			return "Connection Refused: unknown reason.";
	}
}
```
* 获取描述MQTT连接结果的字符串

name|description|
---|------------|
connack_code|MQTT连接结果
返回值|描述结果的字符串

###52、mosquitto_sub_topic_tokenise
```c
int mosquitto_sub_topic_tokenise(const char *subtopic, char ***topics, int *count)
{
	int len;
	int hier_count = 1;
	int start, stop;
	int hier;
	int tlen;
	int i, j;

	if(!subtopic || !topics || !count) return MOSQ_ERR_INVAL;

	len = strlen(subtopic);

	for(i=0; i<len; i++){
		if(subtopic[i] == '/'){
			if(i > len-1){
				/* Separator at end of line */
			}else{
				hier_count++;
			}
		}
	}

	(*topics) = _mosquitto_calloc(hier_count, sizeof(char *));
	if(!(*topics)) return MOSQ_ERR_NOMEM;

	start = 0;
	stop = 0;
	hier = 0;

	for(i=0; i<len+1; i++){
		if(subtopic[i] == '/' || subtopic[i] == '\0'){
			stop = i;
			if(start != stop){
				tlen = stop-start + 1;
				(*topics)[hier] = _mosquitto_calloc(tlen, sizeof(char));
				if(!(*topics)[hier]){
					for(i=0; i<hier_count; i++){
						if((*topics)[hier]){
							_mosquitto_free((*topics)[hier]);
						}
					}
					_mosquitto_free((*topics));
					return MOSQ_ERR_NOMEM;
				}
				for(j=start; j<stop; j++){
					(*topics)[hier][j-start] = subtopic[j];
				}
			}
			start = i+1;
			hier++;
		}
	}

	*count = hier_count;

	return MOSQ_ERR_SUCCESS;
}
```
* 将topic或者订阅字符串放入数组中来表示层级
* 参数说明

name|description|
---|------------|
subtopic|要分级的subscription/topic
topics|字符串数组，用来保存每一级的内容
count|保存级别数量

例如，
```
For example:

 subtopic: "a/deep/topic/hierarchy"
   Would result in:
   topics[0] = "a"
   topics[1] = "deep"
   topics[2] = "topic"
   topics[3] = "hierarchy"
 
  and:
 
  subtopic: "/a/deep/topic/hierarchy/"
 
  Would result in:
 
  topics[0] = NULL
  topics[1] = "a"
  topics[2] = "deep"
  topics[3] = "topic"
  topics[4] = "hierarchy"
 
  Parameters:
 	subtopic - the subscription/topic to tokenise
 	topics -   a pointer to store the array of strings
 	count -    an int pointer to store the number of items in the topics array.
 
  Returns:
 	MOSQ_ERR_SUCCESS - on success
  	MOSQ_ERR_NOMEM -   if an out of memory condition occurred.
  Example:
 
  > char **topics;
  > int topic_count;
  > int i;
  > 
  > mosquitto_sub_topic_tokenise("$SYS/broker/uptime", &topics, &topic_count);
  >
  > for(i=0; i<token_count; i++){
  >     printf("%d: %s\n", i, topics[i]);
  > }
```

###53、mosquitto_sub_topic_tokens_free
```c
int mosquitto_sub_topic_tokens_free(char ***topics, int count)
{
	int i;

	if(!topics || !(*topics) || count<1) return MOSQ_ERR_INVAL;

	for(i=0; i<count; i++){
		if((*topics)[i]) _mosquitto_free((*topics)[i]);
	}
	_mosquitto_free(*topics);

	return MOSQ_ERR_SUCCESS;
}
```
* 释放__mosquitto_sub_topic_tokenise__分配的内存
* 参数说明

name|description|
---|------------|
topics|字符串数组
count|字符串数组中元素个数
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>参数无效，返回 MOSQ_ERR_INVAL

###54、mosquitto_topic_matches_sub（在util_mosq.c中实现）
```c
int mosquitto_topic_matches_sub(const char *sub, const char *topic, bool *result)
{
	int slen, tlen;
	int spos, tpos;
	bool multilevel_wildcard = false;

	if(!sub || !topic || !result) return MOSQ_ERR_INVAL;

	slen = strlen(sub);
	tlen = strlen(topic);

	if(slen && tlen){
		if((sub[0] == '$' && topic[0] != '$')
				|| (topic[0] == '$' && sub[0] != '$')){

			*result = false;
			return MOSQ_ERR_SUCCESS;
		}
	}

	spos = 0;
	tpos = 0;

	while(spos < slen && tpos < tlen){
		if(sub[spos] == topic[tpos]){
			if(tpos == tlen-1){
				/* Check for e.g. foo matching foo/# */
				if(spos == slen-3 
						&& sub[spos+1] == '/'
						&& sub[spos+2] == '#'){
					*result = true;
					multilevel_wildcard = true;
					return MOSQ_ERR_SUCCESS;
				}
			}
			spos++;
			tpos++;
			if(spos == slen && tpos == tlen){
				*result = true;
				return MOSQ_ERR_SUCCESS;
			}else if(tpos == tlen && spos == slen-1 && sub[spos] == '+'){
				spos++;
				*result = true;
				return MOSQ_ERR_SUCCESS;
			}
		}else{
			if(sub[spos] == '+'){
				spos++;
				while(tpos < tlen && topic[tpos] != '/'){
					tpos++;
				}
				if(tpos == tlen && spos == slen){
					*result = true;
					return MOSQ_ERR_SUCCESS;
				}
			}else if(sub[spos] == '#'){
				multilevel_wildcard = true;
				if(spos+1 != slen){
					*result = false;
					return MOSQ_ERR_SUCCESS;
				}else{
					*result = true;
					return MOSQ_ERR_SUCCESS;
				}
			}else{
				*result = false;
				return MOSQ_ERR_SUCCESS;
			}
		}
	}
	if(multilevel_wildcard == false && (tpos < tlen || spos < slen)){
		*result = false;
	}

	return MOSQ_ERR_SUCCESS;
}
```
* 检测topic是否和订阅匹配
* 参数说明

name|description|
---|------------|
sub|订阅字符串，用于和topic比对
topic|检测的topic
result|true，如果匹配
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>参数无效，返回 MOSQ_ERR_INVAL<br>内存溢出，返回 MOSQ_ERR_NOMEM

例如，
```
 foo/bar would match the subscription foo/# or +/bar
 non/matching would not match the subscription non/+/+
```

###55、mosquitto_pub_topic_check（在util_mosq.c中实现）
```c
int mosquitto_pub_topic_check(const char *str)
{
	int len = 0;
	while(str && str[0]){
		if(str[0] == '+' || str[0] == '#'){
			return MOSQ_ERR_INVAL;
		}
		len++;
		str = &str[1];
	}
	if(len > 65535) return MOSQ_ERR_INVAL;

	return MOSQ_ERR_SUCCESS;
}
```
* 检测要发布的topic是否有效
* 会检测‘+’和‘#’，检测长度
* 在__mosquitto_publish__和__mosquitto_will_set__已经执行了此方法，不需要在它们之前直接调用此方法
* 参数说明

name|description|
---|------------|
topic|需要检测的topic
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>如果包含'+'或者'#'，或者长度过大，返回 MOSQ_ERR_INVAL

###56、mosquitto_sub_topic_check
```c
int mosquitto_sub_topic_check(const char *str)
{
	char c = '\0';
	int len = 0;
	while(str && str[0]){
		if(str[0] == '+'){
			if((c != '\0' && c != '/') || (str[1] != '\0' && str[1] != '/')){
				return MOSQ_ERR_INVAL;
			}
		}else if(str[0] == '#'){
			if((c != '\0' && c != '/')  || str[1] != '\0'){
				return MOSQ_ERR_INVAL;
			}
		}
		len++;
		c = str[0];
		str = &str[1];
	}
	if(len > 65535) return MOSQ_ERR_INVAL;

	return MOSQ_ERR_SUCCESS;
}
```
* 检测一个订阅是否有效
* 会检测‘+’和‘#’，检测它们是否在有效的位置，检测长度,例如

```foo/#/bar, foo/+bar or foo/bar#```
* 在__mosquitto_subscribe__和__mosquitto_unsubscribe__中已经调用，不需要在它们之前直接调用此方法
* 参数说明

name|description|
---|------------|
str|要检测的订阅
返回值|成功，返回 MOSQ_ERR_SUCCESS<br>如果包含'+'或者'#'，或者长度过大，返回 MOSQ_ERR_INVAL

