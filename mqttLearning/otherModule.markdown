#mosquitto模块分析
##概述
__libmosquitto__ 作为 __mosquitto__ 开源代码的一部分，主要用来实现 __MQTT__ 协议栈和数据包通讯功能。
下面将对__libmosquitto__包含的模块进行简要介绍（__mosquitto__模块参考[mosquitto模块](https://github.com/happyHeartJ/learningMqtt/blob/master/mqttLearning/mosquittoModule.markdown)）。

----
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

##主要数据结构
* 消息方向枚举值

```c
enum mosquitto_msg_direction {
	mosq_md_in = 0,// 流 入
	mosq_md_out = 1// 流 出
};
```
* 消息状态枚举值

```c
enum mosquitto_msg_state {
	mosq_ms_invalid = 0,        
	mosq_ms_publish_qos0 = 1,    
	mosq_ms_publish_qos1 = 2,    
	mosq_ms_wait_for_puback = 3,    //等待puback
	mosq_ms_publish_qos2 = 4,        
	mosq_ms_wait_for_pubrec = 5,    //等待pubrec
	mosq_ms_resend_pubrel = 6,        
	mosq_ms_wait_for_pubrel = 7,      //等待pubrel
	mosq_ms_resend_pubcomp = 8,        
	mosq_ms_wait_for_pubcomp = 9,        //等待pubcomp
	mosq_ms_send_pubrec = 10,           
	mosq_ms_queued = 11                    
};
```
* 客户端状态，用户连接成功，并且发送CONNECT之后的结果


```c
enum mosquitto_client_state {

    mosq_cs_new = 0,

    mosq_cs_connected = 1,

    mosq_cs_disconnecting = 2,// mosquitto_disconnect时设置

    mosq_cs_connect_async = 3,// mosquitto_connect_bind_async，异步线程来connect _mosquitto_thread_main（需要WITH_THREADING）

    mosq_cs_connect_pending = 4//没用到

};
```
* 消息状态，响应的状态表达为__mosq_ms_wait_for_xxxx__，客户端处理此类消息，消息的收发流程使用


```c
enum mosquitto_msg_state {。

    mosq_ms_invalid = 0,        //消 息 无 效

    mosq_ms_publish_qos0 = 1,    //发布qos0

    mosq_ms_publish_qos1 = 2,    //发布qos1

    mosq_ms_wait_for_puback = 3,//Oos==1时，发送PUBLISH后等待PUBACK返回

    mosq_ms_publish_qos2 = 4,    //发布qos2

    mosq_ms_wait_for_pubrec = 5, //Oos==2时，发送PUBLISH后，等待PUBREC返回

    mosq_ms_resend_pubrel = 6,    //重发pubrel

    mosq_ms_wait_for_pubrel = 7, //Oos==2时，发送PUBREC后等待PUBREL返回

    mosq_ms_resend_pubcomp = 8,    //重发pubcomp

    mosq_ms_wait_for_pubcomp = 9, //Oos==2时，发送PUBREL后等待PUBCOMP返回

    mosq_ms_send_pubrec = 10,     //发布pubrec

    mosq_ms_queued = 11            //排队状态

};
```
* 协议枚举值

```c
enum _mosquitto_protocol {
	mosq_p_invalid = 0,
	mosq_p_mqtt31 = 1,
	mosq_p_mqtt311 = 2,
	mosq_p_mqtts = 3
}
```

* 传输协议枚举值

```c
enum _mosquitto_transport {
	mosq_t_invalid = 0,
	mosq_t_tcp = 1,
	mosq_t_ws = 2,    //websockets
	mosq_t_sctp = 3
};
```
* 数据包，发送的数据在组包之后，或者接收的数据在解包之前的状态

```c
struct _mosquitto_packet{

    uint8_t *payload;

    struct _mosquitto_packet *next;

    uint32_t remaining_mult;

    uint32_t remaining_length;

    uint32_t packet_length;

    uint32_t to_process;//发送进度，记录还未发送多少字节，缺省为packet_length

    uint32_t pos;//组包或者发送时用到，发送时记录发送到什么位置

    uint16_t mid;//消息id，当Qos==0 时回调on_publish时用

    uint8_t command;

    int8_t remaining_count;

};
```
* 消息

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
* 消息队列

```c
struct mosquitto_message_all{

    struct mosquitto_message_all *next;

    time_t timestamp;//时间，记录本地软件tick时间

    //enum mosquitto_msg_direction direction;

    enum mosquitto_msg_state state;

    bool dup;

    struct mosquitto_message msg;

};
```
* 上下文会话属性


```c
struct mosquitto {

    mosq_sock_t sock;

    // socket管道通知：非阻塞模式时
    //通知用，在mosquitto_loop 调用发送,
    mosq_sock_t sockpairR, sockpairW;

    enum _mosquitto_protocol protocol;

    char *address;

    char *id;//客户端ID

    char *username;

    char *password;

    uint16_t keepalive;

    uint16_t last_mid;  //最后一个消息id，发消息后++

    enum mosquitto_client_state state;

    time_t last_msg_in;

    time_t last_msg_out;

    time_t ping_t;

    struct _mosquitto_packet in_packet;//接收数据包用

    struct _mosquitto_packet *current_out_packet;

    struct _mosquitto_packet *out_packet;//发送数据包队列

    struct mosquitto_message *will;

#ifdef WITH_TLS

    SSL *ssl;

    SSL_CTX *ssl_ctx;

    char *tls_cafile;

    char *tls_capath;

    char *tls_certfile;

    char *tls_keyfile;

    int (*tls_pw_callback)(char *buf, int size, int rwflag, void *userdata);

    char *tls_version;

    char *tls_ciphers;

    char *tls_psk;

    char *tls_psk_identity;

    int tls_cert_reqs;

    bool tls_insecure;

#endif

    bool want_write;

    bool want_connect;

#if defined(WITH_THREADING) && !defined(WITH_BROKER)

    pthread_mutex_t callback_mutex;

    pthread_mutex_t log_callback_mutex;

    pthread_mutex_t msgtime_mutex;

    pthread_mutex_t out_packet_mutex;

    pthread_mutex_t current_out_packet_mutex;

    pthread_mutex_t state_mutex;

    pthread_mutex_t in_message_mutex;

    pthread_mutex_t out_message_mutex;

    pthread_mutex_t mid_mutex;

    pthread_t thread_id;

#endif

    bool clean_session;

 

    void *userdata;

    bool in_callback;

    unsigned int message_retry;

    time_t last_retry_check;

    struct mosquitto_message_all *in_messages;//收到消息队列

    struct mosquitto_message_all *in_messages_last;

    struct mosquitto_message_all *out_messages;发送消息队列

    struct mosquitto_message_all *out_messages_last;

 

    void (*on_connect)(struct mosquitto *, void *userdata, int rc);

    void (*on_disconnect)(struct mosquitto *, void *userdata, int rc);

    void (*on_publish)(struct mosquitto *, void *userdata, int mid);

    void (*on_message)(struct mosquitto *, void *userdata, const struct mosquitto_message *message);

    void (*on_subscribe)(struct mosquitto *, void *userdata, int mid, int qos_count, const int *granted_qos);

    void (*on_unsubscribe)(struct mosquitto *, void *userdata, int mid);

    void (*on_log)(struct mosquitto *, void *userdata, int level, const char *str);

    //void (*on_error)();

    char *host;

    int port;

    int in_queue_len;  //收到消息队列长度

    int out_queue_len;//发送消息队列长度

    char *bind_address;

    unsigned int reconnect_delay;

    unsigned int reconnect_delay_max;

    bool reconnect_exponential_backoff;

    bool threaded;

    int inflight_messages; //对于Qos>0的消息，记录没有完成交互记录

    int max_inflight_messages;

};
```

  <br>
<font color="red" size="5">代码详细注解待添加</font>  



#memory_mosq模块
* 内存分配处理，可记录内存用量

#net_mosq模块
* 网络基础操作，tcp创建，关闭等
* 打包/解包数据，向_mosquitto_packet中写入/读取各种数据

#send_mosq模块
* 主要实现发送请求逻辑（协议组包），实际命令请求实现组包

#send_client_mosq模块
* 与send_mosq类似，主要实现客户端常用高频使用接口；其他接口在send_mosq中

#messages_mosq模块
* 主要针对消息的实现(PUBLISH,PUBACK,PUBREL..)

#read_handle
* 处理收到的数据包，根据数据包类型做相应处理

----