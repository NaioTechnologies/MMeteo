#include "GprsData.h"

#include <Arduino.h>

GprsData::GprsData() :
	_ClientMessage(),
	_ServerMessage(),
	_Valid(false),
	_ServerMessageReceived(false)
{
	_ClientMessage.reserve(GPRS_MAX_MESSAGE_LENGHT);
	_ServerMessage.reserve(GPRS_MAX_MESSAGE_LENGHT);
	_ClientMessage[0] = '\0';
	_ServerMessage[0] = '\0';
}

String GprsData::getClientMessage()
{
	return _ClientMessage;
}

void GprsData::setClientMessage(const String& clientMessage)
{
	_ClientMessage = clientMessage;
}

String GprsData::getServerMessage()
{
	return _ServerMessage;
}

void GprsData::setServerMessage(const String& serverMessage)
{
	_ServerMessage = serverMessage;
}

bool GprsData::isValid()
{
	return _Valid;
}

void GprsData::setValid(bool value)
{
	_Valid = value;
}

bool GprsData::isServerMessageReceived()
{
	return _ServerMessageReceived;
}

void GprsData::setServerMessageReceived(bool value)
{
	_ServerMessageReceived = value;
}
		

