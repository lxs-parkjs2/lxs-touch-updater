#ifndef _CINTERFACE_H_
#define _CINTERFACE_H_

#include "common_define.h"

#ifndef WIN_TEST
#include <dirent.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#else
#include <WinBase.h>
#endif




class CInterface
{
    public:
    CInterface();
    ~CInterface();

    int         OpenDevice();
    int         CloseDevice();
    bool        IsOpen();    
    int         GetInformation();
    int         SetMode(int nMode);
    int         DownloadFW_IAP(unsigned char* pBuffer, int nSize);
    
    //ChromeOS support functions
    int         OpenDeviceFast();
    int         ReadVersionOnly(tSWIPFirmwareSectionVersion_t* pVersion);
    int         GetCurrentProductID(uint16_t* pid);

    //private function
    private:    
    int         HID_Write(unsigned short Command, unsigned short Length, unsigned char* pData);
    int         HID_Read(unsigned short Command, unsigned short Length, unsigned char* pReadData);
    int         WriteCommand(int hDeviceHandle, unsigned char Flag, unsigned short Command, unsigned short Length, unsigned char* pData);
    int         ReadData(int hDeviceHandle, unsigned char Flag, unsigned short Command, unsigned short Length, unsigned char* pReadData);    
    int         WaitForReadyStatus();
    int 	    GetFWResult(int nChk);
    BOOL		GetVerifyMode();

    //private value
    private:
    int                                 m_hDeviceHandle;
    bool                                m_bConnect;
    bool                                m_bDFUP;
    bool                                m_b4KMode;
    int                                 m_nXSize;
    int                                 m_nYSize;
    tSWIPInterface_t                    m_tInterface;
    tSWIPPanel_t                        m_tPanel;
    tSWIPFirmwareSectionVersion_t       m_tVersion;
    tProtocolCRC                        m_tCRC;
    tProtocolSetter                     m_tSetter;  
};

#endif // _CINTERFACE_H_
