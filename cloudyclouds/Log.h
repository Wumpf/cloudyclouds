#pragma once

#define OUTPUT_DEBUG_STRINGS

/// \brief Log-Class
/// \remarks Simple Log class - singleton!
class Log
{
public:

	/// writes text to the Log - "arbitrary"
	template <class T> Log& operator<< (const T text)
	{
		outputStream << text;
#ifdef _DEBUG
		outputStream.flush();
#endif

#ifdef OUTPUT_DEBUG_STRINGS
		std::stringstream tmp("");
		tmp << text;
	#ifdef WIN32
		OutputDebugStringA(tmp.str().c_str());
	#endif
	#ifdef _CONSOLE
		std::cout << tmp.str().c_str();
	#endif
#endif

		return *this;
	}


	/// singleton getter
	static Log& get ()
	{
		static Log theOnlyOne("log.txt");
		return theOnlyOne;
	}

private:
	/// constructor
	/// \param Filename Der Dateiname des Logs
	Log(const std::string& Filename) : outputStream (Filename.c_str()) {}

	/// deconstructor
	~Log()
	{
		outputStream.flush();
		outputStream.close();
	}

	/// outputstream
	std::ofstream outputStream; 
};
