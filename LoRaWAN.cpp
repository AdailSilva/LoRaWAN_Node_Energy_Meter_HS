/* 
 *   
 *  Project:          IoT Energy Meter with C/C++, Java/Spring, TypeScript/Angular and Dart/Flutter;
 *  About:            End-to-end implementation of a LoRaWAN network for monitoring electrical quantities;
 *  Version:          1.0;
 *  Backend Mote:     ATmega328P/ESP32/ESP8266/ESP8285/STM32;
 *  Radios:           RFM95w and LoRaWAN EndDevice Radioenge Module: RD49C;
 *  Sensors:          Peacefair PZEM-004T 3.0 Version TTL-RTU kWh Meter;
 *  Backend API:      Java with Framework: Spring Boot;
 *  LoRaWAN Stack:    LoRaMac-node Library (LoRaWAN L2 1.0.3 - Released / API via AT commands);
 *  Activation mode:  Activation by Personalization (ABP) or Over-the-Air Activation (OTAA);
 *  Author:           Adail dos Santos Silva
 *  E-mail:           adail101@hotmail.com
 *  WhatsApp:         +55 89 9 9433-7661
 *  
 *  This project was conceived using the LoRaWAN library,
 *  whose copyright belongs to the RD49C module manufacturer, the company Radioenge from Brazil.
 *  All the remaining implementation is authored by the creator of the LoRaWAN Electricity Meter project.
 *  
 *  WARNINGS:
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the “Software”), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *  the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  
 */

#include "LoRaWAN.h"
#include <string.h>


/* AT Commands Strings */
const char* str_at_commands[]
{
  "DADDR",
  "APPKEY",
  "APPSKEY",
  "NWKSKEY",
  "APPEUI",
  "ADR",
  "TXP",
  "DR",
  "DCS",
  "PNM",
  "RX2FQ",
  "RX2DR",
  "RX1DL",
  "RX2DL",
  "JN1DL",
  "JN2DL",
  "NJM",
  "NWKID",
  "CLASS",
  "JOIN",
  "NJS",
  "SENDB",
  "SEND",
  "VER",
  "CFM",
  "SNR",
  "RSSI",
  "BAT",
  "BAUDRATE",
  "NBTRIALS",
  "KEEPALIVE",
  "TXCFM",
  "CHMASK",
  "ADC",
  "GPIOC",
  "WPIN",
  "RPIN"
};


#define BUFFER_SIZE 256


/* ----- Private Variables ----- */
// static SoftwareSerial* hSerialCommand = NULL;
// static SoftwareSerial* hSerialTransp = NULL;
static HardwareSerial* hSerialCommand = NULL;
static HardwareSerial* hSerialTransp = NULL;
static uint8_t array[BUFFER_SIZE];
static char g_payload[BUFFER_SIZE];


/* ----- Public Variables ----- */


/* ----- Private Functions ----- */
void SerialFlush(HardwareSerial* hSerial)
{
  while(hSerial->available() > 0)
  {
    hSerial->read();
  }
}


/* ----- Public Function Definitions ----- */


HardwareSerial* SerialCommandsInit(uint8_t rxPin, uint8_t txPin, uint32_t baudRate, uint8_t uart)
{
  /* Filter not used baudrates */
  
  static HardwareSerial radioSerialCommands(uart);
  radioSerialCommands.begin(baudRate, SERIAL_8N1, rxPin, txPin);
  hSerialCommand = &radioSerialCommands;

  return &radioSerialCommands;
}


HardwareSerial* SerialTranspInit(uint8_t rxPin, uint8_t txPin, uint32_t baudRate, uint8_t uart)
{
  /* Filter not used baudrates */
  
  static HardwareSerial radioSerialTransp(uart);
  radioSerialTransp.begin(baudRate, SERIAL_8N1, rxPin, txPin);
  hSerialTransp = &radioSerialTransp;

  return &radioSerialTransp;
}


Status_Typedef ReceivePacketCommand(char* payload, uint8_t* payloadSize, uint32_t timeout)
{
  uint16_t waitNextByte = 20;
  uint8_t i = 0;
  
  /* Assert parameters */
  if(payload == NULL) return RAD_ERROR;
  if(payloadSize == NULL) return RAD_ERROR;
  if(hSerialCommand == NULL) return RAD_ERROR;

//  if(!hSerialCommand->isListening()) hSerialCommand->listen();

  /* Waits for reception */
  while( ((timeout > 0 ) || (i > 0)) && (waitNextByte > 0) )
  {
    if(hSerialCommand->available() > 0)
    {
      payload[i++] = hSerialCommand->read();
	    waitNextByte = 20;
    }
    
	  if(i > 0)
    {
      waitNextByte--;
    }
    timeout--;
    delay(1);
  }

  /* In case it didn't get any data */
  if((timeout == 0) && (i == 0)) return RAD_ERROR;

  /* Copies payload size */
  *payloadSize = i;
  
  return RAD_OK;
}


Status_Typedef ReceivePacketTransp(char* payload, uint8_t* payloadSize, uint32_t timeout)
{
  uint16_t waitNextByte = 20;
  uint8_t i = 0;
  
  /* Assert parameters */
  if(payload == NULL) return RAD_ERROR;
  if(payloadSize == NULL) return RAD_ERROR;
  if(hSerialTransp == NULL) return RAD_ERROR;

//  if(!hSerialTransp->isListening()) hSerialTransp->listen();
  
  /* Waits for reception */
  while( ((timeout > 0 ) || (i > 0)) && (waitNextByte > 0) )
  {
    if(hSerialTransp->available() > 0)
    {
      payload[i++] = hSerialTransp->read();
	    waitNextByte = 20;
    }
    
	  if(i > 0)
    {
      waitNextByte--;
    }
    timeout--;
    delay(1);
  }

  /* In case it didn't get any data */
  if((timeout == 0) && (i == 0)) return RAD_ERROR;

  /* Copies payload size */
  *payloadSize = i;
  
  return RAD_OK;
}


Status_Typedef InitializeOTAA(char* p_appkey, char* p_appeui)
{
  char char_one[] = "1";
  /* Checks the pointers */
  if((p_appkey == NULL) || (p_appeui == NULL))
  {
    return RAD_ERROR;
  }
  
  /* Checks the parameters */
  if((strlen(p_appkey) != 47) || (strlen(p_appeui) != 23))
  {
    return RAD_ERROR;
  }
  
  /* Configure Join mode to OTAA */
  SendAtCommand(AT_NJM, AtSet, char_one);
  delay(1000);
  
  /* Configure APPKEY */
  SendAtCommand(AT_APPKEY, AtSet, p_appkey);
  delay(1000);
  
  /* Configure APPEUI */
  SendAtCommand(AT_APPEUI, AtSet, p_appeui);
  delay(1000);
}


Status_Typedef IsJoined()
{
  char response[8];
  uint8_t size;

  SerialFlush(hSerialCommand);
  SendAtCommand(AT_NJS, AtGet, NULL);
  ReceivePacketCommand(response, &size, 500);

  /* Checks the response */
  if(response[0] == '1')
  {
    return RAD_OK;
  }
  return RAD_ERROR;
}


Status_Typedef JoinNetwork(uint8_t retries)
{
  uint16_t remaining_retries = retries;

  /* Checks if the EndDevice is already joined */
  delay(4000);
  while( (IsJoined() != RAD_OK) && ((retries == 0) || (remaining_retries > 0)) )
  {
//    delay(10000);
    if(remaining_retries)
    {
      remaining_retries--;
    }
    SendAtCommand(AT_JOIN, AtRun, NULL);
    delay(15000);
  }
  return IsJoined();
}


Status_Typedef SendString(char* string, uint8_t port)
{
  uint8_t size;

  if(string == NULL)
  {
    return RAD_ERROR;
  }
  /* Checks the message length */
  if(strnlen(string, BUFFER_SIZE) >= BUFFER_SIZE)
  {
    return RAD_ERROR;
  }

  /* Assemble the payload */
  sprintf(g_payload, "%d:%s\0", port, string);

  SendAtCommand(AT_SEND, AtSet, g_payload);
}


Status_Typedef SendRaw(char* payload)
{
  static const char at_string[] = "ATZ";
  uint8_t index = 0; //index += sizeof(array);
   memset(array, 0, BUFFER_SIZE);
  //strcpy((char*)&array[index], payload);
 // memcpy(array, at_string, 3);
 /* Appends the payload */
 
    strcpy((char*)&array[index], payload);
    index += strlen(payload);
  
  hSerialCommand->write(array, index);
  delay(500);
}


Status_Typedef SendAtCommand(AT_Commands_e command, CommandType_e command_type, char* payload)
{
  static const char at_string[] = "AT+";
  static const char at_get_string[] = "=?";
  static const char at_set_string[] = "=";
  uint8_t index = 3;

  if((payload == NULL) && (command_type == AtSet))
  {
    return RAD_ERROR;
  }

  /* Copies the AT+ prefix */
  memset(array, 0, BUFFER_SIZE);
  memcpy(array, at_string, 3);

  /* Copies the command */
  if(command >= NUMBER_OF_COMMANDS)
  {
    return RAD_ERROR;
  }
  memcpy(&array[index], str_at_commands[command], strlen(str_at_commands[command]));
  index += strlen(str_at_commands[command]);

  /* Copies the parameter */
  if(command_type == AtGet)
  {
    memcpy(&array[index], at_get_string, 2);
    index += 2;
  }
  else if(command_type == AtSet)
  {
    array[index] = at_set_string[0];
    index += 1;
    /* Appends the payload */
    strcpy((char*)&array[index], payload);
    index += strlen(payload);
  }
  else if(command_type != AtRun)
  {
    return RAD_ERROR;
  }

  array[index++] = '\r';
  array[index++] = '\n';

  /* Sends the message */
  hSerialCommand->write(array, index);
  delay(500);
}
