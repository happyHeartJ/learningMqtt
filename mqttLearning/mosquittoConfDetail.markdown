#mosquitto.conf详细说明
__mosquitto.conf__是mosquitto broker的配置文件，在启动配置broker时，可以通过参数__“-c”__来选择加载__mosquitto.conf__来使用文件中的配置。默认地，broker不需要加载mosquitto.conf，会使用默认参数。  

配置文件中，在每一行的起始位置使用 __“#”__ 作为注释标记。配置项与值之间使用一个空格分隔。  

##Authentication
配置Authentication部分可以控制对broker的访问权限。默认地，没有给出Authentication配置参数。mosquitto broker提供了三种身份验证方式：用户名/密码，certificate，pre-shared-key。  

MQTT协议本身提供了用户名/密码的身份验证。使用password_file文件可以定义用户名和密码。使用这种方式要确保使用了网络加密，以防止用户名/密码被中途拦截，遭到攻击。  

使用加密的certificate方式来提供身份验证，需要开启require_certificate选项，将此项设置为true，客户端必须提供一个有效的certificate才可以成功连接broker。除此之外，有另一个选项也可以提供验证：use_identity_as_username。将use_identity_as_username设置为true，会使用客户端certificate中的Common Name作为MQTT的用户名，以此作为访问控制的验证。  

使用pre-shared-key需要开启psk_hint 和 psk_file options选项。客户端必须提供有效的id和key才可以成功连接broker。如果设置use_identity_as_username为true，那么会使用PSK id来代替MQTT用户名，以此来验证身份。  

###acl_file
设置acl_file的路径，格式如下：
```
acl_file file path
```
acl_file的内容包含client可以访问的topic。acl_file限定topic访问权限的格式为：
```
topic [read|write|readwrite] <topic>
```

默认值为__read/write__，topic可以包含过滤器限定符__“+”__和__“#”__。
	当allow_anonymous设置为true时，以上的设置是针对所有的匿名登录的client。如果allow_anonymous设置为false，将不允许匿名登录。可以使用格式为：
user <username>
来限制具体的用户对topic的访问，这里的用户名是在password_file文件中定义的，并不是client id。此时，使用格式为：
pattern [read|write|readwrite] <topic>
来控制topic的read、write权限。在topic中可以使用
* %c来匹配client id
* %u来匹配client username

例如，pattern write sensor/%u/data。
安装mosquitto后，在/etc/mosquitto/目录下，会有acfile.example。可以参照修改。内容如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/1.png)
 

###allow_anonymous
###allow_duplicate_messages
###autosave_interval
###autosave_on_changes
###clientid_prefixes
###connection_messages
###include_dir
###log_dest
###log_facility
###log_timestamp
###log_type
###max_inflight_messages
###max_queued_messages
###message_size_limit
###password_file
###persistence
###persistence_file
###persistence_location
###persistent_client_expiration
###pid_file
###psk_file
###queue_qos0_messages
###retained_persistence
###retry_interval
###store_clean_interval
###sys_interval
###upgrade_outgoing_qos
###user
##Listeners
通过Listerners配置，可以控制broker的网络端口。
###bind_address
###http_dir
###listener
###max_connections
###mount_point
###port
###protocol
###use_username_as_clientid
###websockets_log_level
##Certificate based SSL/TLS Support
基于SSL/TLS的身份验证，可用于所有的Listeners。
###cafile
###capath
###certfile
###ciphers
###crlfile
###keyfile
###require_certificate
###tls_version
###use_identity_as_username
##Pre-shared-key based SSL/TLS Support
基于SSL/TLS的PSK，可用于所有的Listeners。
###ciphers
###psk_hint
###tls_version
###use_identity_as_username
##Configuring Bridges
通过配置，可以使用桥接将多个broker连接，构成分布式架构的broker群。
###address
###bridge_attempt_unsubscribe
###bridge_protocol_version
###cleansession
###connection
###keepalive_interval
###idle_timeout
###local_clientid
###local_password
###local_username
###notifications
###notification_topic
###remote_clientid
###remote_password
###remote_username
###restart_timeout
###round_robin
###start_type
###threshold
###topic
###try_private
##SSL/TLS Support
对于所有的桥接都可以使用SSL/TLS。
###bridge_attempt_unsubscribe
###bridge_cafile
###bridge_capath
###bridge_certfile
###bridge_identity
###bridge_insecure
###bridge_keyfile
###bridge_psk
###bridge_tls_version
