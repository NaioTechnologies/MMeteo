#include <Arduino.h>
#include "Logger.h"
#include "Types.h"


void Logger::logLnF(const String& message)
{
	#ifdef SERIAL_OUT_ON
	Serial.println(message.c_str());
	#endif
}

void Logger::logF(const String&  message)
{
	#ifdef SERIAL_OUT_ON
	Serial.print(message.c_str());
	#endif
}

void Logger::logLn(const String&  message)
{
	#ifdef SERIAL_OUT_ON
	Serial.println(message.c_str());
	#endif
}

void Logger::log(const String&  message)
{
	#ifdef SERIAL_OUT_ON
	Serial.print(message.c_str());
	#endif
}

/*
void Logger::logLn(const int value)
{
	#ifdef SERIAL_OUT_ON
	Serial.println(value);
	#endif
}

void Logger::log(const int value)
{
	#ifdef SERIAL_OUT_ON
	Serial.print(value);
	#endif
}
*/