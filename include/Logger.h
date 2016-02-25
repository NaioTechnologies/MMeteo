#ifndef LOGGER_H
#define LOGGER_H


class Logger
{
	public:
		Logger();
		Logger(const Logger&)=delete;
		Logger& operator = (const Logger&)=delete;
		
		static void logLnF(const String&  message);
		static void logF(const String&  message);
		
		static void logLn(const String&  message);
		static void log(const String&  message);
		
		//static void logLn(const int value);
		//static void log(const int value);
	private:
};

#endif