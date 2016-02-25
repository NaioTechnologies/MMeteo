/* DHT library 

MIT license
written by Adafruit Industries
*/

#include "DHT.h"
#include "Logger.h"
#include "Types.h"

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count)
{
	_pin = pin;
	_type = type;
	_count = count;
	firstreading = true;
	
	oldhum = 666;
	oldtemp = 666;
}

void DHT::begin(void)
{
	// set up the pins!
	pinMode(_pin, INPUT);
	digitalWrite(_pin, HIGH);
	_lastreadtime = 0;
}

//boolean S == Scale.  True == Farenheit; False == Celcius
float DHT::readTemperature(bool S)
{
	float f;
	
	Logger::logLnF("->DHT::readTemperature");

	for(int i = 0 ; i < MAX_SENSOR_READ_TRIES ; i++ )
	{
		if ( read() )
		{
			switch (_type)
			{
				case DHT11:
					f = data[2];
					
					if(S)
					{
						f = convertCtoF(f);
					}
					
					//oldtemp = f;
					//return f;
				case DHT22:
				case DHT21:
					f = data[2] & 0x7F;
					f *= 256;
					f += data[3];
					f /= 10;
					
					if (data[2] & 0x80)
					{
						f *= -1;
					}
					
					if(S)
					{
						f = convertCtoF(f);
					}
					
					//oldtemp = f;
					//return f;
			}
		}
		
		if( ( f > -30 ) &&  ( f <= 80 ) )
		{
			oldtemp = f;

			return f;
		}
	}
	
	Logger::logLnF("DHT::readTemperature fail");
	
	oldtemp = 666;
	
	return oldtemp;
}

float DHT::convertCtoF(float c)
{
	return c * 9 / 5 + 32;
}



float DHT::readHumidity(void) 
{
	float lastHumidityMeasure[DHT_HUMIDITY_AVERAGE_MEASURES];	
	float medianHumidityMeasure[DHT_HUMIDITY_AVERAGE_MEASURES];	
	float filteredHumidityMeasure[DHT_HUMIDITY_AVERAGE_MEASURES];	
	float measure = 0;
	ubyte measureCount = 0;
	
	ubyte filteredCount = 0;
	ubyte floatFilteredCount = 0;
	
	Logger::logLnF("->DHT::readHumidity");
	
	for( ubyte i = 0; i < DHT_HUMIDITY_AVERAGE_MEASURES ; i++ )
	{
		lastHumidityMeasure[i] = 666;		
	}
	
	for( ubyte i = 0; i < DHT_HUMIDITY_AVERAGE_MEASURES ; i++ )
	{
		lastHumidityMeasure[i] = directReadHumidity();		
	}
	
	for( ubyte i = 0; i < DHT_HUMIDITY_AVERAGE_MEASURES ; i++ )
	{
		if( lastHumidityMeasure[i] != 666 )
		{
			medianHumidityMeasure[measureCount] = lastHumidityMeasure[i];
			
			measureCount++;
		}
	}
	
	//Bubble sorting
	for( int x = 0 ;  x < measureCount ; x++)
	{
		for( int y = 0 ; y < measureCount-1; y++ )
		{
			if( medianHumidityMeasure[y] > medianHumidityMeasure[y+1] )
			{
				float temp = medianHumidityMeasure[y+1];
				
				medianHumidityMeasure[y+1] = medianHumidityMeasure[y];
				
				medianHumidityMeasure[y] = temp;
			}
		}
	}
	
	float median = medianHumidityMeasure[ measureCount / 2 ];
	
	// filter far from median value
	for( ubyte i = 0; i < measureCount ; i++ )
	{
		if( ( medianHumidityMeasure[i] > median - DHT_HUMIDITY_MEDIAN_MARGIN ) && ( medianHumidityMeasure[i] < median + DHT_HUMIDITY_MEDIAN_MARGIN ) )
		{
			filteredHumidityMeasure[filteredCount] = medianHumidityMeasure[i];
			
			filteredCount++;
			floatFilteredCount = floatFilteredCount + 1;
		}
	}	
	
	// compute average value of filterer values.
	for( ubyte i = 0; i < filteredCount ; i++ )
	{
		measure = measure + filteredHumidityMeasure[filteredCount];
	}
	
	if( filteredCount != 0 )
	{
		measure = measure / floatFilteredCount;
	}
	else
	{
		measure = 666;
	}
	
	return measure;
}

float DHT::directReadHumidity(void) 
{
	float f;

	for(int i = 0 ; i < MAX_SENSOR_READ_TRIES ; i++ )
	{
		if (read())
		{
			switch (_type)
			{
				case DHT11:
					f = data[0];
					//oldhum = f;
					//return f;
				case DHT22:
				case DHT21:
					f = data[0];
					f *= 256;
					f += data[1];
					f /= 10;
					//oldhum = f;
					//return f;
			}
		}
		
		if( ( f > 0 ) && ( f <= 100 ) )
		{
			oldhum = f;

			return f;
		}
	}
	
	Logger::logLnF("DHT::readHumidity fail");
	
	oldhum = 666;
	
	return oldhum;
}


boolean DHT::read(void) 
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	unsigned long currenttime;

	// pull the pin high and wait 250 milliseconds
	digitalWrite(_pin, HIGH);
	delay(250);

	currenttime = millis();
	
	if (currenttime < _lastreadtime)
	{
		// ie there was a rollover
		_lastreadtime = 0;
	}
	
	if (!firstreading && ((currenttime - _lastreadtime) < 2000))
	{
		return true; // return last correct measurement
		//delay(2000 - (currenttime - _lastreadtime));
	}
	
	firstreading = false;
	
	/*
		Serial.print("Currtime: "); Serial.print(currenttime);
		Serial.print(" Lasttime: "); Serial.print(_lastreadtime);
	*/
	
	_lastreadtime = millis();

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;
  
	// now pull it low for ~20 milliseconds
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	delay(20);
	cli();
	digitalWrite(_pin, HIGH);
	delayMicroseconds(40);
	pinMode(_pin, INPUT);

	// read in timings
	for ( i=0; i< MAXTIMINGS; i++)
	{
		counter = 0;
		while (digitalRead(_pin) == laststate)
		{
			counter++;
			delayMicroseconds(1);
			
			if (counter == 255)
			{
				break;
			}
		}
		
		laststate = digitalRead(_pin);

		if (counter == 255) break;

		// ignore first 3 transitions
		if ( ( i >= 4 ) && ( i % 2 == 0) )
		{
			// shove each bit into the storage bytes
			data[j/8] <<= 1;
			
			if (counter > _count)
			{
				data[j/8] |= 1;
			}
			
			j++;
		}

	}

	sei();

	/*
	Serial.println(j, DEC);
	Serial.print(data[0], HEX); Serial.print(", ");
	Serial.print(data[1], HEX); Serial.print(", ");
	Serial.print(data[2], HEX); Serial.print(", ");
	Serial.print(data[3], HEX); Serial.print(", ");
	Serial.print(data[4], HEX); Serial.print(" =? ");
	Serial.println(data[0] + data[1] + data[2] + data[3], HEX);
	*/

	// check we read 40 bits and that the checksum matches
	if ( (j >= 40) && (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
	{
		return true;
	}
  

	return false;

}
