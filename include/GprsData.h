#ifndef GPRSDATA_H
#define GPRSDATA_H

#include <string.h>


#include "Types.h"


class GprsData
{
	public:
		GprsData();
		GprsData(const GprsData&)=delete;
		GprsData& operator = (const GprsData&)=delete;
		
		String getClientMessage();
		void setClientMessage(const String& clientMessage);
		
		String getServerMessage();
		void setServerMessage(const String& clientMessage);
		
		bool isServerMessageReceived();
		void setServerMessageReceived(bool value);
		
		bool isValid();
		void setValid(bool value);
	private:
		String _ClientMessage;
		String _ServerMessage;
		bool _Valid;
		bool _ServerMessageReceived;
};

#endif