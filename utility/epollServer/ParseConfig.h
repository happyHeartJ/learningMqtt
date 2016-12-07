#pragma once

#include "commoninc.h"

class ParseConfig
{
public:
	ParseConfig(void);
	~ParseConfig(void);

public:
	void description();
	int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal);

private:
	char* left_trim(char * szOutput, const char *szInput);
	char* right_trim(char *szOutput, const char *szInput);
	char* all_trim(char * szOutput, const char * szInput);
};
