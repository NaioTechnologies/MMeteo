#include "ArduinoMemory.h"
#include "Logger.h"

ArduinoMemory::ArduinoMemory() :
	_MMeteoUser(),
	_On(true),
	_MMeteoTel(),
	_MMeteoServerHost(),
	_MMeteoImei(),
	_TempLimitMin(10),
	_TempLimitMax(30),
	_HumLimitMin(0),
	_HumLimitMax(100),
	_SmsReceived(0),
	_SmsSent(0),
	_BytesReceived(0),
	_BytesSent(0),
	_MMeteoServerPort(4243)
{
	_MMeteoTel.reserve(LEN_TEL);
	_MMeteoServerHost.reserve(GPRS_SERV_HOST_MAX_SIZE);
	_MMeteoImei.reserve(IMEI_LENGTH);

	_MMeteoTel[0] = '\0';
	_MMeteoServerHost[0] = '\0';
	_MMeteoImei[0] = '\0';
}

String ArduinoMemory::getMMeteoIMEI()
{
	return _MMeteoImei;
}

void ArduinoMemory::setMMeteoIMEI(const String& imei)
{
	_MMeteoImei = imei;
}

String ArduinoMemory::getMMeteoServerHost()
{
	return _MMeteoServerHost;
}

void ArduinoMemory::setMMeteoServerHost(const String& host)
{
	_MMeteoServerHost = host;
}

unsigned int ArduinoMemory::getMMeteoServerPort()
{
	return _MMeteoServerPort;
}

void ArduinoMemory::setMMeteoServerPort(unsigned int port)
{
	_MMeteoServerPort = port;
}

void ArduinoMemory::setMMeteoTel(const String& tel)
{
	_MMeteoTel = tel;
}

void ArduinoMemory::setOn(bool value)
{
	_On = value;
}

bool ArduinoMemory::isOn()
{
	return _On;
}

String ArduinoMemory::getMMeteoTel()
{
	return _MMeteoTel;
}

ubyte ArduinoMemory::getNextGprsCallIndex()
{
	return _NextGprsCallIndex;
}

void ArduinoMemory::setNextGprsCallIndex(ubyte index)
{
	_NextGprsCallIndex = index;
}

sbyte ArduinoMemory::getTempLimitMin()
{
	return _TempLimitMin;
}

sbyte ArduinoMemory::getTempLimitMax()
{
	return _TempLimitMax;
}


sbyte ArduinoMemory::getHumLimitMin()
{
	return _HumLimitMin;
}

sbyte ArduinoMemory::getHumLimitMax()
{
	return _HumLimitMax;
}

void ArduinoMemory::setTempLimitMin(sbyte value)
{
	_TempLimitMin = value;
}

void ArduinoMemory::setTempLimitMax(sbyte value)
{
	_TempLimitMax = value;
}


void ArduinoMemory::setHumLimitMin(sbyte value)
{
	_HumLimitMin = value;
}

void ArduinoMemory::setHumLimitMax(sbyte value)
{
	_HumLimitMax = value;
}

byte ArduinoMemory::loadByteEEPROM( ubyte index  )
{
	byte value;
	
	while ( !eeprom_is_ready() );
	
	cli();
	
	value = EEPROM.read(index);
	
	sei();
	
	return value;
}


void ArduinoMemory::writeByteEEPROM( ubyte index, byte value )
{
	while (!eeprom_is_ready());
	
	cli();
	
	if( EEPROM.read(index) != value )
	{
		EEPROM.write(index,value);
	}
	
	sei();
}		
		
void ArduinoMemory::setDefaultLimitValues()
{
	_TempLimitMin = 0;
	_TempLimitMax = 30;
	_HumLimitMin = 0;
	_HumLimitMax = 100;
}
		
void ArduinoMemory::load()
{
	int bufferIdx = 0;
	
	Logger::logLnF("->load()");
	
	char mmeteoUserTel[LEN_TEL];
	char mmeteoTel[LEN_TEL];
	char mmeteoServerHost[GPRS_SERV_HOST_MAX_SIZE];
	char mmeteoImei[IMEI_LENGTH];
	
	if (loadByteEEPROM(bufferIdx++) == CURRENT_EEPROM_DATA_VERSION)
	{
		// read current number phone
		for (ubyte i = 0;i<LEN_TEL;i++)
		{
			mmeteoTel[i] = loadByteEEPROM(bufferIdx++);
		}
		setMMeteoTel( mmeteoTel );
	
		for (ubyte i = 0;i<GPRS_SERV_HOST_MAX_SIZE;i++)
		{
			mmeteoServerHost[i] = loadByteEEPROM(bufferIdx++);
		}
		
		setMMeteoServerHost(String(mmeteoServerHost));
		
		for (ubyte i = 0;i<IMEI_LENGTH;i++)
		{
			mmeteoImei[i] = loadByteEEPROM(bufferIdx++);
		}
		
		setMMeteoIMEI(String(mmeteoImei));		
		
		_On = loadByteEEPROM(bufferIdx++);
		
		_TempLimitMin = loadByteEEPROM(bufferIdx++);
		_TempLimitMax = loadByteEEPROM(bufferIdx++);
		_HumLimitMin = loadByteEEPROM(bufferIdx++);
		_HumLimitMax = loadByteEEPROM(bufferIdx++);
		
		// ################################################
		
		for (ubyte i = 0;i<MAX_USERS ;i++)
		{
			for (ubyte j = 0;j<LEN_TEL;j++)
			{
				mmeteoUserTel[j] = loadByteEEPROM(bufferIdx++);
			}
			
			_MMeteoUser[i].setTel(String(mmeteoUserTel));
			
			_MMeteoUser[i].setActive( loadByteEEPROM(bufferIdx++) );
			
			_MMeteoUser[i].setDaily( loadByteEEPROM(bufferIdx++) );
			
			_MMeteoUser[i].setAlertDisabled( loadByteEEPROM(bufferIdx++) );
			
			for (ubyte h = 0;h<24;h++)
			{
				_MMeteoUser[i].getAlerts()[h] = loadByteEEPROM(bufferIdx++);
			}
			
			_MMeteoUser[i].setIndex( i );
		}

		_SmsReceived  = ((unsigned long) loadByteEEPROM(bufferIdx++)) << 8;
		_SmsReceived |= ((unsigned long) loadByteEEPROM(bufferIdx++));					   					   
					   
		_SmsSent  = ((unsigned long) loadByteEEPROM(bufferIdx++)) << 8;
		_SmsSent |= ((unsigned long) loadByteEEPROM(bufferIdx++));					   
					   
		_BytesReceived  = ((unsigned long) loadByteEEPROM(bufferIdx++)) << 24;
		_BytesReceived |= ((unsigned long) loadByteEEPROM(bufferIdx++)) << 16;
		_BytesReceived |= ((unsigned long) loadByteEEPROM(bufferIdx++)) << 8;
		_BytesReceived |= ((unsigned long) loadByteEEPROM(bufferIdx++));					   
					   
		_BytesSent  = ((unsigned long) loadByteEEPROM(bufferIdx++)) << 24;
		_BytesSent |= ((unsigned long) loadByteEEPROM(bufferIdx++)) << 16;
		_BytesSent |= ((unsigned long) loadByteEEPROM(bufferIdx++)) << 8;
		_BytesSent |= ((unsigned long) loadByteEEPROM(bufferIdx++));					   
					   
		_MMeteoServerPort  = ((unsigned long) loadByteEEPROM(bufferIdx++)) << 8;
		_MMeteoServerPort  |= ((unsigned long) loadByteEEPROM(bufferIdx++));		
		
		_NextGprsCallIndex = loadByteEEPROM(bufferIdx++);
		
		Logger::logF("READ IMEI :");
		Logger::logLn(_MMeteoImei);
		Logger::logF("READ NUMBER PHONE :");
		Logger::logLn(_MMeteoTel);
		Logger::logF("READ SERVER HOST :");
		Logger::logLn(_MMeteoServerHost);
	}
	else
	{
		Logger::logLnF("BAD CURRENT_EEPROM_DATA_VERSION RESETING EEPROM");
		
		for (int i = 0;i<4096;i++)
		{
			writeByteEEPROM(i, 0);
		}
		
		writeByteEEPROM(0, CURRENT_EEPROM_DATA_VERSION);
		
		setMMeteoServerHost( RESOURCE_GPRS_DEFAULT_SERV_URL );
		
		setMMeteoServerPort( GPRS_SERV_PORT );
		
		setMMeteoIMEI( RESOURCE_GPRS_DEFAULT_IMEI );
		
		setNextGprsCallIndex( GPRS_DATA_SEND_FREQUENCY_MINUTES );
		
		save();
		
		resetAllUsers();
	}
	
	Logger::logLnF("<-load()");
}

void ArduinoMemory::resetAllUsers()
{
	for (ubyte i = 0;i<MAX_USERS;i++)
	{
		//loadMMeteoUserFromEeprom(i);
		
		_MMeteoUser[i].setActive(false);
		_MMeteoUser[i].setDaily(false);
		_MMeteoUser[i].setAlertDisabled(true);

		for (ubyte h = 0; h<24 ; h++)
		{
			_MMeteoUser[i].getAlerts()[h] = 0;
		}
		
		_MMeteoUser[i].getTel()[0] = 'X';
		_MMeteoUser[i].getTel()[1] = '\0';
		
		//writeMMeteoUserIntoEeprom();
	}
}

void ArduinoMemory::resetEeprom()
{
	Logger::logLnF("->resetEeprom()");
	
	for (int i = 0;i<1024 ;i++)
	{
		writeByteEEPROM(i, 0);	
	}
	
	Logger::logLnF("<-resetEeprom()");
}

void ArduinoMemory::save()
{
	int bufferIdx = 0;
	
	Logger::logLnF("->save()");
	
	//EEPROM.write(ad,val);
	writeByteEEPROM(bufferIdx++, CURRENT_EEPROM_DATA_VERSION);
	
	// Save current number phone
	for (ubyte i = 0;i<LEN_TEL ;i++)
	{
		writeByteEEPROM(bufferIdx++,(byte)_MMeteoTel.charAt(i));
	}
	
	for (ubyte i = 0;i<GPRS_SERV_HOST_MAX_SIZE;i++)
	{
		writeByteEEPROM(bufferIdx++,(byte)_MMeteoServerHost.charAt(i));
	}
	
	for (ubyte i = 0;i<IMEI_LENGTH;i++)
	{
		writeByteEEPROM(bufferIdx++,(byte)_MMeteoImei.charAt(i));
	}
	
	//On sauvegarde l'état ON/OFF
	writeByteEEPROM(bufferIdx++,(byte)_On);
	
	//On sauvegarde les températures d'alertes
	writeByteEEPROM(bufferIdx++,_TempLimitMin);
	writeByteEEPROM(bufferIdx++,_TempLimitMax);
	writeByteEEPROM(bufferIdx++,_HumLimitMin);
	writeByteEEPROM(bufferIdx++,_HumLimitMax);
	
	// #################################################
	
	for (ubyte i = 0;i<MAX_USERS ;i++)
	{
		for (ubyte j = 0;j< LEN_TEL;j++)
		{
			writeByteEEPROM(bufferIdx++,(_MMeteoUser[i].getTel().charAt(j)));
		}
		
		writeByteEEPROM(bufferIdx++,_MMeteoUser[i].isActive());
		
		writeByteEEPROM(bufferIdx++,_MMeteoUser[i].isDaily());
				
		writeByteEEPROM(bufferIdx++,_MMeteoUser[i].isAlertDisabled());

		//24 octets pour les heures
		//octets à partir de 150 
		for (ubyte h = 0;h<24;h++)
		{
			writeByteEEPROM(bufferIdx++, _MMeteoUser[i].getAlerts()[h]);
		}
	}

	writeByteEEPROM(bufferIdx++, (unsigned int)((_SmsReceived & 0xFF00) >> 8 ));		
	writeByteEEPROM(bufferIdx++, (unsigned int)((_SmsReceived & 0X00FF)));
	
	writeByteEEPROM(bufferIdx++, (unsigned int)((_SmsSent & 0xFF00) >> 8 ));		
	writeByteEEPROM(bufferIdx++, (unsigned int)((_SmsSent & 0X00FF) ));

	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesReceived & 0xFF000000) >> 24 ));		
	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesReceived & 0x00FF0000) >> 16 ));		
	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesReceived & 0x0000FF00) >> 8 ));		
	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesReceived & 0X000000FF) ));

	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesSent & 0xFF000000) >> 24 ));		
	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesSent & 0x00FF0000) >> 16 ));		
	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesSent & 0x0000FF00) >> 8 ));		
	writeByteEEPROM(bufferIdx++, (unsigned long)((_BytesSent & 0X000000FF) ));
	
	writeByteEEPROM(bufferIdx++, (unsigned int)((_MMeteoServerPort & 0xFF00) >> 8 ));		
	writeByteEEPROM(bufferIdx++, (unsigned int)((_MMeteoServerPort & 0X00FF)));
	
	writeByteEEPROM(bufferIdx++, _NextGprsCallIndex);
	
	Logger::logLnF("<-save()");
}

/*
void ArduinoMemory::writeMMeteoUserIntoEeprom()
{
	Serial.print(F("writeMMeteoUserIntoEeprom() : "));
	Serial.println( _MMeteoUser.getIndex() );
	
	if( _MMeteoUser.getIndex() != -1)
	{
		writeMMeteoUserToEeprom(_MMeteoUser.getIndex());
	}
}
*/

//sbyte ArduinoMemory::findUserIndex(char num[LEN_TEL])
sbyte ArduinoMemory::findUserIndex(const String& num)
{
	for (sbyte i = 0;i<MAX_USERS;i++)
	{
		if( _MMeteoUser[i].isActive() == true && compareTel( num, _MMeteoUser[i].getTel() ) == true )
		{
			return i;
		}
	}
	
	return -1;
}

MMeteoUser* ArduinoMemory::getMMeteoUser(ubyte userIndex)
{
	return &_MMeteoUser[userIndex];
}


bool ArduinoMemory::compareTel(const String& tel1, const String& tel2)
{
	bool isEqual = false;

	ubyte cpt =0;
	ubyte telIndex1 =0;
	ubyte telIndex2 =0;
	
	String telFilter1;
	String telFilter2;
	
	Logger::logF("Compare Tel : ");
	Logger::log(tel1);
	Logger::logF(" : ");
	Logger::log(tel2);

	
	if( tel1.charAt(0) == '0' && tel1.charAt(1) == '0' && tel1.charAt(2) == '3' && tel1.charAt(3) == '3')
	{
		telIndex1 = 4;
	}
	else if( tel1.charAt(0) == '+' && tel1.charAt(1) == '3' && tel1.charAt(2) == '3' )
	{
		telIndex1 = 3;
	}
	if( tel1.charAt(0) == '0'  )
	{
		telIndex1 = 1;
	}

	if( tel2.charAt(0) == '0' && tel2.charAt(1) == '0' && tel2.charAt(2) == '3' && tel2.charAt(3) == '3')
	{
		telIndex2 = 4;
	}
	else if( tel2.charAt(0) == '+' && tel2.charAt(1) == '3' && tel2.charAt(2) == '3' )
	{
		telIndex2 = 3;
	}
	if( tel2.charAt(0) == '0'  )
	{
		telIndex2 = 1;
	}
		
	telFilter1 = tel1.substring(telIndex1);	
		
	telFilter2 = tel2.substring(telIndex2);	

	if( telFilter1.length() != 9 || telFilter2.length() != 9 )
	{
		isEqual = false;	
	}
	else
	{
		if( telFilter1.equals( telFilter2 ) == true )
		{
			isEqual = true;
		}
		else
		{
			isEqual = false;
		}
	}
	
	Logger::logF(" : ");
	Logger::log(telFilter1);
	Logger::logF(" : ");
	Logger::log(telFilter2);		
		
	Logger::logF(" = ");
	Logger::logLn(String(isEqual));
	
	return isEqual;
}

unsigned int ArduinoMemory::getSmsReceived()
{
	return _SmsReceived;
}

unsigned int ArduinoMemory::getSmsSent()
{
	return _SmsSent;
}

unsigned long ArduinoMemory::getBytesReceived()
{
	return _BytesReceived;
}

unsigned long ArduinoMemory::getBytesSent()
{
	return _BytesSent;
}
		
void ArduinoMemory::setSmsReceived(unsigned int value)
{
	_SmsReceived = value;
}

void ArduinoMemory::setSmsSent(unsigned int value)
{
	_SmsSent = value;
}

void ArduinoMemory::setBytesReceived(unsigned long value)
{
	_BytesReceived = value;
}

void ArduinoMemory::setBytesSent(unsigned long value)
{
	_BytesSent = value;
}

