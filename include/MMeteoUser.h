#ifndef MMETEOUSER_H
#define MMETEOUSER_H

#include "Types.h"

class MMeteoUser
{
	public:
		MMeteoUser();
		
		MMeteoUser(const MMeteoUser&)=delete;
		MMeteoUser& operator = (const MMeteoUser&)=delete;
		
		ubyte getIndex();
		String getTel();
		bool* getAlerts();
		bool isActive();
		bool isDaily();
		bool isAlertDisabled();
		
		void setIndex(ubyte index);
		//void setTel(char tel[LEN_TEL]);
		void setTel(const String& tel);
		//void setAlerts(char alerts[24]);
		void setAlerts(bool alerts[24]);
		void setActive(bool active);
		void setDaily(bool active);
		void setAlertDisabled(bool active);
		
		void resetUser();
	private:
		ubyte _Index;
		//char _Tel[LEN_TEL];
		String _Tel;
		bool _Alerts[24];
		bool _Active;
		bool _Daily;
		bool _AlertDisabled;
};

#endif