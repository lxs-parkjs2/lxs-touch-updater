#include "CLXSFwupdate.h"
#include <sys/stat.h>
#ifndef WIN_TEST
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#define sleep(x) Sleep((x)*1000)
#endif

#define FW_PATH     "/lib/firmware/lxs/Touch_Binary.img"

CLXSFwupdate::CLXSFwupdate()
{
    m_hDeviceHandle = 0;
}
CLXSFwupdate::~CLXSFwupdate()
{
    CloseDevice();
}

int         CLXSFwupdate::OpenDevice()
{
    if(!m_interface.IsOpen())
    {
        m_hDeviceHandle = m_interface.OpenDevice();
        if(m_hDeviceHandle)
        {
            int nMode = m_interface.GetInformation();
            if( nMode > 0)
            {
                m_nMode = nMode;
                fprintf(stderr, "Device open Success!!\n\n");
            }
                    
        }
        return m_hDeviceHandle;
    }
    else
    {
        fprintf(stderr, "Device is already open!!\n\n");
        return FALSE;
    }    
}

int         CLXSFwupdate::CloseDevice()
{
    if(m_interface.IsOpen())
    {
        if(m_interface.CloseDevice())
        {
            m_hDeviceHandle = 0;
            fprintf(stderr, "Device close success!!\n\n");
        }
    }  

    return 0;
}

int         CLXSFwupdate::StartDownload(const char* fwPath)
{
    // Use provided path or default
    const char* path = (fwPath != NULL) ? fwPath : FW_PATH;
    
    //read swip and dfup
    if (m_nMode == 1)
    {
        //set dfup
        m_interface.SetMode(M_TOUCH_DFUP);

        //CloseDevice();

        fprintf(stderr, "wait for reconnect\n");

        for(int i = 0; i<3; i++)
        {
            fprintf(stderr, ".");
            sleep(1);
        }
        fprintf(stderr, "\n");

        //OpenDevice();
    }
    else if (m_nMode == 2)
    {
        fprintf(stderr, "Debug Msg : Already DFUP Mode download will be start!!\n");
    }

    FILE* fp = fopen(path, "rb");
    if(fp)
    {
        unsigned char* pBuff = 0;
        struct stat st;
        stat(path, &st);
        int nSize = st.st_size;

        pBuff = new unsigned char[nSize];
        memset(pBuff, 0, nSize);

        fread(pBuff, 1, nSize, fp);

        fclose(fp);

        fprintf(stderr, "FW Download begin!\n");
        m_interface.DownloadFW_IAP(pBuff, nSize);
        fprintf(stderr, "FW Download Done!\n");
        
        delete[] pBuff;
        return TRUE;
    }
    else
    {
        fprintf(stderr, "File Not found: %s\n", path);
        return FALSE;
    }
}

int CLXSFwupdate::GetFirmwareVersionFast(char* version, int maxLen)
{
    // Minimum buffer: "Boot : XX.XX\nCore : XX.XX\nApp : XX.XX\nParam : XX.XX\n" + null = 53 bytes
    if (!version || maxLen < 53)
    {
        return FALSE;
    }

#ifndef WIN_TEST
    struct timeval start, end;
    gettimeofday(&start, NULL);
#endif
    
    tSWIPFirmwareSectionVersion_t ver;
    memset(&ver, 0, sizeof(ver));
    
    if (m_interface.ReadVersionOnly(&ver))
    {
        snprintf(version, maxLen, "Boot : %02X.%02X\nCore : %02X.%02X\nApp : %02X.%02X\nParam : %02X.%02X\n", 
                 MSB(ver.bcdBootVer), 
                 LSB(ver.bcdBootVer),
                 MSB(ver.bcdCoreVer),
                 LSB(ver.bcdCoreVer),
                 MSB(ver.bcdAppVer),
                 LSB(ver.bcdAppVer),
                 MSB(ver.bcdParaVer),
                 LSB(ver.bcdParaVer));
        
#ifndef WIN_TEST
        gettimeofday(&end, NULL);
        long elapsed = (end.tv_sec - start.tv_sec) * 1000 + 
                      (end.tv_usec - start.tv_usec) / 1000;
        
        if (elapsed >= 40)
        {
            fprintf(stderr, "Warning: Version check took %ldms (>40ms requirement)\n", elapsed);
        }
#endif
        
        return TRUE;
    }

    return FALSE;
}

int CLXSFwupdate::GetProductID(uint16_t* pid)
{
    if (!pid)
    {
        return FALSE;
    }

    return m_interface.GetCurrentProductID(pid);
}
