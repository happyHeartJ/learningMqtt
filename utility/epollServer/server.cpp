#include "SingleEplServer.h"
#include "ParseConfig.h"


int main(int argc, char *argv[])
{
	ParseConfig pcf;
	pcf.description();
	char localPort[8] = {0};
	char brokerPort[8] = {0};
	char brokerIP[16] = {0};
	char topic[256] = {0};
	pcf.GetProfileString("./cf.conf","SingleEplServer","localPort",localPort);
	pcf.GetProfileString("./cf.conf","SingleEplServer","brokerPort",brokerPort);
	pcf.GetProfileString("./cf.conf","SingleEplServer","brokerIP",brokerIP);
	pcf.GetProfileString("./cf.conf","SingleEplServer","topic",topic);
	
	printf("local port is %s\n",localPort);
	printf("broker port is %s\n",brokerPort);
	printf("broker ip is %s\n",brokerIP);
	printf("topic is %s\n",topic);

	SingleEplServer::Instance().description();
	SingleEplServer::Instance().setArgc(2);
	SingleEplServer::Instance().setServerPort(localPort);
	SingleEplServer::Instance().setBrokerPort(atoi(brokerPort));
	SingleEplServer::Instance().setBrokerIP(brokerIP);
	SingleEplServer::Instance().setBrokerTopic(topic);

	SingleEplServer::Instance().run();

	return 0;
}
