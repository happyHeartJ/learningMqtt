#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#ifdef WIN32

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32")

#else// LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#endif

using namespace std;

#define KEYVALLEN 100

#define MAXEVENTS 64


#define EXE_FILE_NAME_LENGTH	256
#define BROKER_TOPIC_LENGTH		256