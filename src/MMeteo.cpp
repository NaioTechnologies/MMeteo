#include "MMeteo.h"
#include "Logger.h"
#include "version.h"

MMeteo::MMeteo() :
	_ArduinoGsmShield(),
	_ArduinoMemory(),
	_DHT(SENSOR_PIN, DHT22),
	_NextGprsCallIndex(255),
	_NextAlertSendIndex(255),
	_TempAlertOnLevel(0),
	_TempWarningOnLevel(0),
	_HumAlertOnLevel(0),
	_HumWarningOnLevel(0),
	_LastAlertSentTime(0),
	_NextMemorySaveIndex(255),
	_Started(false),
	_GprsFailedLastTime(false),
	_SmsNumSentCount(0)
{

}

void MMeteo::reboot()
{
	Logger::logLnF("Rebooting");
	
	_ArduinoMemory.save();
	
	delay(1000UL);
	
	asm volatile(" jmp 0x00");
}

void MMeteo::setup()
{
	Logger::logLnF("Starting MMeteo.");
	
	Logger::logLnF(GIT_VERSION);
	Logger::logLnF(SOFT_VERSION);
	Logger::logLnF(HOST_NAME_ID);
	Logger::logLnF(DATE_VERSION);
	
	Logger::logLnF(String(PINNUMBER));
	Logger::logLnF(String(GPRS_APN));
	
	_ArduinoGsmShield.setArduinoMemory(&_ArduinoMemory);
	
	_ArduinoMemory.load();
		
	Logger::logLnF("Users stored : "); 
	
	for (ubyte i=0 ; i<MAX_USERS ; i++)
	{
		if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
		{
			Logger::logF("Active user load : "); 
			Logger::log( String(i) ); 
			Logger::logF(" : "); 
			Logger::logLn(_ArduinoMemory.getMMeteoUser(i)->getTel());
		}
	}
}

// Main loop
void MMeteo::loop()
{
	char smsCount = 0;
	
	// #####################################
		
	if( _ArduinoMemory.getMMeteoIMEI().equals( RESOURCE_GPRS_DEFAULT_IMEI ) == true )
	{
		Logger::logLnF("MMeteo Imei : NOT DEFINED");
		
		_ArduinoMemory.setMMeteoIMEI( _ArduinoGsmShield.getImei() );
		
		_ArduinoMemory.save();
	}
	
	// #####################################
	
	// First passage to the loop, or until we get the server time
	// we try to connect to gprs server.
	if( _Started == false )
	{
		Logger::logLnF("Get time by gprs connection first."); 
		
		if( _ArduinoGsmShield.connectGSM() == true )
		{
			if( _ArduinoGsmShield.connectGPRS() == true )
			{
				GprsData *gprsData = buildGprsDataToSend();
				
				_ArduinoGsmShield.sendDataByGprs(_ArduinoMemory.getMMeteoServerHost(), gprsData);
				
				if( gprsData->isServerMessageReceived() == true )
				{
					onServerDataReceived(gprsData);
					
					_Started = true;
					
					Logger::logLnF("Time recuperated, started mode on."); 
				}
				
				delete gprsData;
				
				_ArduinoGsmShield.disconnectGPRS();
			}
			
			_ArduinoGsmShield.disconnectGSM();	
		}
	}
	
	// Display current loaded users.
	for (ubyte i = 0;i<MAX_USERS;i++)
	{
		if ( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
		{
			Logger::logF("User in memory : ");
			Logger::logLn(_ArduinoMemory.getMMeteoUser(i)->getTel());
		}
	}
	
	Logger::logF("SMS Received ");
	Logger::logLn(String(_ArduinoMemory.getSmsReceived()));
	Logger::logF("SMS Sent ");
	Logger::logLn(String(_ArduinoMemory.getSmsSent()));
	Logger::logF("Bytes Received ");
	Logger::logLn(String(_ArduinoMemory.getBytesReceived()));
	Logger::logF("Bytes Sent ");
	Logger::logLn(String(_ArduinoMemory.getBytesSent()));
	
	if( _ArduinoGsmShield.connectGSM() == true )
	{
		if( _ArduinoMemory.isOn() == true )
		{
			if( timeToSendGprsData() == true || _GprsFailedLastTime == true)
			{
				if( _ArduinoGsmShield.connectGPRS() == true )
				{
					Serial.println(F("Time for GPRS data sending."));

					GprsData *gprsData = buildGprsDataToSend();
					
					_ArduinoGsmShield.sendDataByGprs(_ArduinoMemory.getMMeteoServerHost(), gprsData);
					
					if( gprsData->isServerMessageReceived() == true )
					{
						onServerDataReceived(gprsData);
					}
					
					delete gprsData;
				
					_GprsFailedLastTime = false;					

					_ArduinoGsmShield.disconnectGPRS();
				}
				else
				{
					_GprsFailedLastTime = true;
				}
			}
		}

		// #####################################
		
		Logger::logLnF("Retreiving pending SMS.");
		
		// read pending sms several times to let network inform sim they arrived
		for( ubyte s = 0 ; s < TEST_SMS_PENDING_MAX_TRIES ; s++ )
		{
			
			while( _ArduinoGsmShield.isPendingSMS() && smsCount < MAX_SMS_RECEIVING )
			{
				SMS *sms = _ArduinoGsmShield.getPendindSMS();
				
				if( sms->isValid())
				{
					Logger::logLnF("SMS received.");
					
					manageReceivedSms(sms);
				}
				else
				{
					Logger::logLnF("SMS received, but invalid.");
				}
				
				smsCount++;
				
				delete sms;
			}	
			
			delay(500UL);
		}
	
		// #####################################
	
		if( smsCount == 0 )
		{
			Logger::logLnF("No pending SMS.");
		}	
			
		// #####################################			
		
			
		if( _ArduinoGsmShield.isValidNumber( _ArduinoMemory.getMMeteoTel() ) )
		{
			Logger::logF("MMeteo number : ");
			Logger::logLn(_ArduinoMemory.getMMeteoTel());
		}
		else
		{
			Logger::logLnF("MMeteo number : NOT DEFINED");
			
			if( _SmsNumSentCount < MAX_SMS_NUM_SENT_COUNT )
			{
				_ArduinoGsmShield.sendNumSmsToOdroidServer();
			
				_SmsNumSentCount++;
			}
			else
			{
				Logger::logLnF("Too many sms num sent for this session, wait for response or reboot.");
			}
		}
			
		// #####################################
		
		if( _ArduinoMemory.isOn() == true )
		{
			ckeckAlerts();
		}
		
		// #####################################				
		
		if( _ArduinoMemory.isOn() == true )
		{
			if( timeToSendAlertSMS() == true )
			{
				for(ubyte i = 0 ; i < MAX_USERS ; i++ )
				{
					if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true && _ArduinoMemory.getMMeteoUser(i)->isDaily() == true && _ArduinoMemory.getMMeteoUser(i)->getAlerts()[hour()] == true )
					{
						sendDataCommand( _ArduinoMemory.getMMeteoUser(i)->getTel() );
					}
				}
			}
		}
		
		// #####################################
		
		_ArduinoGsmShield.checkSMSToBeSentNextTime();
		
		// #####################################
		
		_ArduinoGsmShield.disconnectGSM();	
	}

	// #####################################		
	
	if( timeToSaveMemory() == true )
	{
		_ArduinoMemory.save();
	}
	
	// #####################################
	
	Serial.println( _ArduinoMemory.getMMeteoIMEI() );
	Serial.println( _ArduinoMemory.getMMeteoTel() );

	Serial.println( "smsToBeSentNextTimeCount : " + String( _ArduinoGsmShield.getSMSToBeSentNextTimeCount() ) );
	
	Serial.println( "Sleeping..." );

	delay(20000);
}

/* ##########################################################
 * #### GprsData* MMeteo::buildGprsDataToSend()          ####
 * ##########################################################
 * #### Build message to be sent by gprs                 ####
   ##########################################################
*/
// build gprs data for further sending
GprsData* MMeteo::buildGprsDataToSend()
{
	String clientPhone[MAX_USERS];
	char clientMessage[GPRS_MAX_MESSAGE_LENGHT];
	
	float temperature = 0;
	float humidity = 0;
	
	GprsData* gprsData = new GprsData();
	
	temperature = _DHT.readTemperature();
	humidity = _DHT.readHumidity();
	
	for (ubyte i = 0;i<MAX_USERS;i++)
	{
		if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
		{
			clientPhone[i] = _ArduinoMemory.getMMeteoUser(i)->getTel();
		}
		else
		{
			clientPhone[i] = String( 'X' );
		}
	}
	
	sprintf( clientMessage, "%s %04d %02d %02d %02d %02d %d.%d %d %s %s %s %s %s %s %03d %03d %03d %03d %d %d",
			_ArduinoMemory.getMMeteoTel().c_str() ,year(),month(),day(),hour(),minute(),(int)temperature, ((int)(temperature*10))%10,(int)humidity, 
			SOFT_VERSION, DATE_VERSION, _ArduinoMemory.getMMeteoIMEI().c_str(), 
			clientPhone[0].c_str(), clientPhone[1].c_str(), clientPhone[2].c_str(),
			_ArduinoMemory.getTempLimitMin(), _ArduinoMemory.getTempLimitMax(), _ArduinoMemory.getHumLimitMin(), _ArduinoMemory.getHumLimitMax() ,
			_ArduinoMemory.getSmsReceived(), _ArduinoMemory.getSmsSent() 
   		);
	
	gprsData->setClientMessage(String(clientMessage) + String(" ") + String(_ArduinoMemory.getBytesReceived()) + String(" ") + String(_ArduinoMemory.getBytesSent()) + String(" ") + String(_ArduinoGsmShield.getSignalStrength()) );
	
	return gprsData;
}

// is time to save memory ?
bool MMeteo::timeToSaveMemory()
{
	ubyte min = hour();
	
	for (ubyte i = 0;i<24;i++)
	{
		if( min <= (i * 1 ) )
		{
			if( _NextMemorySaveIndex != i )
			{
				_NextMemorySaveIndex = i;
				
				return true;
			}
			
			break;
		}
	}
	
	return false;
}

// is time to send sms alert ?
bool MMeteo::timeToSendAlertSMS()
{
	ubyte min = hour();
	
	for (ubyte i = 0;i<24;i++)
	{
		if( min <= (i * 1 ) )
		{
			if( _NextAlertSendIndex != i )
			{
				_NextAlertSendIndex = i;
				
				return true;
			}
			
			break;
		}
	}
	
	return false;
}

// is time to send gprs data
bool MMeteo::timeToSendGprsData()
{
	ubyte min = minute();
	
	for (ubyte i = 0;i<60;i++)
	{
		if( min <= (i * _ArduinoMemory.getNextGprsCallIndex() ) )
		{
			if( _NextGprsCallIndex != i )
			{
				_NextGprsCallIndex = i;
				
				return true;
			}
			
			break;
		}
	}
	
	return false;
}

// test sms typ and call right command receiver
void MMeteo::manageReceivedSms(SMS *sms)
{
	if( sms->isValid() == true )
	{
		switch(sms->getSmsType())
		{
			case SMS::SMS_TYPE_UNKNOW:
				onInvalidCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_NUM:
				onNumCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_DATA:
				onDataCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_ADD:
				onAddCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_STATUS:
				onStatusCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_RESET:
				onResetCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_DELETE:
				onDeleteCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_ON:
				onOnOffCommandReceived(sms, true);
			break;
			
			case SMS::SMS_TYPE_OFF:
				onOnOffCommandReceived(sms, false);
			break;
			
			case SMS::SMS_TYPE_GET_SERVER:
				onOnGetSetServerCommandReceived(sms, true);
			break;

			case SMS::SMS_TYPE_SET_SERVER:
				onOnGetSetServerCommandReceived(sms, false);
			break;
							
			case SMS::SMS_TYPE_GET_PORT:
				onOnGetSetPortCommandReceived(sms, true);
			break;

			case SMS::SMS_TYPE_SET_PORT:
				onOnGetSetPortCommandReceived(sms, false);
			break;
			
			case SMS::SMS_TYPE_START:
				onStartCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_STOP:
				onStopCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_DAILY:
				onDailyCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_SET:
				onSetCommandReceived(sms);
			break;

			case SMS::SMS_TYPE_REBOOT:
				onRebootCommandReceived(sms);
			break;
			
			case SMS::SMS_TYPE_VERSION:
				onVersionCommandReceived(sms);
			break;		
			
			case SMS::SMS_TYPE_SAVE:
				onSaveCommandReceived(sms);
			break;					

			case SMS::SMS_TYPE_BROADCAST:
				onBroadcastCommandReceived(sms);
			break;	

			case SMS::SMS_TYPE_LIST:
				onListCommandReceived(sms);
			break;			
			
			case SMS::SMS_TYPE_EEPROM_RESET:
				onEepromResetCommandReceived(sms);
			break;			
			
			case SMS::SMS::SMS_TYPE_GET_GPRS_FREQ:
				onOnGetSetGprsFreqCommandReceived(sms, true);
			break;	

			case SMS::SMS::SMS_TYPE_SET_GPRS_FREQ:
				onOnGetSetGprsFreqCommandReceived(sms, false);
			break;	
		}
	}
}

void MMeteo::onOnOffCommandReceived(SMS *sms, bool on)
{
	char onOff[4];
	
	Logger::logLnF("->onOnOffCommandReceived");
	
	if( on == true )
	{
		strcpy(onOff, "ON\0");
		Logger::logLnF("ON");	
	}
	else
	{
		strcpy(onOff, "OFF\0");
		Logger::logLnF("OFF");	
	}
	
	_ArduinoMemory.setOn(on);
	
	for(ubyte i=0; i < MAX_USERS ; i++)
	{
		if(_ArduinoMemory.getMMeteoUser(i)->isActive() == true )
		{
			if( _ArduinoGsmShield.telsAreEquals(_ArduinoMemory.getMMeteoUser(i)->getTel(), sms->getRemoteTel()) == false )
			{
				_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), onOff );
			}
		}
	}
	
	_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), onOff );
	
	Logger::logLnF("<-onOnOffCommandReceived");
}

// on invalide command received respond invalid
void MMeteo::onInvalidCommandReceived(SMS *sms)
{
	Logger::logLnF("->onInvalidCommandReceived");
	
	Logger::logF("Received from : ");
	Logger::log(sms->getRemoteTel());
	Logger::logF(" : ");
	Logger::logLn(sms->getMessage());
	
	if( _ArduinoGsmShield.telsAreEquals( sms->getRemoteTel(), _ArduinoMemory.getMMeteoTel() ) == false )
	{
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_UNKNOW_COMMAND_MESSAGE );
	}
	
	Logger::logLnF("<-onInvalidCommandReceived");
}

// on num command received stores the phone number
void MMeteo::onNumCommandReceived(SMS *sms)
{
	String receivedTel;

	if( receivedTel.reserve(LEN_TEL) == 0 )
	{
		Logger::logLnF("void MMeteo::onNumCommandReceived(SMS *sms) receivedTel.reserve(LEN_TEL) == 0 !!! ");
	}
	
	Logger::logLnF("->onNumSmsReceived");
	
	receivedTel = sms->getMessage().substring(4);
	receivedTel.trim();	
		
	Logger::logF("Received # from Num command : ");
	Logger::logLn(receivedTel);
	
	if (_ArduinoGsmShield.isValidNumber(receivedTel))
	{
		Logger::logLnF("Valid.");
		
		_ArduinoMemory.setMMeteoTel(receivedTel);
				  
		_ArduinoMemory.save();
	}
	else
	{
		Logger::logLnF("Invalid.");
		 
		_ArduinoMemory.setMMeteoTel(RESOURCE_MMETEO_NOT_A_NUMBER);
	}
		
	Logger::logLnF("<-onNumSmsReceived");
}

void MMeteo::onDeleteCommandReceived(SMS *sms)
{
	String telToDelete;
	telToDelete.reserve(LEN_TEL);
	String message = "";
	sbyte userIndex = -1;
	
	Logger::logLnF("->onDeleteCommandReceived");

	telToDelete = sms->getMessage().substring(7);
	telToDelete.trim();

	Logger::logF("Trying to delete : ");
	Logger::logLn(telToDelete);
	
	userIndex = _ArduinoMemory.findUserIndex( telToDelete );

	if( userIndex != -1 )
	{
		_ArduinoMemory.getMMeteoUser(userIndex)->setActive(false);
		
		message = _ArduinoMemory.getMMeteoUser(userIndex)->getTel()	+ " deleted";
		
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), message );
				
		if ( _ArduinoGsmShield.telsAreEquals( sms->getRemoteTel(), _ArduinoMemory.getMMeteoUser(userIndex)->getTel()) == false )
		{
			_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(userIndex)->getTel(), message );
		}
			
		Logger::logF("Removed : ");
		Logger::logLn(_ArduinoMemory.getMMeteoUser(userIndex)->getTel());
				
				
		_ArduinoMemory.save();
	}
	else
	{
		message = String( telToDelete ) + " not in user list.";
		
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), message );
	}
	
	Logger::logLnF("<-onDeleteCommandReceived");
}

void MMeteo::onResetCommandReceived(SMS *sms)
{
	Logger::logLnF("->onResetCommandReceived");
	
	if( sms->getMessage().startsWith("RESET SMS") )
	{
		Logger::logLnF("RESETING SMS");
		
		_ArduinoMemory.setSmsReceived(0);
		_ArduinoMemory.setSmsSent(0);
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), sms->getMessage() );
	}
	else if( sms->getMessage().startsWith("RESET BYTES") )
	{
		Logger::logLnF("RESETING BYTES");
		
		_ArduinoMemory.setBytesReceived(0);
		_ArduinoMemory.setBytesSent(0);
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), sms->getMessage() );
	}
	else
	{
		for (ubyte i = 0;i<MAX_USERS;i++)
		{
			if ( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{
				_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), RESOURCE_USER_RESET );
			}
		}
		
		_ArduinoMemory.setDefaultLimitValues();

		_ArduinoMemory.setOn(true);
		
		_ArduinoMemory.resetAllUsers();
	}
	
	_ArduinoMemory.save();
		
	Logger::logLnF("<-onResetCommandReceived");
}

void MMeteo::onAddCommandReceived(SMS *sms)
{
	String telToAdd;
	telToAdd.reserve(LEN_TEL);
	
	//char message[MAX_LENGTH_SMS_MESSAGE];
	String message = "";
	bool userAvailable = false;
	sbyte userIndex = -1;
	
	Logger::logLnF("->onAddCommandReceived");
	
	telToAdd = sms->getMessage().substring(4);
	telToAdd.trim();
				
	Logger::logF("Trying to add : ");
	Logger::logLn(telToAdd);
	
	if( _ArduinoGsmShield.isValidNumber(telToAdd) )
	{
		userIndex = _ArduinoMemory.findUserIndex(telToAdd);
		
		if ( userIndex == -1 )
		{
			for (ubyte i=0 ; i<MAX_USERS ; i++)
			{
				if ( _ArduinoMemory.getMMeteoUser(i)->isActive() == false )
				{
					_ArduinoMemory.getMMeteoUser(i)->setTel( telToAdd );
					_ArduinoMemory.getMMeteoUser(i)->setActive(true);
					_ArduinoMemory.getMMeteoUser(i)->setDaily(false);
					_ArduinoMemory.getMMeteoUser(i)->setAlertDisabled(false);
					
					for( ubyte h=0 ; h < 24 ; h++)
					{
						_ArduinoMemory.getMMeteoUser(i)->getAlerts()[h] = 0;
					}
					
					userAvailable = true;
					
					message = _ArduinoMemory.getMMeteoUser(i)->getTel() + " added";
					
					_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), message );
					
					if ( _ArduinoGsmShield.telsAreEquals( sms->getRemoteTel(), _ArduinoMemory.getMMeteoUser(i)->getTel()) == false )
					{
						_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), message );
					}
					
					Logger::logF("Added : ");
					Logger::logLn(_ArduinoMemory.getMMeteoUser(i)->getTel());
					
					_ArduinoMemory.save();
					
					break;
				}
			}
			
			if (userAvailable == false)
			{
				_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_NO_MORE_USER_AVAILABLE);
			}
		}
		else
		{
			_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_USER_ALREADY_ACTIVE);
		}
	}
	else
	{
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_INVALID_NUMBER);
	}
	
	Logger::logLnF("<-onAddCommandReceived");
}


// on status command received send limits
void MMeteo::onStatusCommandReceived(SMS *sms)
{
	//char message[MAX_LENGTH_SMS_MESSAGE];
	char txtTemp[5];
	
	String message = "";
	
	sbyte userIndex = -1;
	
	Logger::logLnF("->onStatusCommandReceived");	

	userIndex = _ArduinoMemory.findUserIndex( sms->getRemoteTel() );

	if( userIndex != -1 )
	{
		if( _ArduinoMemory.isOn() == true )
		{
			//sprintf( message,"%d<T<%d\n%d<H<%d\n", _ArduinoMemory.getTempLimitMin(), _ArduinoMemory.getTempLimitMax(), _ArduinoMemory.getHumLimitMin(), _ArduinoMemory.getHumLimitMax() );
			message = message +  _ArduinoMemory.getTempLimitMin() + "<T<" +  _ArduinoMemory.getTempLimitMax() + "\n" +  _ArduinoMemory.getHumLimitMin() + "<H<" + _ArduinoMemory.getHumLimitMax() + "\n";
			
			if( _ArduinoMemory.getMMeteoUser(userIndex)->isActive() == true ) 
			{
				//strcat( message, " " );
				message = message + "";
				
				for( ubyte h =0; h < 24 ; h++)
				{
					if( _ArduinoMemory.getMMeteoUser(userIndex)->getAlerts()[h] == true )
					{
						message = message + " " + h;
					}
				}
			}
			else
			{
				message = message + RESOURCE_ALARM_OFF;
			}
		}
		else
		{
			message = message + RESOURCE_OFF;
		}
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
	else
	{
		message = message + sms->getRemoteTel() + " not in user list.";
		
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), message );
	}
	
	Logger::logLnF("<-onStatusCommandReceived");	
}

// on data command received send PTU data
void MMeteo::onDataCommandReceived(SMS *sms)
{
	Logger::logLnF("->onDataCommandReceived");
	
	if( sms->getMessage().startsWith( "DATA ALL" ) ||  sms->getMessage().startsWith( "INFO ALL" ) )
	{
		for( ubyte i=0; i < MAX_USERS ; i++)
		{
			if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{
				sendDataCommand( _ArduinoMemory.getMMeteoUser(i)->getTel() );		
			}
		}
	}
	else
	{
		sendDataCommand( sms->getRemoteTel() );
	}
		
	Logger::logLnF("<-onDataCommandReceived");
}

void MMeteo::sendDataCommand(const String& tel)
{
	Logger::logLnF("->sendDataCommand");	
	
	String message;
	
	if( message.reserve(MAX_LENGTH_SMS_MESSAGE) == 0 )
	{
		Logger::logLnF("void MMeteo::sendDataCommand(const String& tel)  message.reserve(MAX_LENGTH_SMS_MESSAGE) == 0 !!!");
	}
	
	float temperature = 0;
	float humidity = 0;

	temperature = _DHT.readTemperature();
	humidity = _DHT.readHumidity();

	
	//sprintf(message,"%02d/%02d/%04d\n%02d:%02d\nT = %d.%d C\nH = %d%%",day(),month(),year(),hour(),minute(),(int)temperature, ((int)(temperature*10))%10,(int)humidity);
	message = message + twoDigitIntFormat( day() ) + "/" + twoDigitIntFormat( month() ) + "/" + year() + "\n";
	message = message + twoDigitIntFormat( hour() ) + ":" + twoDigitIntFormat( minute() ) + "\n";
	message = message + "T = " + String( (int)temperature ) + "." + String( ((int)(temperature*10))%10 ) + " C";
	message = message + ", H = " + (int)humidity;
	
	_ArduinoGsmShield.sendSMS( tel, message );
	
	Logger::logLnF("<-sendDataCommand");	
}


// on server data received get time and new server host if present
void MMeteo::onServerDataReceived(GprsData *gprsData)
{
	int hr, min, d, mon, yr;
	
	Logger::logLnF("->onServerDataReceived");
	
	yr = -1;
	mon = -1;
	d = -1;
	hr = -1;
	min = -1;

	if( gprsData->getServerMessage().length() > 16 )
	{
		sscanf( gprsData->getServerMessage().c_str(), "%d/%d/%d %d:%d", &yr, &mon, &d, &hr, &min);
		
		if (yr > 2010 && yr <3000 && mon >0 && mon < 13 && d > 0 && d < 32 && hr > -1 && hr < 25 && min > -1 && min < 60)
		{
			Logger::logLnF("Time read from server : ");
			
			Logger::log( twoDigitIntFormat( hr ) );
			Logger::logF( ":" );
			Logger::log( twoDigitIntFormat( min ) );
			Logger::logLnF( ":00" );
			
			setTime(hr,min,00,d,mon,yr);
		}
	}

	Logger::logLnF("<-onServerDataReceived");
}

void MMeteo::onOnGetSetServerCommandReceived(SMS *sms, bool get)
{
	String message;
	String serverUrl;
	serverUrl.reserve(URL_LENGTH);
	
	Logger::logLnF("->onOnGetSetServerCommandReceived");
	
	if( get == true )
	{
		message = "SERVER URL : " + _ArduinoMemory.getMMeteoServerHost();

		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
	else
	{
		serverUrl = sms->getMessage().substring(10);
		serverUrl.trim();
				
		Logger::logF("Set server url to : ");
		Logger::logLn(serverUrl);
		
		_ArduinoMemory.setMMeteoServerHost(serverUrl);
		_ArduinoMemory.save();

		message = "SET URL : " + _ArduinoMemory.getMMeteoServerHost();		
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
		
	Logger::logLnF("<-onOnGetSetServerCommandReceived");
}

void MMeteo::onOnGetSetPortCommandReceived(SMS *sms, bool get)
{
	String message;
	int serverPort;
	
	Logger::logLnF("->onOnGetSetPortCommandReceived");
	
	if( get == true )
	{
		message = "SERVER PORT : " + String( _ArduinoMemory.getMMeteoServerPort() );

		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
	else
	{
		sscanf( sms->getMessage().c_str(), "SETPORT %d", &serverPort );
				
		Logger::logF("Set server url to : ");
		Logger::logLn( String( serverPort ) );
		
		_ArduinoMemory.setMMeteoServerPort( serverPort );
		_ArduinoMemory.save();

		message = "SET URL : " + _ArduinoMemory.getMMeteoServerHost();		
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
		
	Logger::logLnF("<-onOnGetSetPortCommandReceived");
}

void MMeteo::onStartCommandReceived(SMS *sms)
{
	sbyte userIndex = -1;
	
	Logger::logLnF("->onStartCommandReceived");
	
	if( sms->getMessage().startsWith("START ALL") )
	{
		Logger::logLnF("START ALL");

		for( ubyte i=0; i < MAX_USERS ; i++)
		{
			if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{
				_ArduinoMemory.getMMeteoUser(i)->setAlertDisabled( false );
				
				if( _ArduinoGsmShield.telsAreEquals( sms->getRemoteTel(), _ArduinoMemory.getMMeteoUser(i)->getTel() ) == false )
				{
					_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), RESOURCE_ALERT_ON );
				}
			}
		}
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_ALERT_ON  );
		
		_ArduinoMemory.save();
	}
	else
	{
		userIndex = _ArduinoMemory.findUserIndex( sms->getRemoteTel() );
		
		if( userIndex == -1 )
		{
			_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_INVALID_NUMBER );
		}
		else
		{
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_ALERT_ON );
			
			_ArduinoMemory.getMMeteoUser(userIndex)->setAlertDisabled( false );
			
			_ArduinoMemory.save();
		}
	}
	
	
	Logger::logLnF("<-onStartCommandReceived");
}

void MMeteo::onStopCommandReceived(SMS *sms)
{
	sbyte userIndex = -1;
	
	Logger::logLnF("->onStopCommandReceived");
	
	if( sms->getMessage().startsWith("STOP ALL") )
	{
		Logger::logLnF("STOP ALL");
		
		for( ubyte i=0; i < MAX_USERS ; i++)
		{
			if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{
				_ArduinoMemory.getMMeteoUser(i)->setAlertDisabled( true );
				
				if( _ArduinoGsmShield.telsAreEquals( sms->getRemoteTel(), _ArduinoMemory.getMMeteoUser(i)->getTel() ) == false )
				{
					_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), RESOURCE_ALERT_OFF );
				}
			}
		}

		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_ALERT_OFF );
		
		_ArduinoMemory.save();
	}
	else
	{
		userIndex = _ArduinoMemory.findUserIndex( sms->getRemoteTel() );
		
		if( userIndex == -1 )
		{
			_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_INVALID_NUMBER);
		}
		else
		{
			_ArduinoMemory.getMMeteoUser(userIndex)->setAlertDisabled( true );
			
			_ArduinoMemory.save();
			
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_ALERT_OFF  );
		}
	}	
	
	Logger::logLnF("<-onStopCommandReceived");
}

void MMeteo::onDailyCommandReceived(SMS *sms)
{
	char message[MAX_LENGTH_SMS_MESSAGE];
	char txtTemp[5];
	sbyte userIndex = -1;
	ubyte hCount = 0;
	String responseMessage;
	
	Logger::logLnF("->onDailyCommandReceived");
	
	userIndex = _ArduinoMemory.findUserIndex( sms->getRemoteTel() );

	if( userIndex != -1 )
	{
		if ( sms->getMessage().startsWith("DAILY START") )
		{
			_ArduinoMemory.getMMeteoUser(userIndex)->setDaily(true);
			
			_ArduinoMemory.save();
			
			responseMessage = "Alertes activees : ";
			
			for( ubyte h =0; h < 24 ; h++)	
			{
				if( _ArduinoMemory.getMMeteoUser(userIndex)->getAlerts()[h] == true )
				{
					responseMessage = responseMessage + String( h ) + String(" ");
				}
			}
			
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), responseMessage );

		}
		else if( sms->getMessage().startsWith("DAILY STOP") )
		{
			_ArduinoMemory.getMMeteoUser(userIndex)->setDaily(false);
			
			_ArduinoMemory.save();
			
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_DAILY_ALARM_OFF );
		}
		else if( sms->getMessage().startsWith("DAILY SET") )
		{
			_ArduinoMemory.getMMeteoUser(userIndex)->setDaily(true);
					
			for( ubyte h =0; h < 24 ; h++)	
			{
				_ArduinoMemory.getMMeteoUser(userIndex)->getAlerts()[h] = false;
			}
			
			int hour = -1;
			
			for(int i=sms->getMessage().length()-1; i>=10 ; i--)
			{
				if( isdigit( sms->getMessage().charAt(i) ) != 0 &&  isdigit( sms->getMessage().charAt(i-1) ) != 0 )
				{
					hour = digitToInt( sms->getMessage().charAt(i-1) ) * 10;
					
					hour = hour + digitToInt( sms->getMessage().charAt( i ) );
					
					i--;
				}
				else if( isdigit( sms->getMessage().charAt(i) ) != 0 )
				{
					hour = digitToInt( sms->getMessage().charAt( i ) );
				}
				else
				{
					hour = -1;
				}
				
				if( hour != -1 )
				{
					Logger::log( String( hour ) );
					Logger::log( " " );
					
					if ( hour < 24 && hour >=0 )
					{	
						_ArduinoMemory.getMMeteoUser(userIndex)->getAlerts()[hour] = true;
					}
				}
			}
			
			Logger::logLn( ";" );
			
			_ArduinoMemory.save();
			
			responseMessage = "Alertes activees : ";
			
			for( ubyte h =0; h < 24 ; h++)	
			{
				if( _ArduinoMemory.getMMeteoUser(userIndex)->getAlerts()[h] == true )
				{
					responseMessage = responseMessage + String( h ) + String(" ");
				}
			}
			
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), responseMessage );
		}
		else
		{
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_DAILY_BAD_COMMAND );
		}
	}
	else
	{
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_INVALID_NUMBER);
	}
	
	Logger::logLnF("<-onDailyCommandReceived");
}

int MMeteo::digitToInt( char d )
{
	char str[2];

	str[0] = d;
	str[1] = '\0';
	
	return (int) strtol(str, NULL, 10);
}

void MMeteo::onSetCommandReceived(SMS *sms)
{
	String responseMessage;
	char message[MAX_LENGTH_SMS_MESSAGE];
	char txtTemp[5];
	sbyte userIndex = -1;
	
	bool isTemp = false;
	
	Logger::logLnF("->onSetCommandReceived");
	
	if( responseMessage.reserve(MAX_LENGTH_SMS_MESSAGE) == 0 )
	{
		Logger::logLnF("void MMeteo::onSetCommandReceived(SMS *sms)  responseMessage.reserve(MAX_LENGTH_SMS_MESSAGE) == 0  !!!");
	}
	
	userIndex = _ArduinoMemory.findUserIndex( sms->getRemoteTel() );

	if( userIndex != -1 )
	{
		if( sms->getMessage().charAt(4) == 'T' )
		{
			isTemp = true;
		}
		else if( sms->getMessage().charAt(4) == 'H' ) 
		{
			isTemp = false;
		}
		
		if( sms->getMessage().charAt(4) == 'T' || sms->getMessage().charAt(4) == 'H' )
		{
			int min = 0;
			int max = 100;
			
			if( sms->getMessage().charAt(4) == 'T' )
			{
				sscanf( sms->getMessage().c_str(), "SET T MIN %d MAX %d", &min, &max );
				
				_ArduinoMemory.setTempLimitMin(min);
				_ArduinoMemory.setTempLimitMax(max);
			}
			else
			{
				sscanf( sms->getMessage().c_str(), "SET H MIN %d MAX %d", &min, &max );
				
				_ArduinoMemory.setHumLimitMin(min);
				_ArduinoMemory.setHumLimitMax(max);
			}
			
			responseMessage = String("New limit ") + sms->getMessage().charAt(4) + String(" min : ") + String( min ) + String(", max : ") + String( max ); 
			
			for(ubyte i =0; i<MAX_USERS; i++)
			{
				if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
				{
					_ArduinoMemory.getMMeteoUser(i)->setAlertDisabled(false);
					_ArduinoGsmShield.sendSMS(_ArduinoMemory.getMMeteoUser(i)->getTel(), responseMessage);
				}
			}
			
			// reseting alert last time
			_LastAlertSentTime = 0;
			
			_ArduinoMemory.save();
		}
		else
		{
			_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), RESOURCE_SET_BAD_COMMAND );
		}
	}
	else
	{
		_ArduinoGsmShield.sendSMS(sms->getRemoteTel(), RESOURCE_INVALID_NUMBER);
	}
	
	
	Logger::logLnF("<-onSetCommandReceived");
}


void MMeteo::ckeckAlerts()
{
	float temperature = 0;
	float humidity = 0;
	String alertMessage;
		
	Logger::logLnF("->ckeckAlerts");
	
	if( alertMessage.reserve(MAX_LENGTH_SMS_MESSAGE) == 0 )
	{
		Logger::logLnF("MMeteo::ckeckAlerts() alertMessage.reserve(MAX_LENGTH_SMS_MESSAGE) == 0 !!!");
	}
	
	temperature = _DHT.readTemperature();
	humidity = _DHT.readHumidity();
	
	/*
	if( temperature <= (_ArduinoMemory.getTempLimitMin() + 1 ) || temperature >= ( _ArduinoMemory.getTempLimitMax() - 1 ) || 
		humidity <= ( _ArduinoMemory.getHumLimitMin() + 1 )  ||  humidity >= ( _ArduinoMemory.getHumLimitMax() - 1 )
	  )
	{
		for( ubyte i=0; i < MAX_USERS ; i++)
		{
			if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{
				if( _ArduinoMemory.getMMeteoUser(i)->isAlertDisabled() == true )
				{
					_ArduinoMemory.getMMeteoUser(i)->setAlertDisabled( false );	
				}
			}
		}
	}
	*/
	
	if( temperature <= _ArduinoMemory.getTempLimitMin() || temperature >= _ArduinoMemory.getTempLimitMax() || 
		humidity <= _ArduinoMemory.getHumLimitMin()  ||  humidity >= _ArduinoMemory.getHumLimitMax()
	  )
	{
		// last sms was sent 10 minutes earlier ?
		if( ( now() - _LastAlertSentTime ) > 600 )
		{
			//sprintf(monSMS,"Alerte %s\nT = %d.%d C\nH = %d%%",nomVar[i],(int)etat.temp,((int)(etat.temp*10))%10,(int)etat.humid);
				
			if( ( temperature <= _ArduinoMemory.getTempLimitMin() || temperature >= _ArduinoMemory.getTempLimitMax() ) && ( humidity <= _ArduinoMemory.getHumLimitMin()  ||  humidity >= _ArduinoMemory.getHumLimitMax() ) )
			{
				alertMessage = String("Alerte Temperature et Humidite :\nT = ") + String( (int)temperature ) + String(".") + String(((int)(temperature*10))%10) + String(" C\nH = ") + String((int)humidity) + String("%");
			}
			else if( temperature <= _ArduinoMemory.getTempLimitMin() || temperature >= _ArduinoMemory.getTempLimitMax() )
			{
				alertMessage = String("Alerte Temperature :\nT = ") + String( (int)temperature ) + String(".") + String(((int)(temperature*10))%10) + String(" C\nH = ") + String((int)humidity) + String("%");
			}
			else if( humidity <= _ArduinoMemory.getHumLimitMin()  ||  humidity >= _ArduinoMemory.getHumLimitMax() )
			{
				alertMessage = String("Alerte Humidite :\nT = ") + String( (int)temperature ) + String(".") + String(((int)(temperature*10))%10) + String(" C\nH = ") + String((int)humidity) + String("%");
			}
						
			alertMessage = alertMessage + String("\n") + twoDigitIntFormat( day() ) + "/" + twoDigitIntFormat( month() ) + "/" + year() + "\n";
			alertMessage = alertMessage + twoDigitIntFormat( hour() ) + ":" + twoDigitIntFormat( minute() ) + "\n";
						
			for( ubyte i=0; i < MAX_USERS ; i++)
			{
				if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true  &&  _ArduinoMemory.getMMeteoUser(i)->isAlertDisabled() == false )
				{
					_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), alertMessage );
				}
			}
			
			_LastAlertSentTime = now();
		}
	}

	if( _LastAlertSentTime > now() )
	{
		_LastAlertSentTime = now();
	}
	
	Logger::logLnF("<-ckeckAlerts");
}

String MMeteo::twoDigitIntFormat( int value )
{
	String formattedString;
	
	if( value < 10 )
	{
		formattedString = "0" + String(value);
	}
	else
	{
		formattedString = String(value);
	}
	
	return formattedString;
}

void MMeteo::onRebootCommandReceived(SMS *sms)
{
	Logger::logLnF("->onRebootCommandReceived");
	
	SMS *responseSms = new SMS();
	
	Logger::logLnF("->sendSMS");
	
	responseSms->setMessage(String("Rebooting device."));
	responseSms->setRemoteTel(sms->getRemoteTel());
	responseSms->setSent(false);
	responseSms->setValid(true);
	
	for(int i=0; i < MAX_SEND_SMS_TRIES ; i++)
	{
		if( _ArduinoGsmShield.immediateSendSMS(responseSms) == true )
		{
			break;
		}
	}
	
	delete responseSms;
	
	reboot();
	
	Logger::logLnF("<-onRebootCommandReceived");
}

void MMeteo::onVersionCommandReceived(SMS *sms)
{
	String responseMessage;
	
	Logger::logLnF("->onVersionCommandReceived");
	
	responseMessage = String(GIT_VERSION) + String("\n") + String(SOFT_VERSION) + String("\n") + String(HOST_NAME_ID) + String("\n") + String(DATE_VERSION);
	
	_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), responseMessage);	
	
	Logger::logLnF("<-onVersionCommandReceived");
}

void MMeteo::onSaveCommandReceived(SMS *sms)
{
	Logger::logLnF("->onSaveCommandReceived");
	
	_ArduinoMemory.save();
	
	_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), sms->getMessage() );
		
	Logger::logLnF("<-onSaveCommandReceived");
}

void MMeteo::onBroadcastCommandReceived(SMS *sms)
{
	Logger::logLnF("->onBroadcastCommandReceived");

	if( sms->getMessage().length() > 10 )
	{
		String messageToBroadcast = sms->getMessage().substring(10);

		Logger::logF("Broadcasting : ");
		Logger::logLnF("messageToBroadcast");
		
		for( ubyte i=0; i < MAX_USERS ; i++)
		{
			if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{
				_ArduinoGsmShield.sendSMS( _ArduinoMemory.getMMeteoUser(i)->getTel(), messageToBroadcast );
			}
		}
	}
	
	Logger::logLnF("<-onBroadcastCommandReceived");
}


void MMeteo::onListCommandReceived(SMS *sms)
{
	Logger::logLnF("->onListCommandReceived");
	
	String responseMessage = "Users :\n";
	
	for( ubyte i=0; i < MAX_USERS ; i++)
		{
			if( _ArduinoMemory.getMMeteoUser(i)->isActive() == true )
			{					
				responseMessage = responseMessage + _ArduinoMemory.getMMeteoUser(i)->getTel() + "\n";
			}
		}
	
	_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), responseMessage );
		
	Logger::logLnF("<-onListCommandReceived");
}

void MMeteo::onEepromResetCommandReceived(SMS *sms)
{
	Logger::logLnF("->onEepromResetCommandReceived");
	
	_ArduinoMemory.resetEeprom();
	
	reboot();
	
	Logger::logLnF("<-onEepromResetCommandReceived");
}

void MMeteo::onOnGetSetGprsFreqCommandReceived(SMS *sms, bool get)
{
	String message;
	int index;
	
	Logger::logLnF("->onOnGetSetGprsFreqCommandReceived");
	
	if( get == true )
	{
		message = "GPRS FREQ : " + String( _ArduinoMemory.getNextGprsCallIndex() );

		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
	else
	{
		sscanf( sms->getMessage().c_str(), "SETGPRS %d", &index );
				
		Logger::logF("Set gprs index to : ");
		Logger::logLn( String( index ) );
		
		_ArduinoMemory.setNextGprsCallIndex( index );
		
		Logger::logLn( String( _ArduinoMemory.getNextGprsCallIndex() ) );
		
		_ArduinoMemory.save();

		message = "SET GPRS INDEX : " + _ArduinoMemory.getNextGprsCallIndex();		
		
		_ArduinoGsmShield.sendSMS( sms->getRemoteTel(), message );
	}
		
	
	Logger::logLnF("<-onOnGetSetGprsFreqCommandReceived");
}
