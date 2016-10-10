/*
 * Author	: JiangHongjun
 * Date		: 2016/9/30	
 * Singleton mode
 * You can quickly implement a singleton class for you app.
 * "SingleAPIObject" is a class name and you can chooose you favorite 
 * name replacing that.
 * */

#pragma once


#include <iostream>
using namespace std;

class SingleAPIObject
{
public:
	static SingleAPIObject& Instance()
	{
		static SingleAPIObject theSingleton;
		return theSingleton;
	}
	/*
	 * more non-static functions, including public and private
	 * */

private:
	SingleAPIObject();
	SingleAPIObject(SingleAPIObject const&);
	SingleAPIObject& operator=(SingleAPIObject const&);
	~SingleAPIObject();

};
