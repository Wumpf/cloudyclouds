#include "stdafx.h"
#include "CloudyClouds.h"
#include <time.h>

int main()
{
	srand(static_cast<unsigned int>(time(nullptr)));

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