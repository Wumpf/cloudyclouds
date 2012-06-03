#include "stdafx.h"
#include "CloudyClouds.h"

int main()
{
	try
	{
		CloudyClouds cloudyClouds;
		cloudyClouds.mainLoop();
	}
	catch(std::exception& exception)
	{
		Log::get() << exception.what();
	}
	catch(...)
	{
		Log::get() << "ERROR: unhandled Exception Occured!\n";
	};

#ifdef WIN32
	system("PAUSE");
#endif
}