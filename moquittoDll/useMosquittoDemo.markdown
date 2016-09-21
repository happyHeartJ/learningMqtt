#使用mosquitto的broker、pub、sub

##下载
在[mosquitto官网](http://mosquitto.org/download/)下载__Windows__版本的程序，包括broker、pub及sub

##安装
执行exe文件安装

##添加所需的dll文件
运行mosquitto.exe，有可能会需要libeay32.dll、ssleay32.dll、pthreadvc2.dll（可以在[originaldll](http://www.originaldll.com/)下载）。 

执行下列步骤：  
* 将这些文件拷贝到：  __c:\Windows\SysWOW64\__目录中
* 启动“运行”
* 输入下列命令

```
regsvr32 c:\Windows\SysWOW64\pthreadvc2.dll /s

regsvr32 c:\Windows\SysWOW64\libeay32.dll /s

regsvr32 C:\Windows\SysWOW64\ssleay32.dll /s
```

完成上面的步骤即可成功启动 __mosquitto.exe__、__mosquitto_pub.exe__、__mosquitto_sub.exe__。

##测试
* 启动__cmd__，进入mosquitto的安装目录中
* 输入
````
mosquitto_sub.exe -t 'test/topic' -v
````

* 输入
````
mosquitto_pub.exe -t 'test/topic' -m 'hello world'
````
完成以上步骤，即可看到发布/订阅过程。
