#config.mk解析
----
User configuration|description|
---|------------|
WITH_WRAP:=yes|是否支持tcpd/libwrap功能
WITH_TLS:=no|是否开启SSL/TLS支持
WITH_TLS_PSK:=no|是否开启TLS/PSK支持
WITH_THREADING:=yes|是否开启多线程支持
WITH_BRIDGE:=yes|是否开启桥接模式
WITH_PERSISTENCE:=yes|是否开启持久化功能
WITH_MEMORY_TRACKING:=yes|是否监控运行状态
WITH_SYS_TREE:=yes|
WITH_SRV:=no|是否开启SRV支持
WITH_UUID:=no|
WITH_WEBSOCKETS:=no|是否开启websocket支持
WITH_EC:=yes|
WITH_DOCS:=yes|
WITH_SOCKS:=yes|  

----
## Mosquitto Conf
###General configuration
General configuration| description|
--------------|------------|
#retry_interval 20|客户端心跳的间隔时间
#sys_interval 10|系统状态的刷新时间|
#store_clean_interval 10|系统资源的回收时间，0表示尽快处理
#pid_file /var/run/mosquitto.pid|服务进程的PID
#user mosquitto|服务进程的系统用户
#max_inflight_messages 10|客户端心跳消息的最大并发数
#max_queued_messages 100|客户端心跳消息缓存队列
#persistent_client_expiration|用于设置客户端长连接的过期时间，默认永不过期

###Default listener
Default listener|description|
----------------|-----------| 
#bind_address|服务绑定的IP地址
#port 1883|服务绑定的端口号
#max_connections -1|允许的最大连接数，-1表示没有限制
# cafile|CA证书文件
# capath|CA证书目录
# certfile|PEM证书文件
# keyfile|PEM密钥文件
#require_certificate false|必须提供证书以保证数据安全性
#use_identity_as_username false|若require_certificate值为true，use_identity_as_username也必须为true
#psk_hint|启用PSK（Pre-shared-key）支持

###Persistence
 
Persistence|description|
----------------|-----------| 
#autosave_interval 1800|消息自动保存的间隔时间
#autosave_on_changes false|消息自动保存功能的开关
persistence true|持久化功能的开关
#persistence_file mosquitto.db|持久化DB文件
#persistence_location /var/lib/mosquitto/|持久化DB文件目录

###Logging
Logging|description|
----------------|-----------| 
log_dest none|4种日志模式：stdout、stderr、syslog、topic.<br>none 则表示不记日志，此配置可以提升些许性能
#log_type error<br>log_type warning<br>log_type notice<br>log_type information|选择日志的级别（可设置多项）
#connection_messages true|是否记录客户端连接信息
#log_timestamp true|是否记录日志时间

###Security
Security|description|
----------------|-----------| 
#clientid_prefixes|客户端ID的前缀限制，可用于保证安全性
#allow_anonymous true|许匿名用户
#password_file|用户/密码文件，默认格式：username:password
#psk_file|PSK格式密码文件，默认格式：identity:key
#acl_file|# ACL权限配置，常用语法如下:

* 用户限制：user <username>
* 话题限制：topic [read|write]<topic>
* 正则限制：pattern write sensor/%u/data

###Bridges

允许服务之间使用“桥接”模式（可用于分布式部署）
* connection \<name\>
* address \<host\>[:\<port\>]
* topic \<topic\> [[[out | in | both] qos-level] local-prefix remote-prefix]

Bridges|description|
----------------|-----------| 
#clientid|设置桥接的客户端ID
#cleansession false|桥接断开时，是否清除远程服务器中的消息
#notifications true|是否发布桥接的状态信息
# $SYS/broker/connection/<clientid>/state<br>#notification_topic|设置桥接模式下，消息将会发布到的话题地址
#keepalive_interval 60|设置桥接的keepalive数值
#start_type automatic|桥接模式，目前有三种：automatic、lazy、once
#restart_timeout 30|桥接模式automatic的超时时间
#idle_timeout 60|桥接模式lazy的超时时间
#username|桥接客户端的用户名
#password|桥接客户端的密码
# bridge_cafile|桥接客户端的CA证书文件
# bridge_capath|桥接客户端的CA证书目录
# bridge_certfile|桥接客户端的PEM证书文件
# bridge_keyfile|桥接客户端的PEM密钥文件

  
###自己的配置可以放到以下目录中
include_dir /etc/mosquitto/conf.d

##启动服务
```
mosquitto -c /etc/mosquitto/mosquitto.conf -d
```
指定配置文件启动mosquitto服务

```
mosquitto
```
直接启动服务

##发布参数
```
mosquitto_pub [-d] [-h hostname] [-i client_id] [-I client id prefix] [-p port number] [-q message QoS] [--quiet] [-r] { -f file | -l | -m message | -n | -s} [-u username [-P password] ] [ --will-topic topic [--will-payload payload] [--will-qos qos] [--will-retain] ] -t message-topic  
```
publish params|description|
--------------|-----------|
-d, --debug|开启debug选项
-f, --file|把一个文件的内容做为消息的内容发送。经测试，支持txt文件，不支持doc等其他形式文件。
-h, --host|说明所连接到的域名，默认是localhost
-i, --id|客户端的ID号，如果没有指定，默认是mosquitto_pub_加上客户端的进程id，不能和--id_prefix同时使用。
-I, --id-prefix|指定客户端ID的前缀，与客户端的进程ID连接组成客户端的ID，不能喝--id同时使用。
-l, --stdin-line|从总段读取输入发送消息，一行为一条消息，空白行不会被发送。
-m, --message|从命令行发送一条消息，-m后面跟发送的消息内容。
-n, --null-message|发送一条空消息。
-p, --port|连接的端口号，默认是1883.
-P, --pw|指定密码用于代理认证，使用此选项时必须有有效的用户名。 
-q, --qos|指定消息的服务质量，可以为0,1,2，默认是0.
--quiet|如果指定该选项，则不会有任何错误被打印，当然，这排除了无效的用户输入所引起的错误消息。
-r, --retain|如果指定该选项，该条消息将被保留做为最后一条收到的消息。下一个订阅消息者将能至少收到该条消息。
-s, --stdin-file|从标准输入接收传输的消息内容，所有输入做为一条消息发送。
-t, --topic|指定消息所发布到哪个主题。
-u, --username|指定用户名用于代理认证。
--will-payload|如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，该选项必须同时用--will-topic指定主题。
--will-qos|指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
--will-retain|如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。必须和选项 --will-topic同时使用.
--will-topic|指定客户端意外断开时，Will消息发送到的主题。

##订阅参数
```
mosquitto_sub [-c] [-d] [-h hostname] [-i client_id] [-I client id prefix] [-k keepalive time] [-p port number] [-q message QoS] [--quiet] [-v] [ -u username [-Ppassword] ] [ --will-topic topic [--will-payload payload] [--will-qos qos] [--will-retain] ] -t message topic 
```
subscribe params|description|
--------------|-----------|
-c, --disable-clean-session|禁止'clean session'选项，即如果客户端断开连接，这个订阅仍然保留来接收随后到的QoS为1和2的消息，当改客户端重新连接之后，它将接收到已排在队列中的消息。建议使用此选项时，客户端id选项设为--id
-d, --debug|开启debug选项
-h, --host|说明所连接到的域名，默认是localhost
-i, --id|客户端的ID号，如果没有指定，默认是mosquitto_pub_加上客户端的进程id，不能和--id_prefix同时使用。
-I, --id-prefix|指定客户端ID的前缀，与客户端的进程ID连接组成客户端的ID，不能喝--id同时使用。
-k, --keepalive|给代理发送PING命令（目的在于告知代理该客户端连接保持且在正常工作）的间隔时间，默认是60s
-p, --port|说明客户端连接到的端口，默认是1883
-P, --pw|指定密码用于代理认证，使用此选项时必须有有效的用户名。 
-q, --qos|指定消息的服务质量，可以为0,1,2，默认是0.
--quiet|如果指定该选项，则不会有任何错误被打印，当然，这排除了无效的用户输入所引起的错误消息。
-t, --topic|指定订阅的消息主题，允许同时订阅到多个主题
-u, --username|指定用户名用于代理认证。
-v, --verbose|冗长地打印收到的消息。若指定该选项，打印消息时前面会打印主题名——“主题 消息内容”，否则，只打印消息内容
--will-payload|如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，该选项必须同时用--will-topic指定主题。
--will-qos|指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
--will-retain|如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。必须和选项 --will-topic同时使用
--will-topic|指定客户端意外断开时，Will消息发送到的主题

##报文种类
* 连接请求（CONNECT）  当一个从客户端到服务器的TCP/IP套接字连接被建立时，必须用一个连接流来创建一个协议级别的会话
* 连接请求确认（CONNECTACK）  连接请求确认报文（CONNECTACK）是服务器发给客户端，用以确认客户端的连接请求 
* 发布报文（PUBLISH） 客户端发布报文到服务器端，用来提供给有着不同需求的订阅者们。每个发布的报文都有一个主题，这是一个分层的命名空间，他定义了报文来源分类，方便订阅者订阅他们需要的主题。订阅者们可以注册自己的需要的报文类别。 
* 发布确认报文（PUBACK） 发布确认报文（PUBACK）是对服务质量级别为1的发布报文的应答。他可以是服务器对发布报文的客户端的报文确认，也可以是报文订阅者对发布报文的服务器的应答。
* 发布确认报文（PUBREC） PUBREC报文是对服务质量级别为2的发布报文的应答。这是服务质量级别为2的协议流的第二个报文。PUBREC是由服务器端对发布报文的客户端的应答，或者是报文订阅者对发布报文的服务器的应答。 
* 发布确认报文（PUBREL) PUBREL是报文发布者对来自服务器的PUBREC报文的确认，或者是服务器对来自报文订阅者的PUBREC报文的确认。它是服务质量级别为2的协议流的第三个报文。 
* 确定发布完成（PUBCOMP） PUBCOMP报文是服务器对报文发布者的PUBREL报文的应答，或者是报文订阅者对服务器的PUBREL报文的应答。它是服务质量级别为2的协议流的第四个也是最后一个报文。 
* 订阅命名的主题（SUBSCRIBE） 订阅报文（SUBSCRIBE）允许一个客户端在服务器上注册一个或多个感兴趣的主题名字。发布给这些主题的报文作为发布报文从服务器端交付给客户端。订阅报文也描述了订阅者想要收到的发布报文的服务质量等级。 
* 订阅报文确认（SUBACK） 当服务器收到客户端发来的订阅报文时，将发送订阅报文的确认报文给客户端。一个这样的确认报文包含一列被授予的服务质量等级。被授予的服务质量等级次序和对应的订阅报文中的主题名称的次序相符。 
* 退订命名的主题(UNSUBSCRIBE) 退订主题的报文是从客户端发往服务器端，用以退订命名的主题。 
* 退订确认（UNSUBACK） 退订确认报文是从服务器发往客户端，用以确认客户端发来的退订请求报文。 
* Ping请求（PINGREQ） Ping请求报文是从连接的客户端发往服务器端，用来询问服务器端是否还存在。 
* Ping应答（PINGRESP） Ping应答报文是从服务器端发往Ping请求的客户端，对客户端的Ping请求进行确认。 
* 断开通知（DISCONNECT） 断开通知报文是从客户端发往服务器端用来指明将要关闭它的TCP/IP连接，他允许彻底地断开，而非只是下线。如果客户端已经和干净会话标志集联系，那么所有先前关于客户端维护的信息将被丢弃。一个服务器在收到断开报文之后，不能依赖客户端关闭TCP/IP连接。