#broker源码分析
----
(mosquitto的broker源码位于 __mosquitto-1.4.8/src/mosquitto.c__)

broker中引用了__mosquitto_broker.h__、__memory_mosq.h__、__util_mosq.h__等头文件。

逐行分析，
```c
#ifdef WIN32
typedef SOCKET mosq_sock_t;
#else
typedef int mosq_sock_t;
#endif
```
在进行网络连接时，使用__mosq_sock_t__

```c
struct mqtt3_config config;
```
定义了一个配置参数，其中__mqtt3_config__结构体定义如下
```c
struct mqtt3_config {
	char *config_file;
	char *acl_file;
	bool allow_anonymous;
	bool allow_duplicate_messages;
	bool allow_zero_length_clientid;
	char *auto_id_prefix;
	int auto_id_prefix_len;
	int autosave_interval;
	bool autosave_on_changes;
	char *clientid_prefixes;
	bool connection_messages;
	bool daemon;
	struct _mqtt3_listener default_listener;
	struct _mqtt3_listener *listeners;
	int listener_count;
	int log_dest;
	int log_facility;
	int log_type;
	bool log_timestamp;
	char *log_file;
	FILE *log_fptr;
	uint32_t message_size_limit;
	char *password_file;
	bool persistence;
	char *persistence_location;
	char *persistence_file;
	char *persistence_filepath;
	time_t persistent_client_expiration;
	char *pid_file;
	char *psk_file;
	bool queue_qos0_messages;
	int retry_interval;
	int sys_interval;
	bool upgrade_outgoing_qos;
	char *user;
	bool verbose;
#ifdef WITH_WEBSOCKETS
	int websockets_log_level;
	bool have_websockets_listener;
#endif
#ifdef WITH_BRIDGE
	struct _mqtt3_bridge *bridges;
	int bridge_count;
#endif
	char *auth_plugin;
	struct mosquitto_auth_opt *auth_options;
	int auth_option_count;
};
```
后面将对这个参数进行初始化。

在此之前，先初始化网络服务，调用___mosquitto_net_init__，
```c
void _mosquitto_net_init(void)
{
#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

#ifdef WITH_SRV
	ares_library_init(ARES_LIB_INIT_ALL);
#endif

#ifdef WITH_TLS
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	if(tls_ex_index_mosq == -1){
		tls_ex_index_mosq = SSL_get_ex_new_index(0, "client context", NULL, NULL, NULL);
	}
#endif
}
```
随后，调用__mqtt3_config_init__来初始化__config__。
```c
void mqtt3_config_init(struct mqtt3_config *config)
{
	memset(config, 0, sizeof(struct mqtt3_config));
	_config_init_reload(config);
	config->config_file = NULL;
	config->daemon = false;
	config->default_listener.host = NULL;
	config->default_listener.port = 0;
	config->default_listener.max_connections = -1;
	config->default_listener.mount_point = NULL;
	config->default_listener.socks = NULL;
	config->default_listener.sock_count = 0;
	config->default_listener.client_count = 0;
	config->default_listener.protocol = mp_mqtt;
	config->default_listener.use_username_as_clientid = false;
#ifdef WITH_TLS
	config->default_listener.tls_version = NULL;
	config->default_listener.cafile = NULL;
	config->default_listener.capath = NULL;
	config->default_listener.certfile = NULL;
	config->default_listener.keyfile = NULL;
	config->default_listener.ciphers = NULL;
	config->default_listener.psk_hint = NULL;
	config->default_listener.require_certificate = false;
	config->default_listener.crlfile = NULL;
	config->default_listener.use_identity_as_username = false;
#endif
	config->listeners = NULL;
	config->listener_count = 0;
	config->pid_file = NULL;
	config->user = NULL;
#ifdef WITH_BRIDGE
	config->bridges = NULL;
	config->bridge_count = 0;
#endif
	config->auth_plugin = NULL;
	config->verbose = false;
	config->message_size_limit = 0;
}
```
有一些参数是根据编译时的配置进行处理。默认的参数，由__config_init_reload__来生成。

在这里，会对__default_listener__进行初始化，
```c
struct _mqtt3_listener {
	int fd;
	char *host;
	uint16_t port;
	int max_connections;
	char *mount_point;
	mosq_sock_t *socks;
	int sock_count;
	int client_count;
	enum mosquitto_protocol protocol;
	bool use_username_as_clientid;
#ifdef WITH_TLS
	char *cafile;
	char *capath;
	char *certfile;
	char *keyfile;
	char *ciphers;
	char *psk_hint;
	bool require_certificate;
	SSL_CTX *ssl_ctx;
	char *crlfile;
	bool use_identity_as_username;
	char *tls_version;
#endif
#ifdef WITH_WEBSOCKETS
	struct libwebsocket_context *ws_context;
	char *http_dir;
	struct libwebsocket_protocols *ws_protocol;
#endif
};
```
主要包含了，

|name|description|
----|-----------|
fd|文件描述符
host|主机地址
port|主机端口，默认1883，使用TSL，则为8883
max_connections|最大连接数
mount_point|
