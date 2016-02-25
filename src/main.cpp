//=============================================================================
//
//  Copyright (C)  2013  Naio Technologies
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//=============================================================================


//=============================================================================
// I N C L U D E   F I L E S

#include <Arduino.h>

#include "MMeteo.h"

//=============================================================================
// C O N S T A N T (S)   &   L O C A L   C O D E

// Constants for the main loop timings
const unsigned int FREQ = 5;
unsigned long lastLoopTime;
unsigned long lastLoopUsefulTime;
unsigned long loopRefTime;


//-----------------------------------------------------------------------------

MMeteo _MMeteo;

//-----------------------------------------------------------------------------
//
void setup()
{

    //Serial.begin( 115200 );
	Serial.begin(9600);
	
	_MMeteo.setup();
}

//-----------------------------------------------------------------------------
//
void loop()
{
	_MMeteo.loop();
	
    Serial.flush();
}
