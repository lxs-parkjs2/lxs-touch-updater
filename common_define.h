#pragma once

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>


//this define word is only use windows.
//if you run any other OS text it.
//#define WIN_TEST

#define MSB(x)										(((x)>>8)&0xFF)
#define LSB(x)										((x)&0xFF)

#define BYTE                                        unsigned char
#define BOOL                                        unsigned char
#define MAX_PATH                                    (256)

#define TARGET_DEVICE_BUS_TYPE  BUS_I2C
#define TARGET_DEVICE_VID       0x1FD2
#define TARGET_DEVICE_PID       0x5008
#define TARGET_DEVICE_PID_TYPE_2 0x5007
#define TARGET_DEVICE_DFUP_VID  0x29BD
#define TARGET_DEVICE_DFUP_PID  0x5357
#define TARGET_BUFFER_SIZE      (64)

#define TRUE                    (1)
#define FALSE                   (0)

#pragma pack(push, 1)
#define PROTOCOL_NAME_  8

//command
#define FLITFCTRL_COMMAND_FLASH_WRITE					0x03
#define FLITFCTRL_COMMAND_FLASH_4KB_UPDATE_MODE		0x04
#define FLITFCTRL_COMMAND_FLASH_GET_VERIFY			0x05
#define M_WATCH_DOG_RESET                       	0x11

#define SWIP_REG_ADDR_INFO_PANEL                 0x0110
#define SWIP_REG_ADDR_INFO_VERSION               0x0120
#define SWIP_REG_ADDR_INFO_INTEGRITY             0x0140
#define SWIP_REG_ADDR_INFO_INTERFACE             0x0150

#define SWIP_REG_ADDR_CTRL_GETTER                0x0600
#define SWIP_REG_ADDR_CTRL_SETTER                0x0610
#define SWIP_REG_ADDR_CTRL_DFUP_SET_FLAG         0x0623

#define SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD		 0x1400

#define SWIP_REG_ADDR_PARAMETER_BUFFER           0x6000

typedef enum
{
    M_TOUCH_NORMAL,
    M_TOUCH_DIAG,
    M_TOUCH_DFUP
} eProtocolMode_t;

typedef enum
{
    RS_READY    = 0xA0,
    RS_NONE     = 0x05,
    RS_LOG      = 0x77,
    RS_IMAGE	= 0xAA
} eProtocolReadyStatus_t;

typedef struct
{
    char vcProtocolName[PROTOCOL_NAME_];
} tSWIPInterface_t;

typedef struct
{
    uint16_t usXResolution;
    uint16_t usYResolution;
    uint8_t ucXNode_;
    uint8_t ucYNode_;
} tSWIPPanel_t;

typedef struct
{
    uint16_t bcdBootVer;
    uint16_t bcdCoreVer;
    uint16_t bcdAppVer;
    uint16_t bcdParaVer;
}  tSWIPFirmwareSectionVersion_t;

typedef struct
{
	uint8_t bMode;
	uint8_t bEventTriggerType;

} tProtocolSetter;

typedef struct
{
	uint8_t bReadyStatus;
	bool bEventReady;
} tProtocolGetter_t;

typedef struct
{
	uint32_t addr;
	uint16_t size;
	uint8_t status;
	uint8_t cmd;
}  tFlashIAPCmd_t;

typedef struct
{
	uint32_t nBootCRC;
	uint32_t nAppCRC;
} tProtocolCRC;

#pragma pack(pop)