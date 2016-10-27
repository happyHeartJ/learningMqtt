#mosquitto
##安装mosquitto (Linux，已经安装g++)
* 下载源码，可以在[mosquitto](https://github.com/eclipse/mosquitto)的_github_ 上下载
* 解压缩
```
tar zxfv mosquitto-1.4.8.tar.gz
```
* 进入目录
```
cd mosquitto-1.4.8
```
* 编译
```
make
```
* 安装
```
sudo make install
```

注意事项：
* 找不到__openssl/ssl.h__  
```
yum install libssl-devel
```
* 编译过程找不到__ares.h__  
```
修改config.mk中的WITH_SRV:=yes，改为WITH_SRV:=no
```
* 编译过程中找不到__EC_KEY__  
```
将config.mk中的WITH_EC:=yes,改为了WITH_EC：=no
```
* 编译过程中找不到__uuid/uuid.h__  
```
  安装uuid-devel和libuuid-devel  
  
  yum install uuid-devel  
  yum install libuuid-devel
```  
* 执行找不到__libmosquitto.so.1__  
```
  修改libmosquitto.so.1位置，  

  sudo ln -s /usr/local/lib/libmosquitto.so.1 /usr/lib/libmosquitto.so.1  
  sudo ldconfig
```
* 执行找不到__libmosquittopp.so.1__  
```
  修改libmosquittopp.so.1位置，  

  sudo ln -s /usr/local/lib/libmosquittopp.so.1 /usr/lib/libmosquittopp.so.1  
  sudo ldconfig
```
##测试
安装成功后，会生成下列可执行文件：
* mosquitto,作为broker
* mosquitto_sub，作为订阅者
* mosquitto_pub，作为发布者
* mosquitto_passwd，用作管理密码

测试步骤：
* 启动mosquitto  
```
mosquitto -v
```
* 启动订阅者  
```
mosquitto_sub -v -t mqtt_learn
```
* 启动发布者  
```
  mosquitto_pub -v -t mqtt_learn -m "hello mqtt"  
  
  如果要发送的消息中有空格，需用引号括起来
  mosquitto_pub -h localhost -t mqtt_learn -m "hello world",  
  可以用-h来指定broker地址
```
##mosquitto example学习
mosquitto的源码中提供了example（__temperature_conversion__），可以参考学习  
程序包含三个文件：
* main.cpp
* temperature_conversion.h
* temperature_conversion.cpp  

首先，分析下__main.cpp__,
```c
#include "temperature_conversion.h"

int main(int argc, char *argv[])
{
	class mqtt_tempconv *tempconv;
	int rc;

	mosqpp::lib_init();

	tempconv = new mqtt_tempconv("tempconv", "localhost", 1883);
	
	while(1){
		rc = tempconv->loop();
		if(rc){
			tempconv->reconnect();
		}
	}

	mosqpp::lib_cleanup();

	return 0;
}
```
代码比较简单，  
* 创建__mqtt_tempconv__指针
* 调用__mosqpp::lib_init()__来初始化环境
* 为__mqtt_tempconv__分配内存，初始化指定了__id__，__broker地址__，__端口__
* 创建一个循环，在其中启动 __mqtt__ 循环订阅消息，当连接断开时重新连接
* 退出时，清理环境


分析下 __mqtt_tempconv__ 类，  

头文件

```c
#include <mosquittopp.h>

class mqtt_tempconv : public mosqpp::mosquittopp
{
	public:
		mqtt_tempconv(const char *id, const char *host, int port);
		~mqtt_tempconv();

		void on_connect(int rc);
		void on_message(const struct mosquitto_message *message);
		void on_subscribe(int mid, int qos_count, const int *granted_qos);
};

#endif
```
源文件
```c
#include <cstdio>
#include <cstring>

#include "temperature_conversion.h"
#include <mosquittopp.h>

mqtt_tempconv::mqtt_tempconv(const char *id, const char *host, int port) : mosquittopp(id)
{
	int keepalive = 60;

	/* Connect immediately. This could also be done by calling
	 * mqtt_tempconv->connect(). */
	connect(host, port, keepalive);
};

mqtt_tempconv::~mqtt_tempconv()
{
}

void mqtt_tempconv::on_connect(int rc)
{
	printf("Connected with code %d.\n", rc);
	if(rc == 0){
		/* Only attempt to subscribe on a successful connect. */
		subscribe(NULL, "temperature/celsius");
	}
}

void mqtt_tempconv::on_message(const struct mosquitto_message *message)
{
	double temp_celsius, temp_farenheit;
	char buf[51];

	if(!strcmp(message->topic, "temperature/celsius")){
		memset(buf, 0, 51*sizeof(char));
		/* Copy N-1 bytes to ensure always 0 terminated. */
		memcpy(buf, message->payload, 50*sizeof(char));
		temp_celsius = atof(buf);
		temp_farenheit = temp_celsius*9.0/5.0 + 32.0;
		snprintf(buf, 50, "%f", temp_farenheit);
		publish(NULL, "temperature/farenheit", strlen(buf), buf);
	}
}

void mqtt_tempconv::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	printf("Subscription succeeded.\n");
}


```

条理也很清晰，具体看一下使用过程。
* 创建实例时指定__id__，__地址__，__端口__，__保持连接时间__，并启动连接
* 实现了__on_connect__，响应连接成功  
* 实现了__on_message__，响应收到消息  
* 实现了__on_subscribe__，响应订阅成功  


在收到消息后，会对消息进行处理，如果消息的__topic__是__temperature/celsius__，将结果以新的__topic：temperature/farenheit__发布  
