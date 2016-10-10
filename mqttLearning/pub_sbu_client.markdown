#mosquitto附带的client分析


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
启动消息循环，这是一个永久阻塞的循环。因为在sub\_client这个demo中，只是一个MQTT的client，所以可以使用__mosquitto_loop_forever__来启动消息循环。
__mosquitto_loop_forever__在线程中调用__mosquitto_loop_forever__来处理数据收发；
__注意：需要打开预编译WITH_THREADING；__
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

##pub_client

与sub_client略有不同，pub_client要处理输入指令，稍微复杂些。

在main函数前，声明了一些全局变量
```c
static char *topic = NULL;
static char *message = NULL;
static long msglen = 0;
static int qos = 0;
static int retain = 0;//default is 0
static int mode = MSGMODE_NONE;
static int status = STATUS_CONNECTING;
static int mid_sent = 0;
static int last_mid = -1;
static int last_mid_sent = -1;
static bool connected = true;
static char *username = NULL;
static char *password = NULL;
static bool disconnect_sent = false;
static bool quiet = false;
```
```c
int main(int argc, char *argv[])
{
	struct mosq_config cfg;
	struct mosquitto *mosq = NULL;
	int rc;
	int rc2;
	char *buf;
	int buf_len = 1024;
	int buf_len_actual;
	int read_len;
	int pos;

	buf = malloc(buf_len);
	if(!buf){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	memset(&cfg, 0, sizeof(struct mosq_config));
	rc = client_config_load(&cfg, CLIENT_PUB, argc, argv);
	if(rc){
		client_config_cleanup(&cfg);
		if(rc == 2){
			/* --help */
			print_usage();
		}else{
			fprintf(stderr, "\nUse 'mosquitto_pub --help' to see usage.\n");
		}
		return 1;
	}
```
创建局部变量，初始化配置参数__cfg__。

```c
	topic = cfg.topic;
	message = cfg.message;
	msglen = cfg.msglen;
	qos = cfg.qos;
	retain = cfg.retain;
	mode = cfg.pub_mode;
	username = cfg.username;
	password = cfg.password;
	quiet = cfg.quiet;
```
使用初始化的参数为全局变量赋值。
```c
	if(cfg.pub_mode == MSGMODE_STDIN_FILE){
		if(load_stdin()){
			fprintf(stderr, "Error loading input from stdin.\n");
			return 1;
		}
	}else if(cfg.file_input){
		if(load_file(cfg.file_input)){
			fprintf(stderr, "Error loading input file \"%s\".\n", cfg.file_input);
			return 1;
		}
	}

	if(!topic || mode == MSGMODE_NONE){
		fprintf(stderr, "Error: Both topic and message must be supplied.\n");
		print_usage();
		return 1;
	}
```
这里会判断当前发送的消息的模式，定义如下，

* define MSGMODE_NONE 0
* define MSGMODE_CMD 1
* define MSGMODE_STDIN_LINE 2
* define MSGMODE_STDIN_FILE 3
* define MSGMODE_FILE 4
* define MSGMODE_NULL 5

此处是用的是__MSGMODE_CMD__，从命令行获取
```c

	mosquitto_lib_init();

	if(client_id_generate(&cfg, "mosqpub")){
		return 1;
	}

	mosq = mosquitto_new(cfg.id, true, NULL);
	if(!mosq){
		switch(errno){
			case ENOMEM:
				if(!quiet) fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				if(!quiet) fprintf(stderr, "Error: Invalid id.\n");
				break;
		}
		mosquitto_lib_cleanup();
		return 1;
	}
	if(cfg.debug){
		mosquitto_log_callback_set(mosq, my_log_callback);
	}
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
	mosquitto_publish_callback_set(mosq, my_publish_callback);

	if(client_opts_set(mosq, &cfg)){
		return 1;
	}
	rc = client_connect(mosq, &cfg);
	if(rc) return rc;
```
这里和sub_client相同。
```c
	if(mode == MSGMODE_STDIN_LINE){
		mosquitto_loop_start(mosq);
	}
```
如果当前是__MSGMODE_STDIN_LINE__模式，则调用__mosquitto_loop_start__开启一个单独的线程来处理网络传输。
```c
	do{
		if(mode == MSGMODE_STDIN_LINE)
		{
			if(status == STATUS_CONNACK_RECVD)
			{
				pos = 0;
				read_len = buf_len;
				while(fgets(&buf[pos], read_len, stdin))
				{
					buf_len_actual = strlen(buf);
					if(buf[buf_len_actual-1] == '\n')
					{
						buf[buf_len_actual-1] = '\0';
						rc2 = mosquitto_publish(mosq, &mid_sent, topic, buf_len_actual-1, buf, qos, retain);
						if(rc2)
						{
							if(!quiet)
							{
							    fprintf(stderr, "Error: Publish returned %d, disconnecting.\n", rc2);
							 }
							mosquitto_disconnect(mosq);
						}
						break;
					}
					else
					{
						buf_len += 1024;
						pos += 1023;
						read_len = 1024;
						buf = realloc(buf, buf_len);
						if(!buf)
						{
							fprintf(stderr, "Error: Out of memory.\n");
							return 1;
						}
					}
				}
				if(feof(stdin))
				{
					last_mid = mid_sent;
					status = STATUS_WAITING;
				}
			}
			else if(status == STATUS_WAITING)
			{
				if(last_mid_sent == last_mid && disconnect_sent == false)
				{
					mosquitto_disconnect(mosq);
					disconnect_sent = true;
				}
#ifdef WIN32
				Sleep(100);
#else
				usleep(100000);
#endif
			}
			rc = MOSQ_ERR_SUCCESS;
		}
		else
		{
			rc = mosquitto_loop(mosq, -1, 1);
		}
	}while(rc == MOSQ_ERR_SUCCESS && connected);
```
这里会根据当前的模式来启动消息循环，由于使用的是__MSGMODE_CMD__模式，所以会调用__mosquitto_loop__来启动循环。当接收到broker发送的CONNACK消息后，调用连接回调函数，回调函数中使用__mosquitto_publish__接口发布消息。  
__mosquitto_loop__使用select方式来处理socket的接收和发送。将__mosquitto__实例中的__sock__放入两个__fd_set__，分别代表读和写。再将__sockpairR__也放入__读fd_set__中。__sockpairR__用来在select()超时前跳出select()，与之对应的有__sockpairW__，将__sockpairW__放入__写fd_set__中。
```c
	if(mode == MSGMODE_STDIN_LINE){
		mosquitto_loop_stop(mosq, false);
	}

	if(message && mode == MSGMODE_FILE){
		free(message);
	}
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	if(rc){
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;
}
```
释放，退出。