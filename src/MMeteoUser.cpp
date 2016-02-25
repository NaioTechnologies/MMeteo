#include "MMeteoUser.h"
#include "Logger.h"

MMeteoUser::MMeteoUser() :
		_Index(0),
		_Tel(),
		_Alerts(),
		_Active(false),
		_Daily(false),
		_AlertDisabled(false)
{
	if( _Tel.reserve(LEN_TEL) == 0 )
	{
		Logger::logLnF(" MMeteoUser::MMeteoUser()  _Tel.reserve(LEN_TEL) == 0 !!! ");
	}
}

void  MMeteoUser::resetUser()
{
	_Active = false;
	_Tel[0] = '\0';
}

ubyte MMeteoUser::getIndex()
{
	return _Index;
}

String MMeteoUser::getTel()
{
	return _Tel;
}

bool* MMeteoUser::getAlerts()
{
	return _Alerts;
}

bool MMeteoUser::isActive()
{
	return _Active;
}

bool MMeteoUser::isDaily()
{
	return _Daily;
}

bool MMeteoUser::isAlertDisabled()
{
	return _AlertDisabled;
}

void MMeteoUser::setIndex(ubyte index)
{
	_Index = index;
}

void MMeteoUser::setTel(const String& tel)
{
	_Tel = tel;
}

void MMeteoUser::setAlerts(bool alerts[24])
{
	for(ubyte i=0;i<24;i++)
	{
		_Alerts[i] = alerts[i];
	}
}

void MMeteoUser::setActive(bool active)
{
	_Active = active;
}

void MMeteoUser::setDaily(bool daily)
{
	_Daily = daily;
}

void MMeteoUser::setAlertDisabled(bool alertDisabled)
{
	_AlertDisabled = alertDisabled;
}
