#ifndef ARDUINOGSMSHIELD_H
#define ARDUINOGSMSHIELD_H

#include <stdlib.h>

#include <GSM.h>
#include <avr/pgmspace.h>

#include "Types.h"
#include "SMS.h"
#include "GprsData.h"
#include "ArduinoMemory.h"

class ArduinoGsmShield
{
	public:
		ArduinoGsmShield();
		
		ArduinoGsmShield(const ArduinoGsmShield&)=delete;
		ArduinoGsmShield& operator = (const ArduinoGsmShield&)=delete;

		
		bool connectGSM();
		void disconnectGSM();
		bool connectGPRS();
		void disconnectGPRS();
		
		bool isPendingSMS();
		
		SMS* getPendindSMS();
		
		//bool sendSMS(char tel[LEN_TEL], char message[MAX_LENGTH_SMS_MESSAGE]);
		bool sendSMS(const String& tel, const String& message);
		
		bool immediateSendSMS(SMS *sms);
		
		//void sendDataByGprs(char serverHostUrl[URL_LENGTH], GprsData *gprs);
		void sendDataByGprs(const String& serverHostUrl, GprsData *gprs);
		
		bool sendNumSmsToOdroidServer();
		
		//bool isValidNumber(char number[LEN_TEL]);
		bool isValidNumber(const String& number);
		
		//bool telsAreEquals(char tel1[], char tel2[]);
		bool telsAreEquals(const String& tel1, const String& tel2);
		
		// get getSignalStrength from scanner
		String getSignalStrength();
		
		//char* getImei();
		String getImei();
		
		void checkSMSToBeSentNextTime();

		ubyte getSMSToBeSentNextTimeCount();
		
		void setArduinoMemory( ArduinoMemory* arduinoMemory );
	private:
		
		// Class for accessing GSM
		GSM _GSM;
		// Class for handling SMS
		GSM_SMS _GSM_SMS;
		// Sensor class
		//DHT dht(SENSOR_PIN, DHT22);
		// Stores modem
		GSMModem _GSMModem;
		// Gprs Client
		GSMClient _GSMClient;
		// Gprs
		GPRS _GPRS;
		//scanner
		GSMScanner _GSMScanner;
		
		SMS* _SMSToBeSentNextTime[MAX_SMS_TO_BE_SENT_NEXT_TIME];
		
		void reboot();
		
		ArduinoMemory* _ArduinoMemory;
};

#endif