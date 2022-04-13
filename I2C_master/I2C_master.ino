// AP33772 I2C Tester
// by Joseph Liang

// Demonstrates how to use AP33772 Register with I2C interface
// For AP33772 user guide, please refer to https://www.diodes.com/

// Created 11 April 2022


#include <Wire.h>
#include "ap33772.h"

#define SLAVE_ADDRESS        0x51
#define READ_BUFF_LENGTH     30
#define WRITE_BUFF_LENGTH    6
#define SRCPDO_LENGTH        28

byte readBuf[READ_BUFF_LENGTH] = {0};
byte writeBuf[WRITE_BUFF_LENGTH] = {0};
byte numPDO = 0;          // source PDO number
byte indexPDO = 0;        // PDO index, start from index 0
int reqPpsVolt = 0;       // requested PPS voltage, unit:20mV
bool startTesting = 0;

AP33772_STATUS_T ap33772_status = {0};
EVENT_FLAG_T event_flag = {0};
RDO_DATA_T rdoData = {0};
PDO_DATA_T pdoData[7] = {0};


void initWriteBuf()
{
  for(byte i=0 ; i<WRITE_BUFF_LENGTH ; i++)
  {
    writeBuf[i] = 0;
  }
}

void i2c_write(byte slvAddr, byte cmdAddr, byte len)
{
  Wire.beginTransmission(slvAddr);    // transmit to device SLAVE_ADDRESS
  Wire.write(cmdAddr);                // sets the command register
  Wire.write(writeBuf, len);          // write data with len
  Wire.endTransmission();             // stop transmitting
  initWriteBuf();
}

void initReadBuf()
{
  for(byte i=0 ; i<READ_BUFF_LENGTH ; i++)
  {
    readBuf[i] = 0;
  }
}

void i2c_read(byte slvAddr, byte cmdAddr, byte len)
{
  byte i = 0;
  
  initReadBuf();
  Wire.beginTransmission(slvAddr);    // transmit to device SLAVE_ADDRESS
  Wire.write(cmdAddr);                // sets the command register
  Wire.endTransmission();             // stop transmitting

  Wire.requestFrom(slvAddr, len);     // request len bytes from peripheral device
  if (len <= Wire.available()) {      // if len bytes were received
    while (Wire.available()) {
      readBuf[i] = (byte) Wire.read();
      i++;
    }
  }
}

void writeRDO()
{
  writeBuf[3] = rdoData.byte3;
  writeBuf[2] = rdoData.byte2;
  writeBuf[1] = rdoData.byte1;
  writeBuf[0] = rdoData.byte0;
  i2c_write(SLAVE_ADDRESS, CMD_RDO, 4);    // CMD: Write RDO
}

void displayMainMenu()
{
  Serial.print("Select PDO [1~");
  Serial.print(numPDO);
  Serial.println("] or [9] Reset AP33772 :");
}

void checkStatus()
{
  i2c_read(SLAVE_ADDRESS, CMD_STATUS, 1);    // CMD: Read Status
  ap33772_status.readStatus = readBuf[0];

  if(ap33772_status.isOvp)
    event_flag.ovp = 1;
  if(ap33772_status.isOcp)
    event_flag.ocp = 1;

  if(ap33772_status.isReady)          // negotiation finished
  {
    if(ap33772_status.isNewpdo)       // new PDO
    {
      if(ap33772_status.isSuccess)    // negotiation success
        event_flag.newNegoSuccess = 1;
      else
        event_flag.newNegoFail = 1;
    }
    else
    {
      if(ap33772_status.isSuccess)
        event_flag.negoSuccess = 1;
      else
        event_flag.negoFail = 1;
    }
  }
  delay(10);
}

void eventProcess()
{
  if(event_flag.newNegoSuccess)
  {
    event_flag.newNegoSuccess = 0;
    Serial.println("===============================================");
    Serial.println("AP33772 I2C Tester");
    Serial.println();
    Serial.print("Read Status = 0x");
    Serial.print(ap33772_status.readStatus, HEX);
    Serial.println();
    
    i2c_read(SLAVE_ADDRESS, CMD_PDONUM, 1);    // CMD: Read PDO Number
    numPDO = readBuf[0];
    Serial.print("Source PDO Number = ");
    Serial.print(numPDO);
    Serial.println();

    i2c_read(SLAVE_ADDRESS, CMD_SRCPDO, SRCPDO_LENGTH);    // CMD: Read PDOs
    // copy PDOs to pdoData[]
    for(byte i=0 ; i<numPDO ; i++)
    {
      pdoData[i].byte0 = readBuf[i*4];
      pdoData[i].byte1 = readBuf[i*4+1];
      pdoData[i].byte2 = readBuf[i*4+2];
      pdoData[i].byte3 = readBuf[i*4+3];
    }

    // display PDO information
    Serial.println();
    for(byte i=0 ; i<numPDO ; i++)
    {
      if((pdoData[i].byte3 & 0xF0) == 0xC0)         // PPS PDO
      {
        Serial.print("PDO[");
        Serial.print(i+1);        // PDO position start from 1
        Serial.print("] - PPS : ");
        Serial.print((float)(pdoData[i].pps.minVoltage) * 100 / 1000);
        Serial.print("V~");
        Serial.print((float)(pdoData[i].pps.maxVoltage) * 100 / 1000);
        Serial.print("V @ ");
        Serial.print((float)(pdoData[i].pps.maxCurrent) * 50 / 1000);
        Serial.println("A");
      }
      else if((pdoData[i].byte3 & 0xC0) == 0x00)    // Fixed PDO
      {
        Serial.print("PDO[");
        Serial.print(i+1);
        Serial.print("] - Fixed : ");
        Serial.print((float)(pdoData[i].fixed.voltage) * 50 / 1000);
        Serial.print("V @ ");
        Serial.print((float)(pdoData[i].fixed.maxCurrent) * 10 / 1000);
        Serial.println("A");
      }
    }
    Serial.println("===============================================");
    displayMainMenu();
  }

  if(event_flag.negoSuccess)
  {
    event_flag.negoSuccess = 0;
    delay(100);
    i2c_read(SLAVE_ADDRESS, CMD_VOLTAGE, 1);    // CMD: Read VOLTAGE
    ap33772_status.readVolt = readBuf[0];
    i2c_read(SLAVE_ADDRESS, CMD_CURRENT, 1);    // CMD: Read CURRENT
    ap33772_status.readCurr = readBuf[0];
    
    Serial.print("  >> Success : ");
    Serial.print(ap33772_status.readVolt*80);
    Serial.print("mV @ ");
    Serial.print(ap33772_status.readCurr*24);
    Serial.print("mA ; Status = 0x");
    Serial.println(ap33772_status.readStatus, HEX);
    Serial.println();
    displayMainMenu();
  }
}

void setup() {
  Wire.begin();            // join i2c bus (address optional for master)
  Serial.begin(115200);    // start serial for output
  Serial.setTimeout(10);
}

void loop() {
  byte cmd=0;
  byte input=0;

  if (Serial.available())
  {
    input = Serial.read();
  }
  
  if (input == 't')    // Enter 't' to start Tester
  {
    Serial.println("Starting Test ...");
    startTesting = 1;
  }

  if(startTesting)
  {
    checkStatus();
    eventProcess();
  }

  // Enter 1~9 vaild
  if ((input > '0') && (input <= '9'))
  {
    input = input - '0';
    
    // Select PDO
    if ((input > 0) && (input <= numPDO))
    {
      indexPDO = input - 1;    // Start from index 0
      if((pdoData[indexPDO].byte3 & 0xF0) == 0xC0)    // PPS PDO
      {
        Serial.print(" > Set PDO[");
        Serial.print(input);
        Serial.println("] - PPS PDO");

        // set request PPS voltage = max voltage
        reqPpsVolt = pdoData[indexPDO].pps.maxVoltage * 5;    // convert unit: 100mV -> 20mV

        // set pps rdoData
        rdoData.pps.objPosition = input;
        rdoData.pps.opCurrent = pdoData[indexPDO].pps.maxCurrent;
        rdoData.pps.voltage = reqPpsVolt;
        writeRDO();
      }
      else if((pdoData[indexPDO].byte3 & 0xC0) == 0x00)    // Fixed PDO
      {
        Serial.print(" > Set PDO[");
        Serial.print(input);
        Serial.println("] - Fixed PDO");

        // set fixed rdoData
        rdoData.fixed.objPosition = input;
        rdoData.fixed.maxCurrent = pdoData[indexPDO].fixed.maxCurrent;
        rdoData.fixed.opCurrent = pdoData[indexPDO].fixed.maxCurrent;
        writeRDO();
      }
    }
    else
    {
      // Enter '9' for reset command test
      if(input == 9)
      {
        Serial.println(" > Reset AP33772, please wait... ");
        Serial.println();
        writeBuf[0] = 0x00; 
        writeBuf[1] = 0x00;
        writeBuf[2] = 0x00;
        writeBuf[3] = 0x00;
        i2c_write(0x51, 0x30, 4);
        // restsrt I2C module
        Wire.end();
        delay(1000);
        Wire.begin();
      }
      else
      {
        Serial.print(" > Invalid PDO number : ");
        Serial.println(input);
        Serial.println();
        displayMainMenu();
      }
    }
  }
}
