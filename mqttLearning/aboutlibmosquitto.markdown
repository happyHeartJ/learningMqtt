#lib文件说明
* logging_mosq.h、logging_mosq.c ： 日志打印输出，如果设置了on_log回调，则会在打印日志的时候调用回调
* memory_mosq.h、memory_mosq.c：对内存分配、释放等函数进行了封装，在内存分配的同时，可以进行分配空间的记录
* messages_mosq.h、messages_mosq.c：对消息的操作，包括清除所有消息、消息复制、消息删除、消息状态重置（重新连接时）、消息重发等
* mosquitt.h、mosquitto.c：参考[mosquitto模块分析](https://github.com/happyHeartJ/learningMqtt/blob/master/mqttLearning/mosquittoModule.markdown)
* mqtt3_protocol.h：控制报文宏定义
* net_mosq.h、net_mosq.c：网络、数据包层面的功能封装，包括网络环境的初始化、资源回收，连接、断开，数据收发，MQTT数据解析（读、写；单字节、多字节的读取，字节长度的读写）
* read_handle.h、read_handle.c、read_handle_client.c、read_handle_shared.c：处理接收的控制报文
* send_mosq.h、send_mosq.c、send_client_mosq.c：发送控制报文
* util_mosq.h、util_mosq.c：检测keepalive、生成mid
* will_mosq.h、will_mosq.c：设置和清理will
* socks_mosq：sock5代理功能实现
* srv_mosq.c：SRV实现
* thread_mosq.c：多线程实现
* time_mosq.c：获取事件
* tls_mosq.c：TLS相关实现