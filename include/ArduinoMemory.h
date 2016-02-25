#ifndef ARDUINOMEMORY_H
#define ARDUINOMEMORY_H


#include <GSM.h>
#include <EEPROM/EEPROM.h>

#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "Types.h"
#include "MMeteoUser.h"

class ArduinoMemory
{
	public:
		ArduinoMemory();

		ArduinoMemory(const ArduinoMemory&)=delete;
		ArduinoMemory& operator = (const ArduinoMemory&)=delete;

		sbyte findUserIndex(const String& num);
		
		MMeteoUser* getMMeteoUser(ubyte userIndex);
		
		void resetEeprom();
		
		void setOn(bool value);
		bool isOn();
		
		String getMMeteoTel();
		void setMMeteoTel(const String& tel);
		
		unsigned int getMMeteoServerPort();
		void setMMeteoServerPort(unsigned int  port);
				
		String getMMeteoServerHost();
		void setMMeteoServerHost(const String& host);
		//void setMMeteoServerHost(char host[URL_LENGTH]);
		
		String getMMeteoIMEI();
		void setMMeteoIMEI(const String& imei);
		
		ubyte getNextGprsCallIndex();
		void setNextGprsCallIndex(ubyte index);
		
		sbyte getTempLimitMin();
		sbyte getTempLimitMax();

		sbyte getHumLimitMin();
		sbyte getHumLimitMax();
		
		void setTempLimitMin(sbyte value);
		void setTempLimitMax(sbyte value);
		
		void setHumLimitMin(sbyte value);
		void setHumLimitMax(sbyte value);
		
		unsigned int getSmsReceived();
		unsigned int getSmsSent();
		unsigned long getBytesReceived();
		unsigned long getBytesSent();
		
		void setSmsReceived(unsigned int value);
		void setSmsSent(unsigned int value);
		void setBytesReceived(unsigned long value);
		void setBytesSent(unsigned long value);
		
		void setDefaultLimitValues();
		
		void resetAllUsers();
		
		void load();
		void save();
	
	private:
		MMeteoUser _MMeteoUser[MAX_USERS];
		
		bool _On;
		String _MMeteoTel;
		
		ubyte _NextGprsCallIndex;
		
		sbyte _TempLimitMin;
		sbyte _TempLimitMax;
		sbyte _HumLimitMin;
		sbyte _HumLimitMax;

		unsigned int _SmsReceived;
		unsigned int _SmsSent;
		unsigned long _BytesReceived;
		unsigned long _BytesSent;
		
		unsigned int _MMeteoServerPort;
		String _MMeteoServerHost;
		String _MMeteoImei;
		
		byte loadByteEEPROM( ubyte index  );
		void writeByteEEPROM( ubyte index, byte value );
		
		bool compareTel(const String& tel1, const String& tel2);
};

#endif
