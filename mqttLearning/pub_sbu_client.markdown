#mosquitto附带的client分析

##pub_client


##sub_client
```c
int main(int argc, char *argv[])
{
	struct mosq_config cfg;
	struct mosquitto *mosq = NULL;
	int rc;
	
	rc = client_config_load(&cfg, CLIENT_SUB, argc, argv);
	if(rc){
		client_config_cleanup(&cfg);
		if(rc == 2){
			/* --help */
			print_usage();
		}else{
			fprintf(stderr, "\nUse 'mosquitto_sub --help' to see usage.\n");
		}
		return 1;
	}
```
这部分主要是声明一些程序运行所需的局部变量，调用__client_config_load__来设置配置参数__cfg__。
__client_config_load__会尝试读取本地的配置文件，如果没有会为__cfg__创建参数。


```c
	mosquitto_lib_init();
```
初始化，内部调用___mosquitto_net_init()__来初始化网络环境（包括SRV和TSL等）
```c
	if(client_id_generate(&cfg, "mosqsub")){
		return 1;
	}
```
为__cfg__创建id参数。
```c
	mosq = mosquitto_new(cfg.id, cfg.clean_session, &cfg);
	if(!mosq){
		switch(errno){
			case ENOMEM:
				if(!cfg.quiet) fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				if(!cfg.quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}
	if(client_opts_set(mosq, &cfg)){
		return 1;
	}
```
创建mosquitto客户端。
```c
	if(cfg.debug){
		mosquitto_log_callback_set(mosq, my_log_callback);
		mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	}
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);
```
设置回调函数。

```c
	rc = client_connect(mosq, &cfg);
	if(rc) return rc;
```
启动网络连接。内部调用__mosquitto_connect_bind__来完成连接，在__mosquitto_connect_bind__内部，会继续调用___mosquitto_connect_init__来设置__host__、__port__、__keeplive__等参数。随后，调用___mosquitto_reconnect__来进行连接broker，内部调用___mosquitto_socket_connect__，再深入调用___mosquitto_try_connect__，在这里完成soket的创建，TCP协议指定、地址绑定，连接服务器的操作。
```c
	rc = mosquitto_loop_forever(mosq, -1, 1);
```
启动消息循环，这是一个永久阻塞的循环。因为在sub_client这个demo中，只是一个MQTT的client，所以可以使用__mosquitto_loop_forever__来启动消息循环。
```c
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	if(cfg.msg_count>0 && rc == MOSQ_ERR_NO_CONN){
		rc = 0;
	}
	if(rc){
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;
}
```
释放，退出。