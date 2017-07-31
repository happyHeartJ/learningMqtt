#include "appcheck.h"
#define APP_NAME_LENGTH		256
#define CHECK_APP_NAME		0x01
#define CHECK_APP_NUM		0x02
#define CHECK_APP_BOTH		0x03
unsigned int getAppNum()
{
    int count = 0;
    FILE *fp = popen(GET_PID_COMMAND, "r");
    char brokerPid[IAU_PID_LENGTH] = { 0 };
    while (NULL != fgets(brokerPid, IAU_PID_LENGTH, fp))
    {
	count++;
    }
    pclose(fp);

    return count;
}

bool checkApp(char *runPath, size_t pathLen, char *compare, unsigned int mode)
{
    char appName[APP_NAME_LENGTH] = {0};
    char* path_end;

    path_end = strrchr(processdir, '/');
    if (path_end == NULL)
    {
	return -1;
    }

    ++path_end;
    strcpy(appName, path_end);
    //*path_end = '\0';

    bool checkName = (bool)strcmp(appName, compare);
    bool checkNum = getAppNum()>1 ? false : true;

    switch(mode)
    {
	case CHECK_APP_NAME:
	{
	    return checkName;
	}
	case CHECK_APP_NUM:
	{
	    return checkNum;
	}
	case CHECK_APP_BOTH:
	{
	    return checkName & checkNum;
	}
	default:
	    return false;
    }
}
