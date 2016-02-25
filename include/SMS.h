#ifndef SMS_H
#define SMS_H

#include <string.h>

#include "Types.h"

class SMS
{
	public:
		enum SMS_TYPE { SMS_TYPE_UNKNOW, SMS_TYPE_NUM, SMS_TYPE_DATA, SMS_TYPE_STATUS, SMS_TYPE_ADD, SMS_TYPE_RESET, SMS_TYPE_DELETE, SMS_TYPE_ON, SMS_TYPE_OFF, SMS_TYPE_GET_SERVER, SMS_TYPE_SET_SERVER, SMS_TYPE_START, SMS_TYPE_STOP, SMS_TYPE_DAILY, SMS_TYPE_SET, SMS_TYPE_REBOOT, SMS_TYPE_VERSION, SMS_TYPE_SAVE, SMS_TYPE_BROADCAST, SMS_TYPE_LIST, SMS_TYPE_EEPROM_RESET, SMS_TYPE_GET_PORT, SMS_TYPE_SET_PORT, SMS_TYPE_GET_GPRS_FREQ, SMS_TYPE_SET_GPRS_FREQ };
	
	public:
		SMS();
		SMS(const SMS&)=delete;
		SMS& operator = (const SMS&)=delete;
		
		String getRemoteTel();
		void setRemoteTel(const String& tel);
		//void setRemoteTel(char tel[LEN_TEL]);
		
		//void setMessage(char message[MAX_LENGTH_SMS_MESSAGE]);
		void setMessage(const String& message);
		String getMessage();
		
		bool isValid();
		void setValid(bool value);
		
		bool isSent();
		void setSent(bool value);
		
		ubyte getSentFailCount();
		void setSentFailCount(ubyte value);
						
		SMS_TYPE getSmsType();
	private:
		//char _RemoteTel[LEN_TEL];
		String _RemoteTel;
		//char _Message[MAX_LENGTH_SMS_MESSAGE];
		String _Message;
		
		bool _Valid;
		bool _Sent;
		
		ubyte _SentFailCount;
};

#endif