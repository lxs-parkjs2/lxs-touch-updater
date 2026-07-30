#ifndef _CLXS_FWUPDATE_H_
#define _CLXS_FWUPDATE_H_

#include "common_define.h"
#include "CInterface.h"

class CLXSFwupdate
{
    public:
    CLXSFwupdate();
    ~CLXSFwupdate();

    //open functions
    
    int         OpenDevice();
    int         CloseDevice();
    int         StartDownload(const char* fwPath = NULL);

    //ChromeOS support functions
    int         GetFirmwareVersionFast(char* version, int maxLen);
    int         GetProductID(uint16_t* pid);

    //value
    private:
    int         m_nMode;    //1 SWIP. 2 DFUP
    int         m_hDeviceHandle;
    CInterface  m_interface;

};

#endif // _CLXS_FWUPDATE_H_