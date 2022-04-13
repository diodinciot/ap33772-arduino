// AP33772 I2C Command Tester
// by Joseph Liang

// Demonstrates how to use AP33772 Register with I2C interface
// For AP33772 user guide, please refer to https://www.diodes.com/

// Created 11 April 2022


#ifndef __AP33772__
#define __AP33772__

#define CMD_SRCPDO          0x00
#define CMD_PDONUM          0x1C
#define CMD_STATUS          0x1D
#define CMD_MASK            0x1E
#define CMD_VOLTAGE         0x20
#define CMD_CURRENT         0x21
#define CMD_TEMP            0x22
#define CMD_RDO             0x30


typedef struct {
  union {
    struct {
      byte isReady: 1;
      byte isSuccess: 1;
      byte isNewpdo: 1;
      byte reserved: 1;
      byte isOvp: 1;
      byte isOcp: 1;
      byte isOtp: 1;
      byte isDr: 1;
    };
    byte readStatus;
  };
  byte readVolt;    //LSB: 80mV
  byte readCurr;    //LSB: 24mA
  byte readTemp;    //unit: 1C
} AP33772_STATUS_T;


typedef struct {
  union {
    struct {
      byte newNegoSuccess: 1;
      byte newNegoFail: 1;
      byte negoSuccess: 1;
      byte negoFail: 1;
      byte reserved_1: 4;
    };
    byte negoEvent;
  };
  union {
    struct {
      byte ovp: 1;
      byte ocp: 1;
      byte otp: 1;
      byte dr: 1;
      byte reserved_2: 4;
    };
    byte protectEvent;
  };
} EVENT_FLAG_T;


typedef struct {
  union {
    struct {
      unsigned int maxCurrent: 10;    //unit: 10mA
      unsigned int voltage: 10;       //unit: 50mV
      unsigned int reserved_1: 10;
      unsigned int type: 2;
    } fixed;
    struct {
      unsigned int maxCurrent: 7;     //unit: 50mA
      unsigned int reserved_1: 1;
      unsigned int minVoltage: 8;     //unit: 100mV
      unsigned int reserved_2: 1;
      unsigned int maxVoltage: 8;     //unit: 100mV
      unsigned int reserved_3: 3;
      unsigned int apdo: 2;
      unsigned int type: 2;
    } pps;
    struct {
      byte byte0;
      byte byte1;
      byte byte2;
      byte byte3;
    };
    unsigned long data;
  };
} PDO_DATA_T;


typedef struct {
  union {
    struct {
      unsigned int maxCurrent: 10;    //unit: 10mA
      unsigned int opCurrent: 10;     //unit: 10mA
      unsigned int reserved_1: 8;
      unsigned int objPosition: 3;
      unsigned int reserved_2: 1;
    } fixed;
    struct {
      unsigned int opCurrent: 7;      //unit: 50mA
      unsigned int reserved_1: 2;
      unsigned int voltage: 11;       //unit: 20mV
      unsigned int reserved_2: 8;
      unsigned int objPosition: 3;
      unsigned int reserved_3: 1;
    } pps;
    struct {
      byte byte0;
      byte byte1;
      byte byte2;
      byte byte3;
    };
	unsigned long data;
  };
} RDO_DATA_T;

#endif
