#===============================================================================
#
#   Cmake for Arduino
#
#===============================================================================


#-------------------------------------------------------------------------------
#
#	CMake compatibility issues
#
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
MARK_AS_ADVANCED( CMAKE_BACKWARDS_COMPATIBILITY )

#-------------------------------------------------------------------------------
#
#   Arduino Toolchain for cross-compilation
#
Set( CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake )


#-------------------------------------------------------------------------------
#
#  Setup Project
#
Project( Arduino C CXX )


#-------------------------------------------------------------------------------
#
#   Default board and port, when not specified
#
Set( ARDUINO_DEFAULT_PORT /dev/ttyACM1 )
Set( ARDUINO_DEFAULT_SERIAL 115200 )
#Set( ARDUINO_BOARD uno )
Set( ARDUINO_BOARD mega2560 )

Print_board_list()
Print_programmer_list()
Print_board_settings( ${ARDUINO_BOARD} ) 


#-------------------------------------------------------------------------------
#
#   Source & Include directories       
#
INCLUDE_DIRECTORIES(    ${PROJECT_SOURCE_DIR}/include  )
Set( INCLUDE_DIR        ${PROJECT_SOURCE_DIR}/include )
Set( SOURCE_DIR         ${PROJECT_SOURCE_DIR}/src )

execute_process( COMMAND  ${PROJECT_SOURCE_DIR}/version.sh ${PROJECT_SOURCE_DIR}  )

#	cp version.h.template version.h
#	git rev-parse HEAD | (read f;sed -i 's/#1#/'$$f'/g' version.h)
#	( hostname ; hostid ) | (read f;read g;sed -i 's/#2#/'$$f' '$$g'/g' version.h)
#	date +"%Y-%m-%d-%H-%M-%S" | (read f;sed -i 's/#3#/'$$f'/g' version.h)

add_definitions(-Os)
add_definitions(-felide-constructors)
add_definitions(-std=c++11)


#-------------------------------------------------------------------------------
#
#   Library build process

Set( EEPROM_HDRS    /usr/share/arduino/libraries/EEPROM/EEPROM.h )
Set( EEPROM_SRCS    /usr/share/arduino/libraries/EEPROM/EEPROM.cpp )
#Set( EEPROM_BOARD   uno )
Set( EEPROM_BOARD   mega2560 )

Generate_Arduino_Library( EEPROM )

#-------------------------------------------------------------------------------
#
#   Executable build process
#
Set( ${PROJECT_NAME}_HDRS
    ${INCLUDE_DIR}/Time.h
    ${INCLUDE_DIR}/DHT.h
    ${INCLUDE_DIR}/SMS.h
    ${INCLUDE_DIR}/Logger.h
    ${INCLUDE_DIR}/GprsData.h
    ${INCLUDE_DIR}/ArduinoGsmShield.h
    ${INCLUDE_DIR}/ArduinoMemory.h   
    ${INCLUDE_DIR}/MMeteo.h    
    ${INCLUDE_DIR}/MMeteoUser.h   
)

Set( ${PROJECT_NAME}_SRCS
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/Time.cpp
    ${SOURCE_DIR}/DHT.cpp
    ${SOURCE_DIR}/SMS.cpp
    ${SOURCE_DIR}/Logger.cpp
    ${SOURCE_DIR}/GprsData.cpp
    ${SOURCE_DIR}/ArduinoGsmShield.cpp
    ${SOURCE_DIR}/ArduinoMemory.cpp
    ${SOURCE_DIR}/MMeteoUser.cpp
    ${SOURCE_DIR}/MMeteo.cpp
)


Set( ${PROJECT_NAME}_LIBS EEPROM )
Set( ${PROJECT_NAME}_BOARD  ${ARDUINO_BOARD} )

Generate_Arduino_Firmware(  ${PROJECT_NAME} )

