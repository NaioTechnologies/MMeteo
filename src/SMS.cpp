#include "SMS.h"

#include <stdio.h>
#include <ctype.h>
#include "Logger.h"

SMS::SMS() :
	_RemoteTel(),
	_Valid(false),
	_Sent(false),
	_SentFailCount(0)
{
	if( _RemoteTel.reserve(LEN_TEL) == 0 )
	{
		Logger::logLnF(" SMS::SMS()  _RemoteTel.reserve(LEN_TEL) == 0 !!! ");
	}
}

ubyte SMS::getSentFailCount()
{
	return _SentFailCount;
}

void SMS::setSentFailCount(ubyte value)
{
	_SentFailCount = value;
}

String SMS::getRemoteTel()
{
	return _RemoteTel;
}

void SMS::setRemoteTel( const String& tel )
{
	_RemoteTel = tel;
}

String SMS::getMessage()
{
	return _Message;
}

void SMS::setMessage( const String& message )
{
	_Message = message;
	
	_Message.toUpperCase();
}

bool SMS::isValid()
{
	return _Valid;
}

void SMS::setValid(bool value)
{
	_Valid = value;
}

bool SMS::isSent()
{
	return _Sent;
}

void SMS::setSent(bool value)
{
	_Sent = value;
}

SMS::SMS_TYPE SMS::getSmsType()
{
	//if( strncmp( _Message,"NUM",3) == 0 )
	if( _Message.startsWith( "NUM" ) == true )
	{
		return SMS_TYPE_NUM;
	}
	else if( _Message.startsWith( "GETGPRS" ) == true ) 
	{
		return SMS_TYPE_GET_GPRS_FREQ;
	}	
	else if( _Message.startsWith( "SETGPRS" ) == true ) 
	{
		return SMS_TYPE_SET_GPRS_FREQ;
	}		
	else if( _Message.startsWith( "DATA" ) == true || _Message.startsWith("INFO" ) )
	{
		return SMS_TYPE_DATA;
	}
	else if( _Message.startsWith( "STATU" ) == true )
	{
		return SMS_TYPE_STATUS;
	}
	else if( _Message.startsWith( "ADD" ) == true )
	{
		return SMS_TYPE_ADD;
	}
	else if( _Message.startsWith( "RESET" ) == true  )
	{
		return SMS_TYPE_RESET;
	}
	else if( _Message.startsWith( "DELETE" ) == true || _Message.startsWith( "REMOVE" ) == true ) 
	{
		return SMS_TYPE_DELETE;
	}
	else if( _Message.startsWith( "ON" ) == true )
	{
		return SMS_TYPE_ON;
	}
	else if( _Message.startsWith( "OFF" ) == true  ) 
	{
		return SMS_TYPE_OFF;
	}
	else if( _Message.startsWith( "GETSERVER" ) == true )
	{
		return SMS_TYPE_GET_SERVER;
	}
	else if( _Message.startsWith( "SETSERVER" ) == true )
	{
		return SMS_TYPE_SET_SERVER;
	}
	else if( _Message.startsWith( "GETPORT" ) == true )
	{
		return SMS_TYPE_GET_PORT;
	}
	else if( _Message.startsWith( "SETPORT" ) == true )
	{
		return SMS_TYPE_SET_PORT;
	}
	else if( _Message.startsWith( "START" ) == true ) 
	{
		return SMS_TYPE_START;
	}
	else if( _Message.startsWith( "STOP" ) == true ) 
	{
		return SMS_TYPE_STOP;
	}
	else if( _Message.startsWith( "DAILY" ) == true ) 
	{
		return SMS_TYPE_DAILY;
	}
	else if( _Message.startsWith( "SET" ) == true ) 
	{
		return SMS_TYPE_SET;
	}
	else if( _Message.startsWith( "REBOOT" ) == true ) 
	{
		return SMS_TYPE_REBOOT;
	}
	else if( _Message.startsWith( "VERSION" ) == true ) 
	{
		return SMS_TYPE_VERSION;
	}	
	else if( _Message.startsWith( "SAVE" ) == true ) 
	{
		return SMS_TYPE_SAVE;
	}	
	else if( _Message.startsWith( "BROADCAST" ) == true ) 
	{
		return SMS_TYPE_BROADCAST;
	}	
	else if( _Message.startsWith( "LIST" ) == true ) 
	{
		return SMS_TYPE_LIST;
	}		
	else if( _Message.startsWith( "EEPROM RESET" ) == true ) 
	{
		return SMS_TYPE_EEPROM_RESET;
	}		
	

	
	return SMS_TYPE_UNKNOW;
}

