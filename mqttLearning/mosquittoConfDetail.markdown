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
设置是否允许匿名登录，格式如下：
```
allow_anonymous [ true | false ]
```
默认值是__true__，允许匿名登录，设为false，不允许匿名登录。当开启身份验证时，要将该项设置为false。

###allow_duplicate_messages
设置是否允许重复发送消息，格式如下：
```
allow_duplicate_messages [ true | false ]
```
当client向broker订阅的topic，彼此有重叠时，例如订阅了foo/#，又订阅了foo/+/baz，MQTT希望当broker接收到匹配topic的消失，仅转发给客户端一次该消息，而不是重复发送。  
该项默认值是__false__，即不允许重复发送。设置为false，允许重复发送。如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/2.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/3.png)  
发布者仅发送了一条消息，订阅者收到了两条消息。

###autosave_interval
设置自动保存内存中的数据至磁盘的时间间隔，格式如下：
```
autosave_interval seconds
```
如果将值设置为0，仅当broker退出或者接收到信号SIGUSR1时才将内存中的数据保存至磁盘。该选项仅在开启持久化功能时可用。
默认值是__1800__秒，即30分钟保存一次。

###autosave_on_changes
自动保存内存中的数据至磁盘的方式，格式为：
```
autosave_on_changes [ true | false ]
```
如果设置为false，那么autosave_interval设定的数值，将作为时间间隔使用。  
如果设置为true，将autosave_interval设定的数值作为消息的数量使用，当订阅数，retain消息数，队列中的消息数总和超过autosave_interval的设定时，会将内存中的消息保存至磁盘。

###clientid_prefixes
broker过滤client的前缀选项，格式为：
```
clientid_prefixes prefix
```
如果设置了此项，只有前缀匹配的client可以连接到broker。如下图所示，
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/4.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/5.png)  
没有使用前缀hyt-作为id的连接被broker拒绝了。添加id再此连接，如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/6.png)  
连接成功。

###connection_messages
设置是否在log中包含client连接和断开的信息。格式如下：
```
connection_messages [ true | false ]
```
设置为true，包含连接和断开信息，设置为false，不包含。

###include_dir
设置以__.conf__为后缀的配置文件目录。格式为：
```
include_dir dir
```
broker将会从此目录中读取配置文件，需要注意的是，mosquitto.conf文件作为主配置文件不应该在此目录中，并且该项最好在mosquitto.conf的结尾处设置。

###log_dest
设置日志的输出目的地，格式为：
```
log_dest destinations
```
可选项有：
* stdout，命令行输出
* stderr，命令行输出，该项为默认值
* file，输出至文件。需要指定一个文件保存日志内容，例如：log_dest file /var/log/mosquitto.log。
* syslog，使用用户空间的系统日志，例如/var/log/messages。日志级别为debug, error, warning, notice, information and message。
* none，关闭日志功能
需要注意的是，当broker作为Windows服务运行时，默认项为none，并且stdout和stderr都不可用。

###log_facility
在非Windows平台，使用syslog作为日志输出时，默认会将消息注册为守护程序。  
设置格式为：
```
log_facility local facility
```
其中，facility的值为0至7。

###log_timestamp
是否开启日志时间戳，格式为：
```
log_timestamp [ true | false ]
```
设置为true，日志会包含时间戳，设置为false，不包含时间戳。

###log_type
设置消息的日志类型，格式为：
log_type types
可选项有：
* debug
* error
* warning
* notice
* information
* subscribe
* unsubscribe
* websockets
* none
* all
默认值是__error，warning， notice和information__。

###max_inflight_messages
设置最大并行处理消息的数量，格式为：
```
max_inflight_messages count
```
允许同时进行传输处理的QoS1和QoS2的消息数量，包括正在进行握手的消息和进行重新发送的消息。默认值是__20__，设置为0，则数量没有限制；设置为1，则确保消息被顺序处理。

###max_queued_messages
设置队列中在排队的QoS1和QoS2消息的数量，格式为：
```
max_queued_messages count
```
默认值是__100__。设置为0，则没有限制（不推荐）。

###message_size_limit
设置发布消息的最大载荷，格式为：
```
message_size_limit limit
```

###password_file
设置格式为：
```
password_file file path
```
安装mosquitto后，在/etc/mosquitto/目录下，会有__pwfile.example__和__acfile.example__。可以参照修改。
另外，安装mosquitto之后，有一个工具__mosquitto_passwd__，用来管理用户名和密码。运行：
```
mosquitto_passwd -c pwfile.01 userName_Alice
```
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/7.png)  
提示输入密码，输入两遍密码，会在文件中对密码加密，如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/8.png)  
文件中的加密结果如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/9.png)  
生成的pwfile.01就是前面提到的password_file文件。想要在pwfile.01中添加密码时，操作少有不同，打开pwfile.01，
```
vim pwfile.01
```
输入新增的用户名和密码，如下图所示，密码此时明文显示，
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/10.png)  
新增的用户名为userName_Bob，密码为12345。保存退出编辑，输入命令：  
```
mosquitto_passwd -U pwfile.01
```
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/11.png)  
再打开pwfile.01查看，如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/12.png)  
此时，明文的密码已经加密了。但是，这种方式会导致前面添加的用户名密码失效。可以使用命令：
```
mosquitto_passwd -b pwfile.01 userName_Bob 12345
```
来为后续的client添加用户名和密码。需要注意的是，添加密码时是明文显示的。  
password_file中添加用户名可以用来控制访问权限。

###persistence
设置格式：
```
persistence [ true | false ]
```
如果设置为true，连接、订阅、消息数据都不会保存在persistence_location指定的路径下的mosquitto.db中。当broker重启后，会重新读取mosquitto.db中的数据。当broker关闭会保存数据，并且以autosave_interval指定的时间为间隔，周期地保存数据。

###persistence_file
设置格式：
```
persistence_file file name
```
持久化文件名称，默认值为__mosquitto.db__。

###persistence_location
设置格式：
```
persistence_location path
```
持久化数据保存的路径。这个路径必须以反斜杠“\”作为结尾。如果不指定的话，会使用当前路径保存。

###persistent_client_expiration
设置格式：
```
persistent_client_expiration duration
```
设置了该项，意味着clean session为false的client，如果没有在一定的时间内重新连接至broker，那么client的持久化信息将被删除。  
该项不是MQTT标准。__截至目前为止，MQTT说明书中，client的持久化都是永久性的__。

###pid_file
设置格式：
```
pid_file file path
```
在指定的目录写入pid文件。默认__没有设置__，不会写入pid文件。如果pid文件不可写，broker将会退出。该项只在broker运行在守护模式时才有作用。  
如果将broker使用初始化脚本自动启动，通常需要写pid文件。文件的路径可以为：/var/run/mosquitto.pid。

###psk_file
设置格式：
```
psk_file file path
```
设置psk文件目录，该项设置需要listener__启用PSK__。如果设置了该项，会使用psk文件中的访问权限来控制client的连接。psk文件中每一行都是identity:key的格式，key是没有0x开头的十六进制数。Client必须提供可以匹配的identity和PSK才能够连接到listener。

###queue_qos0_messages
设置格式：
```
queue_qos0_messages [ true | false ]
```
该项__不是MQTT标准__。设置该项为true，意味着在持久连接的client断开时，是否将QoS0的消息数计算在max_queued_messages参数中。在MQTT V3.1说明书中，仅指定了QoS1和QoS2消息。

###retained_persistence
设置格式：
```
retained_persistence [ true | false ]
```
与persistence选项相同。

###retry_interval
设置格式：
```
retry_interval seconds
```
该项数值必须为整数，表明在QoS1和QoS2消息由broker发出后，等待响应的时间间隔。如果在该时间间隔内，没有收到响应，将会重发消息。如果没设置此项，默认值是__20__。

###store_clean_interval
设置格式：
```
store_clean_interval seconds
```
默认值为__10__。当消息不再被引用至内部存储消息被清理的时间。值越小意味着占用更小的内存，但会需要更多的处理时间，相反，值越大，则需要更多的内存和更少的处理时间。如果设置为0，意味着不再被引用的消息将会尽快的被处理。

###sys_interval
设置格式：
```
sys_interval seconds
```
broker更新$SYS消息的时间间隔。默认值为10，如果设置为0，将会完全禁用发布$SYS消息。

###upgrade_outgoing_qos
设置格式：
```
upgrade_outgoing_qos [ true | false ]
```
目前的MQTT标准说明书中没有该项设置，这__不是标准协议内容__。如果设置为true，broker会将发送给client的消息QoS级别与订阅的级别匹配，即订阅时用QoS=0，1，2，发布给client时的消息级别对应的为0，1，2。MQTT标准规定，使用级别较小的来发布，即订阅级别为2，发布者发布的级别为0，broker发布时会使用0发布。默认值为false。

###user
设置格式：
```
user username
```
当使用root用户运行broker时，会切换使用user指定的用户名的用户，如果无法切换到该用户及组，broker会退出并报错。默认值为__mosquitto__。如果使用非root用户运行，该项没有影响。
在Windows平台，该设置没有影响，你可以使用任何用户运行broker。

##Listeners
通过Listerners配置，可以控制broker的网络端口。  

###bind_address
设置格式：
```
bind_address address
```
服务绑定的地址。可以限制对于某一接口的访问。如果想要限制只有本地的应用可以连接至broker，可以设置“bind_address localhost”，来自其他计算机的连接默认的1883端口将会被拒绝。可以设置listener项来开启新的端口供访问。

###http_dir
设置格式：
```
http_dir directory
```
当一个listener使用websocket协议时，可以提供http服务。设置该项，指定含有提供服务文件的目录。如果没有设置该项，则不可以提供http服务。该目录即提供的连接域名。

###listener
设置格式：
```
listener port [bind address/host]
```
在指定的端口上监听连接。可以选择设置绑定在一个指定的IP地址或主机名上。如果设置了此项，全局设置项bind_address和port均为设置，则不会开启默认的监听（1883端口）。  
此处的bind address/host选项可以使用IP地址和主机名，但对于websocket协议，只可以使用IP地址。下图为使用websocket监听的内容，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/13.png)  
可以使用websockets客户端连接broker的9001端口。

###max_connections
设置格式：
max_connections count
限制当前的listener最大连接数。如果设置为-1，则没有限制。如下图设置，
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/14.png)  
最大连接数为2，测试client连接，如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/15.png)  
共有3个client连接9001端口，图中只有两个成功连接，第三个一直处于发送CONNECT，等待CONNACK的状态。在broker一端，已经显示超过最大连接数据，如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/16.png)  

###mount_point
设置格式：
mount_point topic prefix
添加topic前缀，当client连接了一个使用此项的listener时，所有订阅的topic都会被添加前缀。当消息发布给client时，会移除前缀。意味着，一个client连接到使用了此项的listener时，只能接收到以添加了前缀的topic发布的消息。例如，mount_point设置为example，有clientA向broker订阅topic：mqtt，clientB向broker发布消息，topic：mqtt，如下图所示，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/17.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/18.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/19.png)  
虽然clientA和clientB使用的topic都是mqtt，但是在broker上显示的topic是example/mqtt，是添加了前缀的。

###port
设置格式：
```
port port number
```
设置默认的listener监听端口，默认使用1883。

###protocol
设置格式：
```
protocol value
```
设置listener使用的协议。可选项有：
* mqtt
* default
* websockets
websockets在编译时，默认是没有开启的。除了cafile, certfile, keyfile ciphers之外，基于TLS的身份验证也可以用于websockets。

###use_username_as_clientid
设置格式：
```
use_username_as_clientid [ true | false ]
```
设置为true，client在连接broker的时候，使用username来替代clientid。该项允许将身份验证绑定在clientid上。意味着，可以防止client因为clientid相同而连接失败。默认值是false。

###websockets_log_level
设置格式：
```
websockets_log_level level
```
修改websockets日志级别。这是全局设置项。不能针对单独的listener来设置级别。必须使用log_type websockets项才能使用此项，默认值是__0__。

##Certificate based SSL/TLS Support
基于SSL/TLS的身份验证，可用于所有的Listeners。
###cafile
使用SSL，cafile与capath至少要提供1个。  
设置格式：
```
cafile file path
```
指定PEM编码的CA证书文件。

###capath
使用SSL，cafile与capath至少要提供1个。  
设置格式：
```
capath directory path
```
包含PEM编码的CA证书文件目录，文件必须以.pem作为后缀名。每次添加或者删除一个证书时，使用命令：
```
c_rehash <path to capath>
```

###certfile
设置格式：
```
certfile file path
```
指定PEM证书文件。

###ciphers
设置格式：
```
ciphers cipher:list
```
允许的密码列表，密码之间使用冒号分隔，这就是SSL/TLS的加密算法。可以使用命令：
```
openssl ciphers
```
获得正在使用的密码列表，如下图所示，
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/20.png)  


###crlfile
设置格式：
```
crlfile file path
```
指定一个PEM编码的文件，该文件包含撤销访问权限的client证书列表。使用该项设置，需要将require_certificate设置为true。

###keyfile
设置格式：
```
keyfile file path
```
指定PEM编码的密钥文件。

###require_certificate
设置格式：
```
require_certificate [ true | false ]
```
默认地，SSL/TLS在listener上的工作方式类似于https在web server上的工作方式，server拥有一个CA签名的证书，client会判断这个证书是不是可信的，目的是对网络传输进行加密。将该项设置为true，client必须提供一个有效的证书来进行网络连接。这种机制可以在MQTT协议之外来控制对broker的访问权限。

###tls_version
设置格式：
```
tls_version version
```
设置listener使用的TLS的版本。可选项有：
* tlsv1.2
* tlsv1.1
* tlsv1
如果没设置该项，默认会启动__所有版本__的TLS。

###use_identity_as_username
设置格式：
```
use_identity_as_username [ true | false ]
```
如果已经将require_certificate设置为true，可以将该项也设置为true，来使用client证书中的CN值作为username。
当该项为ture时，listener则不再使用password_file提供的用户名和密码来验证权限。

##Pre-shared-key based SSL/TLS Support
基于SSL/TLS的PSK，可用于所有的Listeners。
###psk_hint
设置格式：
```
psk_hint hint
```
设置该项来启用listener的PSK功能，同时也作为listener的identifier。hint值是开放格式的字符串，本身没有太多含义。hit值会发送给client。

###use_identity_as_username
设置格式：
```
use_identity_as_username [ true | false ]
```
设置此项，将使用client发送的psk identity作为username。username会正常检查，必须使用password_file或者其他的身份验证方式，在没有密码时使用。

##Configuring Bridges
通过配置，可以使用桥接将多个broker连接，构成分布式架构的broker群。
###address
设置格式：
```
address address[:port] [address[:port]], addresses address[:port] [address[:port]]
```
桥接的broker的地址和端口，默认端口为1883。可以指定多个地址和端口。对于每一个桥接都必须设置此项。

###bridge_attempt_unsubscribe
设置格式：
```
bridge_attempt_unsubscribe [ true | false ]
```
默认值为__true__,topic由in转为out时，会发出取消订阅topic到桥接的另一端broker，将此项设置为false，在in和out转换时，不会发送取消订阅的请求。

###bridge_protocol_version
设置格式：
bridge_protocol_version version
设置桥接使用的MQTT协议版本，可以使用
* mqttv31
* mqttv311
默认值为__mqttv31__。

###cleansession
设置格式：
```
cleansession [ true | false ]
```
默认值为__false__，在网络连接失效时，在remote broker上的所有订阅都会被保留。如果设置为true，所有订阅将被清除。  
需要注意的是，如果设置为true，每次在重新建立桥接时，将会发送大量retain消息。  
如果设置为false，如果更改了订阅，那么在接收消息时，有可能会造成意想不到的结果，这是因为remote broker会保留原有的消息订阅。当产生这个问题时，将此项设置为true来建立桥接，再将此项设置为false建立桥接。

###connection
设置格式：
```
connection name
```
桥接的名字。作为在remote broker上显示的client id。

###keepalive_interval
设置格式：
```
keepalive_interval seconds
```
当没有数据传输时，桥接之间的心跳检测时间间隔。默认是__60__，最小值是5。

###idle_timeout
设置格式：
```
idle_timeout seconds
```
当一个桥接使用lazy类型时，在它停止前，必须处于空闲的时间量。默认是__60__。

###local_clientid
设置格式：
```
local_clientid id
```
设置在本地broker上显示的client id。如果没设置，将使用如下格式定义：
```
local.<clientid>
```
如果你建立一个指向自己的桥接，local_clientid和clientid不能相同。

###local_password
设置格式：
```
local_password password
```
当建立一个指向自己的桥接时使用的本地密码。

###local_username
设置格式：
```
local_username username
```
当建立一个指向自己的桥接时使用的用户名。

###notifications
设置格式：
```
notifications [ true | false ]
```
默认值为__true__，向local broker和remote broker发布关于桥接状态的通知消息。如果没设置notification_topics项，retain消息会使用
```
$SYS/broker/connection/<clientid>/state
```
作为topic发布。message为1，表明连接正常，0表示连接失败。

###notification_topic
设置格式：
```
notification_topic topic
```
为桥接通知设置发布的topic。

###remote_clientid
设置格式：
```
remote_clientid id
```
设置在remote broker上显示的id。如果没设置，将使用如下格式定义：
```
name.hostname
```
其中，name是桥接的名字，hostname是主机名。

###remote_password
设置格式：
```
remote_password value
```
配置桥接密码，仅在启用remote_username时有效。当连接到一个使用MQTT V3.1或以上协议的broker，并且该broker开启了用户名/密码验证身份时，需要使用该项配置。

###remote_username
设置格式：
```
remote_username name
```
用法参考remote_password。

###restart_timeout
设置格式：
```
restart_timeout value
```
桥接（使用automatic start类型）重新连接前等待的时间量，默认是__30秒__。

###round_robin
设置格式：
```
round_robin [ true | false ]
```
如果在address/addresses配置项为桥接设置多个地址，round_robin项可以重定义桥接失败时的行为。  
默认为__false__，将第一个地址作为主地址，如果连接失败，会轮流连接后续的地址，同时，也会周期的尝试连接主地址。  
如果设置为true，所有的地址都是相同级别，一个连接失败，会尝试连接下一个。如果连接成功，会保持连接直到连接失效。

###start_type
设置格式：
```
start_type [ automatic | lazy | once ]
```
设置桥接的启动类型。可选项有：
* automatic，默认值，启动时自动连接，如果连接失败，会在30秒后重新连接
* lazy，当队列中的消息数量超过threshold项的值时，会启动连接。在空闲一段时间后（idle_timeout项的值），会自动停止。
* once，启动时尝试连接一次，失败后不会尝试重新连接。

###threshold
设置格式：
```
threshold count
```
启动lazy类型的桥接的队列中消息数量的阈值。默认为__10__。

###topic
设置格式：
```
topic pattern [[[ out | in | both ] qos-level] local-prefix remote-prefix]
```
设置桥接的topic。桥接中的broker，其实相当于彼此订阅的client，需要设置订阅和发布topic。任何符合pattern定义的topic都会在桥接中共享。  
对于local broker，in表示消息来自remote broker，流入local broker；out表示消息来自local broker，流向remote broker；both表示双向均可。默认值为__out__。  
qos-level定义了topic的消息级别，默认值为__0__。  
local-prefix 和 remote-prefix是附加的topic前缀。local broker会向remote broker订阅remote-prefix/topic，当有client发布匹配的消息至remote broker时，remote broker会将消息转发至local broker，桥接会将remote-prefix替换成local-prefix，构成local-prefix/topic，订阅此topic的client会收到消息。同样的,当local broker接收到client发布的消息,发布主题为local-prefix/topic，会将local-prefix/topic替换成remote-prefix/topic转发至remote broker，此时，在remote broker订阅remote-prefix/topic的client可以收到消息。  
local和remote是相对的概念，topic或者消息率先到达的broker即为local broker，转发消息的目的地，即为remote broker。下列图显示了使用桥接的过程。  
配置桥接信息，  
* connection bridge_14_15
* address 192.168.72.14:1883
* cleansession true
* topic # both 0 local/topic/ remote/topic/
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/21.png)  
本地ip地址为192.168.72.14，远端ip为192.268.72.15。  
启动remote broker和local broker，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/22.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/23.png)  
可以看到桥接在loca broker上订阅的topic是local/topic/#，在remote broker上订阅的topic是remote/topic/#。  
启动client在local broker和remote broker上订阅，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/24.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/25.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/26.png)  
启动client，发布消息，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/27.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/28.png)  
桥接已经将prefix进行替换，订阅在remote broker和local broker的client均收到的了消息，在remote broker和local broker上的内容如下，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/29.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/30.png)  
更换topic和ip重新发布消息，  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/31.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/32.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/33.png)  
![mosquitto.conf](https://github.com/happyHeartJ/learningMqtt/blob/master/imgs/mosquitto.conf/34.png)  
local-prefix和remote-prefix均为可选项。空prefix可以使用””替代。  
###try_private
设置格式：
```
try_private [ true | false ]
```
默认值为__true__，意味着桥接不会将broker看作普通的client。

##SSL/TLS Support
对于所有的桥接都可以使用SSL/TLS。
###bridge_cafile
设置格式：
```
bridge_cafile file path
```
开启SSL/TLS支持，bridge_cafile和bridge_capath至少要设置其中一个。
bridge_cafile指定了一个PEM编码的CA证书文件，该证书已经为remote broker签名。

###bridge_capath
设置格式：
```
bridge_capath file path
```
开启SSL/TLS支持，bridge_cafile和bridge_capath至少要设置其中一个。
指定一个目录，目录中包含PEM编码的CA证书文件，并且已经为remote broker签名。文件必须以”.crt”结尾。每次添加或删除一个证书时，使用命令：
```
c_rehash <path to bridge_capath>
```

###bridge_certfile
设置格式：
```
bridge_certfile file path
```
用于桥接的PEM编码的client证书文件目录。

###bridge_identity
设置格式：
```
bridge_identity identity
```
使用PSK加密的client identity。对于一个桥接，一次仅能使用1个PSK加密的证书文件。

###bridge_insecure
设置格式：
```
bridge_insecure [ true | false ]
```
设置为false，禁止主机名验证。设置为true，有可能会遭到第三方的恶意攻击。在生产环境中，需要将此项设置为false。

###bridge_keyfile
设置格式：
```
bridge_keyfile file path
```
指定目录，目录中包含用于桥接的PEM编码的key。

###bridge_psk
设置格式：
```
bridge_psk key
```
PEM编码的十六进制格式key文件。

###bridge_tls_version
设置格式：
```
bridge_tls_version version
```
设置桥接使用的TLS版本。可选项有：
* tlsv1.2，默认值
* tlsv1.1 
* tlsv1
remote broker必须支持相同的TLS版本才能连接成功。

----
部分内容翻译的较为生硬，有些理解也不完全正确，暂且定为目标，后续有时间再修改
(That's all)