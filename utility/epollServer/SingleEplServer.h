/*
 * Author	: JiangHongjun
 * Date		: 2016/10/20	
 * Singleton mode
 * You can quickly implement a singleton class for you app.
 * "SingleAPIObject" is a class name and you can chooose you favorite 
 * name replacing that.
 *
 * This is a TCP server
 * */

#pragma once

#include "commoninc.h"

class SingleEplServer
{
public:
	static SingleEplServer& Instance()
	{
		static SingleEplServer theSingleton;
		return theSingleton;
	}
	/*
	 * more non-static functions, including public and private
	 * */
public:
	void description();

	void setArgc(int agc);
	int getArgc();

	void setArgv_0(char *argv_0);
	char* getArgv_0();

	void	setServerPort(char *port);
	char*	getServerPort();

	void	setBrokerPort(unsigned int port);
	unsigned int getBrokerPort();

	void	setBrokerIP(char *ip);
	char*	getBrokerIp();

	void	setBrokerTopic(char *topic);
	char*	getBrokerTopic();
	
	void run();

private:
	int makeSocketNonBlocking(int sfd);
	int createAndBind(char *port);

private:
	SingleEplServer();
	SingleEplServer(SingleEplServer const&);
	SingleEplServer& operator=(SingleEplServer const&);
	~SingleEplServer();

private:
	int					m_argc;
	char				m_argv_0[EXE_FILE_NAME_LENGTH];

	//unsigned int		m_serverPort;
	char				m_serverPort[8];

	unsigned int		m_brokerPort;
	char				m_brokerIP[16];
	char				m_topic[BROKER_TOPIC_LENGTH];

};
