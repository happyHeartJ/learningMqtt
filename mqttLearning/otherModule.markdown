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