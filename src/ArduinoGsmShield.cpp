#include "ArduinoGsmShield.h"
#include "Time.h"
#include "Logger.h"

ArduinoGsmShield::ArduinoGsmShield() :
		_GSM(true),
		_GSM_SMS(),
		_GSMModem(),
		_GSMClient(),
		_GPRS(),
		_SMSToBeSentNextTime(),
		_GSMScanner()
{
	for( ubyte i = 0 ; i < MAX_SMS_TO_BE_SENT_NEXT_TIME ; i++ )
	{
		_SMSToBeSentNextTime[i] = nullptr;
	}
}

void ArduinoGsmShield::setArduinoMemory( ArduinoMemory* arduinoMemory )
{
	_ArduinoMemory = arduinoMemory;
}

void ArduinoGsmShield::reboot()
{
	Logger::logLnF("Rebooting");
	
	delay(1000UL);
	
	asm volatile(" jmp 0x00");
}

String ArduinoGsmShield::getImei()
{
	Logger::logLnF("GSM modem connection.");
	
	while( _GSMModem.begin() != 1 )
	{
		Logger::logLnF("try to connect to GSM modem.");
	}
	
	Logger::logLnF("GSM modem connected.");
	
	String imei = _GSMModem.getIMEI();

	imei.replace("\n","");
	
	//strcpy(imeiToReturn, imei.c_str());
	
	Logger::logLnF("IMEI :");
	Logger::logLn(imei);
	
	return imei;
}

ubyte ArduinoGsmShield::getSMSToBeSentNextTimeCount()
{
	ubyte smsToBeSentNextTimeCount = 0;
	
	for(ubyte i=0; i<MAX_SMS_TO_BE_SENT_NEXT_TIME; i++ )
	{
		if( _SMSToBeSentNextTime[i] != nullptr )
		{
			if( _SMSToBeSentNextTime[i]->isSent() == false )
			{
				smsToBeSentNextTimeCount++;
			}
		}
	}
	
	return smsToBeSentNextTimeCount;
}



void ArduinoGsmShield::checkSMSToBeSentNextTime()
{
	Logger::logLnF("->checkSMSToBeSentNextTime");
	
	ubyte failInSession = 0;
	
	for(ubyte i=0; i<MAX_SMS_TO_BE_SENT_NEXT_TIME; i++ )
	{
		if( _SMSToBeSentNextTime[i] != nullptr )
		{
			if( _SMSToBeSentNextTime[i]->isSent() == false )
			{
				if( immediateSendSMS(_SMSToBeSentNextTime[i]) == false )
				{
					failInSession++;
					
					Logger::logLnF("Was not possible to send this round, sms will be sent next time.");	
					
					if( _SMSToBeSentNextTime[i]->getSentFailCount() >= MAX_SEND_SMS_FALURES )
					{	
						Logger::logLnF("Too many send failures, delete the sms.");	
						
						_SMSToBeSentNextTime[i]->setSent(true);
						
						delete _SMSToBeSentNextTime[i];
					
						_SMSToBeSentNextTime[i] = nullptr;
					}
				}
				else
				{
					_SMSToBeSentNextTime[i]->setSent(true);
					
					delete _SMSToBeSentNextTime[i];
					
					_SMSToBeSentNextTime[i] = nullptr;
				}
			}
		}
		
		if( failInSession >= MAX_SEND_SMS_FAIL_IN_SESSION )
		{
			Logger::logLnF("Too many fail in this session, better to skip it.");	
			
			break;
		}
	}
	
	Logger::logLnF("<-checkSMSToBeSentNextTime");
}	


void ArduinoGsmShield::sendDataByGprs(const String& serverHostUrl, GprsData *gprsData)
{
	unsigned long startMillis;
	char receivedMessage[MAX_LENGTH_SMS_MESSAGE];
	ubyte index = 0;
	
	Logger::logLnF("->sendGprsData");
	
	Logger::logF("Connecting to server :");
	Logger::logLn(serverHostUrl);
	
	if( _GSMClient.connect(serverHostUrl.c_str(), GPRS_SERV_PORT) )
	{
		Logger::logLnF("Connected.");
		
		_GSMClient.println(gprsData->getClientMessage());
		
		Logger::logLnF("Sending data : ");
		
		Logger::logLn(gprsData->getClientMessage());
				
		Logger::logLnF("Receiving data : ");
		
		startMillis = millis();
		
		while (    (_GSMClient.available() || _GSMClient.connected() )
				&& ( millis() < startMillis + 10000UL && ( millis() >= startMillis || millis() < 10000UL ) ) 
				&& index < MAX_LENGTH_SMS_MESSAGE)
		{
			if ( _GSMClient.available() )
			{
				receivedMessage[index] = _GSMClient.read();
				Serial.print(receivedMessage[index]);
				index++;
			}
		}
		
		receivedMessage[index] = '\0';
		
		if( index > 0 )
		{
			gprsData->setServerMessageReceived(true);
		}
		
		Logger::logLnF("");
		
		gprsData->setServerMessage(String(receivedMessage));
		
		_ArduinoMemory->setBytesReceived(  _ArduinoMemory->getBytesReceived() + ((unsigned long)(strlen( receivedMessage ))) );
		
		_ArduinoMemory->setBytesSent(  _ArduinoMemory->getBytesSent() + ((unsigned long)(strlen( gprsData->getClientMessage().c_str() )))  );
	}
	
	Logger::logLnF("Disconnecting.");
	
	_GSMClient.stop();
	
	Logger::logLnF("Disconnected.");
	
	Logger::logLnF("<-sendGprsData");
}

bool ArduinoGsmShield::connectGSM()
{
	Logger::logLnF("PIN CONNECTION : ");
	
	_GSM.begin((char*)PINNUMBER, true, false);
	
	for( ubyte tries = 0 ; tries < MAX_PIN_CONNECTION_TRIES ; tries++ )
	{
		if( _GSM.getStatus() == GSM_READY )
		{
			Logger::logLnF("PIN OK");
			break;
		}
		else
		{
			Logger::logLnF("Waiting for PIN OK");
			
			if( tries >= ( MAX_PIN_CONNECTION_TRIES - 1 ) )
			{
				//Logger::logLnF("too much tries rebooting.");
				
				Logger::logLnF("CAN'T CONNECT TO PIN GSM");
				
				//reboot();
				
				return false;
			}
		}
		
		delay(1000UL);
	}
	
	delay(1000UL);
	
	return true;
}

void ArduinoGsmShield::disconnectGSM()
{
	Logger::logLnF("GSM DISCONNECTING");
	
	_GSM.shutdown();
	
	Logger::logLnF("GSM DISCONNECTED");
	
	delay(1000);
}

bool ArduinoGsmShield::connectGPRS()
{
	Logger::logLnF("ATTACH GPRS : ");

	_GPRS.attachGPRS((char*)GPRS_APN, (char*)GPRS_LOGIN, (char*)GPRS_PASSWORD, false);
	
	for(ubyte tries=0;tries<MAX_GPRS_CONNECTION_TRIES; tries++)
	{
		if( _GPRS.getStatus() == GPRS_READY )
		{
			Logger::logLnF("GPRS ATTACHED");
			
			break;
		}
		else
		{
			Logger::logLnF("Waiting for GPRS Attached");
			
			if( tries >= (MAX_GPRS_CONNECTION_TRIES - 1))
			{
				Logger::logLnF("CAN'T ATTACH TO GPRS");
				
				delay(1000UL);
				
				return false;
			}
		}
		
		delay(1000UL);
	}
	
	delay(1000UL);
	
	return true;
}

void ArduinoGsmShield::disconnectGPRS()
{
	Logger::logLnF("GPRS DETACHING");
	
	_GPRS.detachGPRS(false);
	
	for(ubyte tries=0;tries<MAX_GPRS_CONNECTION_TRIES; tries++)
	{
		if( _GPRS.getStatus() == GSM_READY )
		{
			Logger::logLnF("GPRS DETACHED");
			
			delay(1000UL);
			
			break;
		}
		else
		{
			Logger::logLnF("Waiting for GPRS Detached");
			
			if( tries >= (MAX_GPRS_CONNECTION_TRIES - 1))
			{
				Logger::logLnF("CAN'T DETACH FROM GPRS");
				
				delay(1000UL);
				
				break;
			}
		}
		
		delay(1000UL);
	}
}	

bool ArduinoGsmShield::isValidNumber(const String& number)
{
	if (number.length() == 10)
	{
		if( number.startsWith("06") || number.startsWith("07") )
		{
			return true;
		}
	}
	
	if (number.length() == 12)
	{
		if( number.startsWith("+336") || number.startsWith("+337") )
		{
			return true;
		}
	}
	
	//TODO:Test 00336 00337
	
	return false;
}


bool ArduinoGsmShield::isPendingSMS()
{
	bool isPendingSMS = false;
	
	isPendingSMS = _GSM_SMS.available();
	
	return isPendingSMS;	
}



//bool ArduinoGsmShield::sendSMS(char tel[LEN_TEL], char message[MAX_LENGTH_SMS_MESSAGE])
bool ArduinoGsmShield::sendSMS(const String& tel, const String& message)
{
	bool availableMemory = false;
	SMS *sms = new SMS();
	
	Logger::logLnF("->sendSMS");
	
	sms->setMessage(message);
	sms->setRemoteTel(tel);
	sms->setSent(false);
	sms->setValid(true);
	
	for(ubyte i=0; i<MAX_SMS_TO_BE_SENT_NEXT_TIME; i++)
	{
		if( _SMSToBeSentNextTime[i] == nullptr )
		{
			_SMSToBeSentNextTime[i] = sms;
			
			availableMemory = true;
			
			break;
		}
	}
	
	if( availableMemory == false )
	{
		Logger::logLnF("No more free space trying to send immediatly.");
		
		if( immediateSendSMS(sms) == false )
		{
			Logger::logLnF("Was not possible to send, sms lost.");	
		}
		
		delete sms;
	}
	
	//delay(500UL);
	
	Logger::logLnF("<-sendSMS");
}

bool ArduinoGsmShield::immediateSendSMS(SMS *sms)
{
	bool smsWasSent = false;
	ubyte tries = 0;
	
	Logger::logLnF("->immediateSendSMS()");	
	
	String remoteTel = sms->getRemoteTel();
	
	if( !remoteTel.startsWith("+33") )
	{
		if( remoteTel.startsWith("0") )	
		{
			remoteTel =  "+33" + remoteTel.substring(1);
		}
		else
		{
			remoteTel =  "+33" + remoteTel;
		}
	}
	
	Logger::logLnF( "sending to " );
	Logger::log( remoteTel );
	Logger::logLnF( " : " );
	Logger::logLn( sms->getMessage() );
	
	_GSM_SMS.beginSMS(remoteTel.c_str());
	
	_GSM_SMS.print(sms->getMessage().c_str());

	int endSms = _GSM_SMS.endSMS();
	
	while( endSms != 1 && tries < MAX_SEND_SMS_TRIES )
	{
		delay(500UL);
		
		Logger::logLn( String( "endSMS : " ) + String( endSms ) ); 
		
		endSms = _GSM_SMS.endSMS();
		
		tries++;
	}
	
	Logger::logLn( String( "endSMS : " ) + String( endSms ) ); 	
	
	
	/*
	mySerial.println("AT+CMGF=1"); // set the SMS mode to text
	delay(2500);
	mySerial.write("AT+CMGS=");
	mySerial.write(34); //ASCII of “
	mySerial.write("+1234567890");
	mySerial.write(34);
	mySerial.write(13);
	mySerial.write(10);
	delay(2500);
	mySerial.println("Yahoo, You are Successful...!"); //this is the message to be sent
	delay(2500);
	mySerial.write(26);
	mySerial.write(13);
	mySerial.write(10);//Ascii code of ctrl+z to send the message
	*/
	
	_GSM_SMS.flush();
	
	if( tries == MAX_SEND_SMS_TRIES)
	{
		Logger::logLnF("SMS was not sent.");
		smsWasSent = false;
		
		sms->setSentFailCount( sms->getSentFailCount() + 1 );
	}
	else
	{
		smsWasSent = true;
		
		_ArduinoMemory->setSmsSent(  _ArduinoMemory->getSmsSent() + 1 );
		
		delay(1000UL);
	}
	
	Logger::logLnF("<-immediateSendSMS()");	
	
	return smsWasSent;
}

SMS* ArduinoGsmShield::getPendindSMS()
{
	SMS *sms = new SMS();
	ubyte messageIndex = 0;
	char messageChar[MAX_LENGTH_SMS_MESSAGE];
	char c;
	char remoteTel[LEN_TEL];
	
	Logger::logLnF("->getPendindSMS()");
	
	_GSM_SMS.remoteNumber(remoteTel, LEN_TEL - 1);
	
	_ArduinoMemory->setSmsReceived(  _ArduinoMemory->getSmsReceived() + 1 );
	
	sms->setRemoteTel(String(remoteTel));
	
	if( !isValidNumber(String(remoteTel)) )
	{
		Logger::logLnF("Not valid remote sms");	

		_GSM_SMS.flush();		
		
		sms->setValid(false);
	}
	else
	{
		sms->setValid(true);
		
		do
		{
			c = _GSM_SMS.read();
			
			messageChar[messageIndex] = c;
			messageIndex++;
			
			if (messageIndex >= MAX_LENGTH_SMS_MESSAGE)
			{
				messageIndex = 0;
			}
		}
		while(c>0);
	
		if( messageIndex > 0 )
		{
			messageChar[messageIndex-1] ='\0';
		}
	
		_GSM_SMS.flush();
		
		sms->setMessage( messageChar );
	}
	
	Logger::logF("Receiving from ");
	Logger::log(sms->getRemoteTel());
	Logger::logF(" : ");
	Logger::logLn(sms->getMessage());
	
	Logger::logLnF("<-getPendindSMS()");
	
	delay(500);
	
	return sms;
}

bool ArduinoGsmShield::telsAreEquals(const String& tel1, const String& tel2)
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

bool ArduinoGsmShield::sendNumSmsToOdroidServer()
{
	Logger::logLnF("->sendNumSmsToOdroidServer");

	sendSMS(RESOURCE_ODROID_SERVER_PHONE_NUMBER, String("NUM"));
	
	Logger::logLnF("<-sendNumSmsToOdroidServer");
}

String ArduinoGsmShield::getSignalStrength()
{
	return _GSMScanner.getSignalStrength();
}
