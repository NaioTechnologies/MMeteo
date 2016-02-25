#include <WString.h>

#ifndef TYPES_H
#define TYPES_H

#define SERIAL_OUT_ON

#ifndef ubyte
typedef	unsigned char ubyte;
#endif

#ifndef sbyte
typedef	signed char sbyte;
#endif
	
static const ubyte LEN_TEL = 13;		
	
//#ifndef P(str)
	//#define P(str) (strcpy_P(p_buffer, PSTR(str)), p_buffer)
//#endif


#define MATOOMA

#ifdef MATOOMA
	static const char* PINNUMBER = { "" };
	static const char* GPRS_APN = { "matooma" };// replace your GPRS APN
#else
	static const char* PINNUMBER = { "1234" };
	static const char* GPRS_APN = { "free" };// replace your GPRS APN
#endif


static const char* GPRS_LOGIN = { "" }; // replace with your GPRS login
static const char* GPRS_PASSWORD = { "" }; // replace with your GPRS password 	

#define SENSOR_PIN 5
// ################################################
static const ubyte GPRS_DATA_SEND_FREQUENCY_MINUTES = 15;		
// ################################################

static const ubyte MAX_USERS = 3;		

static const int USER_STORAGE_START_INDEX = 512;		

// ################################################
static const ubyte CURRENT_EEPROM_DATA_VERSION = 6;		
// ################################################
static const ubyte MAX_SENSOR_READ_TRIES = 3;

static const ubyte MAX_PIN_CONNECTION_TRIES = 30;

static const ubyte GPRS_SERV_HOST_MAX_SIZE = 32;

//#define GPRS_SERV_PORT 4243 

static const ubyte GPRS_MAX_MESSAGE_LENGHT = 164;
static const ubyte MAX_GPRS_CONNECTION_TRIES = 5;
static const ubyte IMEI_LENGTH = 20;
static const ubyte MESSAGE_LENGTH = 32;
static const ubyte URL_LENGTH = 32;

static const ubyte MAX_LENGTH_SMS_MESSAGE = 100;	

// number of max unsend sms stored
static const ubyte MAX_SMS_TO_BE_SENT_NEXT_TIME = 12;

// how many tries in a sending session : async count for waiting endsms results
static const ubyte MAX_SEND_SMS_TRIES = 4;

// how many session the sms will be kept in to send state before being deleted
static const ubyte MAX_SEND_SMS_FALURES = 6;

// how many times we should try to read pending sms
static const ubyte TEST_SMS_PENDING_MAX_TRIES = 2;

// considering there is a trouble when in a same session we had too many fail to skip and try later
static const ubyte MAX_SEND_SMS_FAIL_IN_SESSION = 2;

// max sms we can read in the session
static const ubyte MAX_SMS_RECEIVING = 10;

static const ubyte MAX_SMS_NUM_SENT_COUNT = 2;

static const ubyte DHT_HUMIDITY_AVERAGE_MEASURES = 5;
static const float DHT_HUMIDITY_MEDIAN_MARGIN = 6;

static const ubyte MAX_LOG_MESSAGE_LENGTH = 64;

static const char RESOURCE_GPRS_DEFAULT_IMEI[IMEI_LENGTH] = { "XXXXXXXXXXXXXXXX\0" };

static const char RESOURCE_ODROID_SERVER_PHONE_NUMBER[LEN_TEL] = { "+33782239176" };
//static const char RESOURCE_ODROID_SERVER_PHONE_NUMBER[LEN_TEL] = { "+33607753407" };

static const char RESOURCE_MMETEO_NOT_A_NUMBER[LEN_TEL] = { "NOTANUMBER" };
static const char RESOURCE_UNKNOW_COMMAND_MESSAGE[MESSAGE_LENGTH] = { "Commande non valide.\0" };
static const char RESOURCE_NO_MORE_USER_AVAILABLE[MESSAGE_LENGTH] = { "Plus d'emplacement disponible.\0" };
static const char RESOURCE_GPRS_DEFAULT_SERV_URL[URL_LENGTH] = { "m2m.naio-technologies.com\0" };
static const int GPRS_SERV_PORT = 4243;

static const char RESOURCE_USER_RESET[MESSAGE_LENGTH] = { "Reset de votre numero.\0" };
static const char RESOURCE_USER_ALREADY_ACTIVE[MESSAGE_LENGTH] = { "Numero deja actif.\0" };
static const char RESOURCE_INVALID_NUMBER[MESSAGE_LENGTH] = { "Numero non valide.\0" };

/*
static const String RESOURCE_GPRS_DEFAULT_IMEI =  "";// "XXXXXXXXXXXXXXXX";
static const String RESOURCE_ODROID_SERVER_PHONE_NUMBER = "";// "+33782239176";
static const String RESOURCE_MMETEO_NOT_A_NUMBER = "";// "NOTANUMBER";
static const String RESOURCE_UNKNOW_COMMAND_MESSAGE = "";// "Commande non valide.";
static const String RESOURCE_NO_MORE_USER_AVAILABLE = "";//"Plus d'emplacement disponible.";
static const String RESOURCE_GPRS_DEFAULT_SERV_URL ="";// "private.naio-technologies.com";
static const String RESOURCE_USER_RESET = "";//"Reset de votre numero.";
static const String RESOURCE_USER_ALREADY_ACTIVE = "";//"Numero deja actif.";
static const String RESOURCE_INVALID_NUMBER = "";//"Numero non valide.";
*/

/*
static const String RESOURCE_ALERT_OFF = "";//"Alertes OFF";
static const String RESOURCE_ALERT_ON = "";//"Alertes ON";
static const String RESOURCE_ALARM_OFF = "";//"Alarmes OFF";
static const String RESOURCE_ALARM_ON = "";//"Alarmes ON";
*/

static const char RESOURCE_ALERT_OFF[MESSAGE_LENGTH] = { "Alertes OFF\0" };
static const char RESOURCE_ALERT_ON[MESSAGE_LENGTH] = { "Alertes ON\0" };
static const char RESOURCE_ALARM_OFF[MESSAGE_LENGTH] = { "Alarmes OFF\0" };
static const char RESOURCE_ALARM_ON[MESSAGE_LENGTH] = { "Alarmes ON\0" };



static const char RESOURCE_DAILY_ALARM_OFF[MESSAGE_LENGTH] = { "Alarmes quotiennes OFF\0" };
static const char RESOURCE_DAILY_BAD_COMMAND[MESSAGE_LENGTH] = { "Commande daily invalide.\0" };
static const char RESOURCE_SET_BAD_COMMAND[MESSAGE_LENGTH] = { "Commande set invalide.\0" };
static const char RESOURCE_OFF[MESSAGE_LENGTH] = { "OFF\0" };

/*
static const String RESOURCE_DAILY_ALARM_OFF = "Alarmes quotiennes OFF";
static const String RESOURCE_DAILY_BAD_COMMAND = "Commande daily invalide.";
static const String RESOURCE_OFF = "OFF";
*/

#endif