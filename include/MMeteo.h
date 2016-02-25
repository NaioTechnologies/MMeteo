#ifndef MMETEO_H
#define MMETEO_H

#include "Time.h"
#include "DHT.h"

#include "ArduinoGsmShield.h"
#include "ArduinoMemory.h"
#include "GprsData.h"

class MMeteo
{
	public:
		MMeteo();
		MMeteo(const MMeteo&)=delete;
		MMeteo& operator = (const MMeteo&)=delete;
		
		void setup();
		void loop();
	private:
		ArduinoGsmShield _ArduinoGsmShield;
		ArduinoMemory _ArduinoMemory;
		DHT _DHT;
		
		bool _Started;
		bool _GprsFailedLastTime;
		
		ubyte _NextGprsCallIndex;
		ubyte _NextAlertSendIndex;
		ubyte _NextMemorySaveIndex;
		
		ubyte _SmsNumSentCount;
		
		ubyte _TempAlertOnLevel;
		ubyte _HumAlertOnLevel;
		ubyte _TempWarningOnLevel;
		ubyte _HumWarningOnLevel;		
		
		time_t _LastAlertSentTime;
		
		void reboot();
		int digitToInt( char d );
		String twoDigitIntFormat( int value );
		
		bool timeToSendGprsData();
		bool timeToSendAlertSMS();
		bool timeToSaveMemory();
		
		void manageReceivedSms(SMS *sms);
		
		void ckeckAlerts();

		GprsData* buildGprsDataToSend();

		//void sendDataCommand(char tel[LEN_TEL]);
		void sendDataCommand(const String& tel);
		
		void onServerDataReceived(GprsData *gprsData);
		
		void onInvalidCommandReceived(SMS *sms);
		void onNumCommandReceived(SMS *sms);
		
		void onDataCommandReceived(SMS *sms);
		void onAddCommandReceived(SMS *sms);
		void onStatusCommandReceived(SMS *sms);
		void onResetCommandReceived(SMS *sms);
		void onDeleteCommandReceived(SMS *sms);
		void onOnOffCommandReceived(SMS *sms, bool on);
		void onOnGetSetServerCommandReceived(SMS *sms, bool get);
		void onStartCommandReceived(SMS *sms);
		void onStopCommandReceived(SMS *sms);
		void onDailyCommandReceived(SMS *sms);
		void onSetCommandReceived(SMS *sms);
		void onRebootCommandReceived(SMS *sms);
		void onVersionCommandReceived(SMS *sms);
		void onSaveCommandReceived(SMS *sms);		
		void onBroadcastCommandReceived(SMS *sms);	
		void onListCommandReceived(SMS *sms);	
		void onEepromResetCommandReceived(SMS *sms);	
		void onOnGetSetPortCommandReceived(SMS *sms, bool get);
		void onOnGetSetGprsFreqCommandReceived(SMS *sms, bool get);
};

#endif