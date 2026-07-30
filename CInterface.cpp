#include "CInterface.h"

tSWIPInterface_t				g_Interface = { 0, };

CInterface::CInterface()
{
    m_hDeviceHandle = 0;
    m_bConnect      = FALSE;
    m_bDFUP         = FALSE;
    m_nXSize        = 0;
    m_nYSize        = 0;
    m_b4KMode       = FALSE;
}

CInterface::~CInterface()
{
}

int CInterface::OpenDevice()
{

#ifndef WIN_TEST
    int                             hFileHandle = 0;
    struct dirent*                  pDirEntry = NULL;
    char                            szFilePath[256] = { 0, };
    int                             nError = 0;    
    int                             bFind = FALSE;
    int                             DescriptorSize = 0;
    DIR*                            pDirectory = NULL; 
    struct hidraw_devinfo           DeviceInfo = { 0, };      
    struct hidraw_report_descriptor HidReportDescriptor = { 0, };
    char                            szRawName[256] = { 0, };
    char                            szPhysical[256] = { 0, };
    

    do
    {
        pDirectory = opendir("/dev");
        if (pDirectory == NULL)
        {
            fprintf(stderr, "opendir error \n");
            break;
        }

        while ((pDirEntry = readdir(pDirectory)) != NULL)
        {
            if (strlen(pDirEntry->d_name) <= 0)
            {
                break;
            }

            if (strncmp(pDirEntry->d_name, "hidraw", 6) != 0)
            {
                continue;       
            }

            memset(szFilePath, 0, 256);
            sprintf(szFilePath, "/dev/%s", pDirEntry->d_name);
            
            nError = open(szFilePath, O_RDWR | O_NONBLOCK);
            if (nError < 0)
            {
                fprintf(stderr, "open error : %d \n", nError);
                continue;
            }

            hFileHandle = nError;
            nError = ioctl(hFileHandle, HIDIOCGRAWINFO, &DeviceInfo);
            if (nError < 0)
            {
                fprintf(stderr, "ioctl HIDIOCGRAWINFO error : %d \n", nError);
                continue;
            }

            nError = ioctl(hFileHandle, HIDIOCGRAWNAME(256), szRawName);
            if (nError < 0)
            {
                fprintf(stderr, "ioctl HIDIOCGRAWNAME error : %d \n", nError);
                continue;
            }

            nError = ioctl(hFileHandle, HIDIOCGRAWPHYS(256), szPhysical);
            if (nError < 0)
            {
                fprintf(stderr, "ioctl HIDIOCGRAWPHYS error : %d \n", nError);
                continue;
            }

            fprintf(stderr, "%s - bus type %d, VID : 0x%04X, PID : 0x%04X \n", szFilePath, DeviceInfo.bustype, DeviceInfo.vendor, DeviceInfo.product);
            fprintf(stderr, "%s - %s \n\n", szRawName, szPhysical);

            if ( ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && (DeviceInfo.vendor == TARGET_DEVICE_VID) && (DeviceInfo.product == TARGET_DEVICE_PID)) ||
            ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && (DeviceInfo.vendor == TARGET_DEVICE_VID) && (DeviceInfo.product == TARGET_DEVICE_PID_TYPE_2)) ||
             ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && (DeviceInfo.vendor == TARGET_DEVICE_VID) && (DeviceInfo.product == TARGET_DEVICE_DFUP_PID)) ||
             ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && (DeviceInfo.vendor == TARGET_DEVICE_DFUP_VID) && (DeviceInfo.product == TARGET_DEVICE_DFUP_PID))
            )
            {
                nError = ioctl(hFileHandle, HIDIOCGRDESCSIZE, &DescriptorSize);
                if (nError >= 0)
                {
                    fprintf(stderr, "DescriptorSize: %d \n", DescriptorSize);

                    HidReportDescriptor.size = DescriptorSize;
                    nError = ioctl(hFileHandle, HIDIOCGRDESC, &HidReportDescriptor);
                    if (nError >= 0)
                    {
                        fprintf(stderr, "DescriptorSize 2 : %d \n", DescriptorSize);

                        // Custom HID
                        //if ((HidReportDescriptor.value[0] == 0x06) && 
                        //    (HidReportDescriptor.value[1] == 0x00) && 
                        //    (HidReportDescriptor.value[2] == 0xFF) )
                        {
                            unsigned char   ReadBuffer[TARGET_BUFFER_SIZE] = { 0, };

                            WriteCommand(hFileHandle, 0x68, 0x0150, 0, NULL);
                            ReadData(hFileHandle, 0x69, 0x0150, 0x0E, ReadBuffer);
                            if (((ReadBuffer[4] == 'S') &&
                                (ReadBuffer[5] == 'W') &&
                                (ReadBuffer[6] == 'I') &&
                                (ReadBuffer[7] == 'P') ) ||
                                ((ReadBuffer[4] == 'D') &&
                                (ReadBuffer[5] == 'F') &&
                                (ReadBuffer[6] == 'U') &&
                                (ReadBuffer[7] == 'P') ))
                            {
                                bFind = TRUE;
                                fprintf(stderr, "Work Fine !! \n");
                                break;
                            }
                        }
                    }
                }
            }

            close(hFileHandle);
            hFileHandle = 0;

        }

        closedir(pDirectory);
        pDirectory = NULL;

    } while (FALSE);

    m_hDeviceHandle = hFileHandle;
    m_bConnect = (bool)bFind;

    return m_hDeviceHandle;
#else
    return TRUE;
#endif
}

int CInterface::CloseDevice()
{
#ifndef WIN_TEST
    if(m_hDeviceHandle)
    {
        close(m_hDeviceHandle);
        m_hDeviceHandle = 0;
        m_bConnect = FALSE;

        return TRUE;
    }
#else
    return TRUE;
#endif

    return FALSE;
}

bool CInterface::IsOpen()
{
    return m_bConnect;
}

int CInterface::HID_Write(unsigned short Command, unsigned short Length, unsigned char* pData)
{
    if(m_hDeviceHandle)
    {
        return WriteCommand(m_hDeviceHandle, 0x68, Command, Length, pData);
    }
    return FALSE;
}

int CInterface::HID_Read(unsigned short Command, unsigned short Length, unsigned char* pReadData)
{
    if(m_hDeviceHandle)
    {
        // Allocate at least TARGET_BUFFER_SIZE because ReadData() always
        // calls read(..., TARGET_BUFFER_SIZE) regardless of Length.
        int nSize = Length+4;
        int nAllocSize = (nSize < TARGET_BUFFER_SIZE) ? TARGET_BUFFER_SIZE : nSize;
        unsigned char* pBuff = new unsigned char[nAllocSize];
        memset(pBuff, 0, nAllocSize);
        WriteCommand(m_hDeviceHandle, 0x68, Command, 0, NULL);
        int nRet = ReadData(m_hDeviceHandle, 0x69, Command, nSize, pBuff);
        if(nRet)
        {
            memcpy(pReadData, &pBuff[4], Length);
        }
        delete[] pBuff;
        return nRet;
    }
    return FALSE;
}

int CInterface::WriteCommand(int hDeviceHandle, unsigned char Flag, unsigned short Command, unsigned short Length, unsigned char* pData)
{
    int             nRet = 0;
    unsigned char   WriteBuffer[TARGET_BUFFER_SIZE] = { 0, };
    unsigned short  i = 0;

    do
    {
        if (hDeviceHandle == 0)
        {
            break;
        }

        if (Flag == 0)
        {
            break;
        }

        if (Command == 0)
        {
            break;
        }

        if (Flag == 0x68)
        {
            Length += 2;
        }

        WriteBuffer[0] = 0x09;
        WriteBuffer[1] = Flag;
        WriteBuffer[2] = (unsigned char)(Length & 0x00FF);
        WriteBuffer[3] = (unsigned char)((Length & 0xFF00) >> 8);
        WriteBuffer[4] = (unsigned char)((Command & 0xFF00) >> 8);
        WriteBuffer[5] = (unsigned char)(Command & 0x00FF);
        if ((Flag == 0x68) && (pData != NULL))
        {
            for (i = 0; i < Length; i++)
            {
                WriteBuffer[6 + i] = pData[i];
            }
        }
#ifndef  WIN_TEST
        nRet = write(hDeviceHandle, WriteBuffer, TARGET_BUFFER_SIZE);
#else
        nRet = TARGET_BUFFER_SIZE;
#endif
        if (nRet == TARGET_BUFFER_SIZE)
        {
            nRet = TRUE;
        }
        else
        {
            nRet = FALSE;
        }
        
    } while (FALSE);
    
    return nRet;
}

int CInterface::ReadData(int hDeviceHandle, unsigned char Flag, unsigned short Command, unsigned short Length, unsigned char* pReadData)
{
    int             nRet = 0;
#ifdef WIN_TEST
    fprintf(stderr, "Read Data : [0x%X] Length : [%d]\n", Command, Length);
    nRet = TARGET_BUFFER_SIZE;

    //virtual data
    switch(Command)
    {
        case SWIP_REG_ADDR_INFO_INTERFACE:
        {
            memset(&g_Interface, 0, sizeof(tSWIPInterface_t));
            if (m_bDFUP == TRUE)
            {
                memcpy(g_Interface.vcProtocolName, "DFUP", 4);
            }
            else
            {
                memcpy(g_Interface.vcProtocolName, "SWIP", 4);
            }
            
            memcpy(pReadData, &g_Interface, Length);
            nRet = sizeof(tSWIPInterface_t);
            break;
        }
        default:
        break;
    }
#else
    
    fd_set          FileDesc = { 0, };
    struct timeval  ReadTimeOut = { 0, };

    do
    {
        if (hDeviceHandle == 0)
        {
            break;
        }

        if (Command == 0)
        {
            break;
        }

        if (Length > 0)
        {
            if (pReadData == NULL)
            {
                break;
            }
        }

        if (Flag == 0x69)
        {
            WriteCommand(hDeviceHandle, Flag, Command, Length, NULL);
        }

        FD_ZERO(&FileDesc);
        FD_SET(hDeviceHandle, &FileDesc);

        ReadTimeOut.tv_sec = 1; // 1sec
        ReadTimeOut.tv_usec = 0;
        nRet = select(hDeviceHandle+1, &FileDesc, NULL, NULL, &ReadTimeOut);
        if (nRet <= 0)
        {
            break;
        }

        nRet = read(hDeviceHandle, pReadData, TARGET_BUFFER_SIZE);
        if (nRet == TARGET_BUFFER_SIZE)
        {
            nRet = TRUE;
        }
        else
        {
            nRet = FALSE;
        }

    } while (FALSE);
#endif    
    return nRet;
}

//0 - data none
//1 - data swip & init
//2 - dfup
int         CInterface::GetInformation()
{
    int nType = 0;
    //check connect is good
    HID_Read(SWIP_REG_ADDR_INFO_INTERFACE, sizeof(m_tInterface), (unsigned char*)&m_tInterface);

    if(strcmp((const char*)&m_tInterface.vcProtocolName, "DFUP") == 0)
    {
        fprintf(stderr, "Debug Msg : [ Protocol Name : DFUP] \n");
        nType = 2;
    }        
    else if(strcmp((const char*)&m_tInterface.vcProtocolName, "SWIP") == 0)
    {
        fprintf(stderr, "Debug Msg : [ Protocol Name : SWIP] \n");
        nType = 1;
    }        
    else
    {
        nType = 0;
    }
    
    if(nType == 1)
    {
        //read node size
        HID_Read(SWIP_REG_ADDR_INFO_PANEL, sizeof(m_tPanel), (unsigned char*)&m_tPanel);
        m_nXSize = m_tPanel.ucXNode_;
        m_nYSize = m_tPanel.ucYNode_;

        //read version
        HID_Read(SWIP_REG_ADDR_INFO_VERSION, sizeof(m_tVersion), (unsigned char*)&m_tVersion);
        fprintf(stderr, "Debug Msg : [Boot Version : %X.%X][Fw Version : %X.%X]\n", MSB(m_tVersion.bcdBootVer), LSB(m_tVersion.bcdBootVer), MSB(m_tVersion.bcdCoreVer), LSB(m_tVersion.bcdCoreVer));

        HID_Read(SWIP_REG_ADDR_INFO_INTEGRITY, sizeof(m_tCRC), (unsigned char*)&m_tCRC);
        fprintf(stderr, "Debug Msg : [Boot CRC : %X][Fw CRC : %X]\n", m_tCRC.nBootCRC, m_tCRC.nAppCRC);

        //set normal
        SetMode(M_TOUCH_NORMAL);
    }
    
    return nType;
}

BOOL	CInterface::GetVerifyMode()
{
	tFlashIAPCmd_t	FAPCtrl;
    FAPCtrl.addr = 0;
    FAPCtrl.size = 0;
    FAPCtrl.status = 0;
    FAPCtrl.cmd = FLITFCTRL_COMMAND_FLASH_4KB_UPDATE_MODE;
    
	// Write command to flash IAP control register
	HID_Write(SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD, sizeof(FAPCtrl), (unsigned char*)&FAPCtrl);
    
    // Wait for device to process the command
    WaitForReadyStatus();
    
    // Read back the status
    HID_Read(SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD, sizeof(FAPCtrl), (unsigned char*)&FAPCtrl);

	return FAPCtrl.status;
}

int         CInterface::SetMode(int nMode)
{
    int nChk = 0;
    HID_Read(SWIP_REG_ADDR_CTRL_SETTER, sizeof(m_tSetter), (unsigned char*)&m_tSetter);

    if (nMode != m_tSetter.bMode)
	{
		m_tSetter.bMode = nMode;		
		m_tSetter.bEventTriggerType = 0;

        HID_Write(SWIP_REG_ADDR_CTRL_SETTER, sizeof(m_tSetter), (unsigned char*)&m_tSetter);
        
        nChk = WaitForReadyStatus();

        if(nMode == M_TOUCH_DFUP)
        {
            m_b4KMode = GetVerifyMode();
        }
        
        return nChk;
	}
	else
		return TRUE;
}

int         CInterface::DownloadFW_IAP(unsigned char* pBuffer, int nSize)
{
    int nRealFileLength = 0;
	int nOffset = 0;
	tFlashIAPCmd_t	FAPCtrl;
	BYTE			TEMP_BUF[50];
    bool            bInit = FALSE;
    bool            bVerify = FALSE;

	//memset(TEMP_BUF, 0xAA, sizeof(TEMP_BUF));

	nRealFileLength = nSize;

	if (nSize == (112 * 1024))
	{		
		nOffset = 0x4000;
        nRealFileLength = nSize+nOffset;
		fprintf(stderr, "Application download mode\n");
	}
	else if (nSize == (128 * 1024))
	{
		//boot		
		nOffset = 0x0;		
        nRealFileLength = nSize;
		fprintf(stderr, "Boot + Application download mode\n");
	}	

	tSWIPInterface_t	tInterface;
	HID_Read(SWIP_REG_ADDR_INFO_INTERFACE, sizeof(tSWIPInterface_t), (unsigned char*)&tInterface);

	if (strcmp((const char*)&tInterface.vcProtocolName, "DFUP") != 0)
	{
		fprintf(stderr, "Failed to switch dfu mode.\n");
		return FALSE;
	}

    if(m_b4KMode)
    {
        //4k mode
		BOOL Flash_Write_Error_Flag = FALSE;
		int Flash_Write_Error_Cnt = 0;
		uint32_t TRANSMIT_UNIT = 48;
		uint32_t FLASH_WRITE_UNIT = 4096;
		uint32_t LAST_TRANSMIT_UNIT = FLASH_WRITE_UNIT % TRANSMIT_UNIT;
		uint32_t NORMAL_TRANSMIT_SIZE = FLASH_WRITE_UNIT - LAST_TRANSMIT_UNIT;
		uint32_t crc_value_tool = 0;
		uint32_t index=0;
		uint32_t idx2;

		for(int idx=(0x00000000+nOffset); idx<nRealFileLength; idx+=FLASH_WRITE_UNIT)
		{
			if(Flash_Write_Error_Flag)
			{
				Flash_Write_Error_Flag = FALSE;
				idx   -= FLASH_WRITE_UNIT;
				index -= FLASH_WRITE_UNIT;
			}

			if((uint32_t)(idx + FLASH_WRITE_UNIT) > (uint32_t)nRealFileLength)
			{
				FLASH_WRITE_UNIT = nRealFileLength - idx;
				LAST_TRANSMIT_UNIT = FLASH_WRITE_UNIT % TRANSMIT_UNIT;
				NORMAL_TRANSMIT_SIZE = FLASH_WRITE_UNIT - LAST_TRANSMIT_UNIT;
			}

			for(idx2=0; idx2 < NORMAL_TRANSMIT_SIZE; idx2+=TRANSMIT_UNIT)
			{
				memcpy((void *)TEMP_BUF, (void *)&pBuffer[index], TRANSMIT_UNIT);
				index += TRANSMIT_UNIT;
				HID_Write(SWIP_REG_ADDR_PARAMETER_BUFFER+idx2, TRANSMIT_UNIT, TEMP_BUF);
			}

			if(LAST_TRANSMIT_UNIT != 0)
			{
				memcpy((void *)TEMP_BUF, (void *)&pBuffer[index], LAST_TRANSMIT_UNIT);
				index += LAST_TRANSMIT_UNIT;
				HID_Write(SWIP_REG_ADDR_PARAMETER_BUFFER+idx2, LAST_TRANSMIT_UNIT, TEMP_BUF);
			}

			FAPCtrl.addr   = idx;
			FAPCtrl.size   = FLASH_WRITE_UNIT;
			FAPCtrl.status = 0;
			FAPCtrl.cmd    = FLITFCTRL_COMMAND_FLASH_WRITE;
			HID_Write(SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD, sizeof(FAPCtrl), (unsigned char*)&FAPCtrl);
			if (!WaitForReadyStatus())
			{
				fprintf(stderr, "Wait Status Fail!! - flash write\n");
				return FALSE;
			}

			// Calculate simple CRC for verification
            if(bVerify)
            {            
                crc_value_tool = 0;
                for (uint32_t i = 0; i < FLASH_WRITE_UNIT; i++) {
                    crc_value_tool += pBuffer[idx-nOffset+i];
                }
                crc_value_tool = crc_value_tool ^ 0xFFFFFFFF;
                HID_Write(SWIP_REG_ADDR_PARAMETER_BUFFER, 4, (unsigned char*)&crc_value_tool);

                FAPCtrl.addr = idx;
                FAPCtrl.size = FLASH_WRITE_UNIT;
                FAPCtrl.status = 0;
                FAPCtrl.cmd = FLITFCTRL_COMMAND_FLASH_GET_VERIFY;
                HID_Write(SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD, sizeof(FAPCtrl), (unsigned char*)&FAPCtrl);
                if (!WaitForReadyStatus())
                {
                    fprintf(stderr, "Wait Status Fail!! - verify flash write\n");
                    return FALSE;
                }

                HID_Read(SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD, sizeof(FAPCtrl), (unsigned char*)&FAPCtrl);

                if(FAPCtrl.status == 0)
                {
                    Flash_Write_Error_Cnt++;
                    Flash_Write_Error_Flag = TRUE;

                    if(Flash_Write_Error_Cnt == 5)
                    {
                        fprintf(stderr, "Flash write verification failed after 5 retries\n");
                        break;
                    }
                }
                else
                {
                    Flash_Write_Error_Cnt = 0;
                }
            }

			if((idx-nOffset)%1024 == 0)
			{
				int nPer = (int)((float)(idx*100)/nSize);

				if(nPer > 100)
					nPer = 100;

				if (!bInit)
				{
					bInit = TRUE;
					fprintf(stderr, "\n4K Mode Download Progress : ");
				}

				fprintf(stderr, "%02d%%", nPer);
				fprintf(stderr, "\b\b\b");
				fflush(stderr);
				usleep(2000);
			}
		}
		
		if(Flash_Write_Error_Cnt >= 5)
		{
			return FALSE;
		}

		m_tSetter.bMode = M_WATCH_DOG_RESET;
		HID_Write(SWIP_REG_ADDR_CTRL_SETTER, sizeof(m_tSetter), (unsigned char*)&m_tSetter);
    }
    else
    {
        int nBufferOffset = 0;
        int nIdxIncrement = 16;
        int nDownloadBlock = 128;

        for (int i = nOffset; i < nRealFileLength; i += nDownloadBlock)
        {
            //Write 16 bytes at a time, 8 iterations per 128-byte block
            for (int j = 0; j < nDownloadBlock; j += nIdxIncrement, nBufferOffset += nIdxIncrement)
            {
                memcpy(TEMP_BUF, pBuffer + nBufferOffset, nIdxIncrement);
                HID_Write(SWIP_REG_ADDR_PARAMETER_BUFFER + j, nIdxIncrement, TEMP_BUF);			
            }

            FAPCtrl.addr = i;
            FAPCtrl.size = nDownloadBlock;
            FAPCtrl.status = 0;
            FAPCtrl.cmd = FLITFCTRL_COMMAND_FLASH_WRITE;
            HID_Write(SWIP_REG_ADDR_FLASH_IAP_CTRL_CMD, sizeof(FAPCtrl), (unsigned char*)&FAPCtrl);
            BOOL bChk = WaitForReadyStatus();
            if (!bChk)
            {
                fprintf(stderr, "Ready Status Failed!!\n");
                return FALSE;
            }
            
            //if(i%1024 == 0)
            {
                int nPer = (int)((float)(i * 100) / nSize);

                if (nPer > 100)
                {
                    nPer = 100;
                }

                if (!bInit)
                {
                    bInit = TRUE;
                    fprintf(stderr, "\nDownload Progress : ");
                }

                fprintf(stderr, "%02d%%", nPer);
                fprintf(stderr, "\b\b\b");
                fflush(stderr);
                usleep(2000);

            }
        }

        m_tSetter.bMode = M_WATCH_DOG_RESET;
        HID_Write(SWIP_REG_ADDR_CTRL_SETTER, sizeof(m_tSetter), (unsigned char*)&m_tSetter);
        
        fprintf(stderr, "\nNormal mode download completed successfully\n");
    }


	return TRUE;
}

int	CInterface::GetFWResult(int nChk)
{
	char strTmp[MAX_PATH];

    bool bChk = FALSE;

	switch(nChk)
	{
	case 0:		strcpy(strTmp, "UPRG_NONE");						bChk = TRUE;	break;
	case 1:		strcpy(strTmp, "UPRG_BOOT_ONLY_SUCCESS");			bChk = TRUE;	break;
	case 2:		strcpy(strTmp, "UPRG_APP_CONFIG_CODE_SUCCESS");	    bChk = TRUE;	break;
	case 3:		strcpy(strTmp, "UPRG_CONFIG_ONLY_SUCCESS");		    bChk = TRUE;	break;
	case 4:		strcpy(strTmp, "UPRG_TOTAL_CODE_SUCCESS");			bChk = TRUE;	break;
	case 128:	strcpy(strTmp, "UPRG_UNKNOWN_FAIL");				bChk = FALSE;	break;
	case 129:	strcpy(strTmp, "UPRG_BOOT_FAIL");					bChk = FALSE;	break;
	case 130:	strcpy(strTmp, "UPRG_APP_FAIL");					bChk = FALSE;	break;
	case 131:	strcpy(strTmp, "UPRG_CONFIG_FAIL");				    bChk = FALSE;	break;
	case 132:	strcpy(strTmp, "UPRG_MAGIC_CODE_FAIL");			    bChk = FALSE;	break;
	case 133:	strcpy(strTmp, "UPRG_APP_CONFIG_FAIL");			    bChk = FALSE;	break;
	case 134:	strcpy(strTmp, "UPRG_CONNECTION_FAIL");			    bChk = FALSE;	break;
	case 135:	strcpy(strTmp, "UPRG_TIMEOUT_FAIL");				bChk = FALSE;	break;
	case 0xFF:	strcpy(strTmp, "UPRG_LIMIT");						bChk = FALSE;	break;
	}

	fprintf(stderr, "FW Result [ Code : %d  MSG : %s]", nChk, strTmp);

    return bChk;
}

int         CInterface::WaitForReadyStatus()
{
    int	bRet = TRUE;

	tProtocolGetter_t	tGetter = { 0, };

	int nCount = 0;

	do {   
		tGetter.bReadyStatus = 0;
		HID_Read(SWIP_REG_ADDR_CTRL_GETTER, sizeof(tGetter), (unsigned char*)&tGetter);
		if (tGetter.bReadyStatus == RS_NONE)
		{
			nCount++;
#ifdef WIN_TEST
			Sleep(1);
#else
            //linux sleep here.
            usleep(1000);
#endif
		}

		if (nCount >= 5000)
		{
			nCount = 0;
			bRet = FALSE;
			break;
		}

        //let me check for future
		// if (tGetter.bReadyStatus == 0)
		// {
		// 	bRet = FALSE;
		// 	break;
		// }
	} while (tGetter.bReadyStatus != RS_READY);

	return bRet;
}

// ChromeOS support functions

int CInterface::OpenDeviceFast()
{
    // Fast open for quick version check (<40ms requirement)
    // This is optimized version of OpenDevice() that stops at first match
    
#ifndef WIN_TEST
    int                             hFileHandle = 0;
    struct dirent*                  pDirEntry = NULL;
    char                            szFilePath[256] = { 0, };
    int                             nError = 0;    
    DIR*                            pDirectory = NULL; 
    struct hidraw_devinfo           DeviceInfo = { 0, };      
    
    pDirectory = opendir("/dev");
    if (pDirectory == NULL)
    {
        return FALSE;
    }

    while ((pDirEntry = readdir(pDirectory)) != NULL)
    {
        if (strlen(pDirEntry->d_name) <= 0)
        {
            break;
        }

        if (strncmp(pDirEntry->d_name, "hidraw", 6) != 0)
        {
            continue;       
        }

        memset(szFilePath, 0, 256);
        sprintf(szFilePath, "/dev/%s", pDirEntry->d_name);
        
        nError = open(szFilePath, O_RDWR | O_NONBLOCK);
        if (nError < 0)
        {
            continue;
        }

        hFileHandle = nError;
        nError = ioctl(hFileHandle, HIDIOCGRAWINFO, &DeviceInfo);
        if (nError < 0)
        {
            close(hFileHandle);
            hFileHandle = 0;
            continue;
        }

        // Check for target device (both normal and DFUP mode)
        if ( ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && 
              (DeviceInfo.vendor == TARGET_DEVICE_VID) && 
              (DeviceInfo.product == TARGET_DEVICE_PID)) ||
              ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && 
              (DeviceInfo.vendor == TARGET_DEVICE_VID) && 
              (DeviceInfo.product == TARGET_DEVICE_PID_TYPE_2)) ||
             ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && 
              (DeviceInfo.vendor == TARGET_DEVICE_DFUP_VID) && 
              (DeviceInfo.product == TARGET_DEVICE_DFUP_PID))||
             ((DeviceInfo.bustype == TARGET_DEVICE_BUS_TYPE) && 
              (DeviceInfo.vendor == TARGET_DEVICE_VID) && 
              (DeviceInfo.product == TARGET_DEVICE_DFUP_PID))
             )
        {
            // Quick validation - just check if we can communicate
            unsigned char ReadBuffer[TARGET_BUFFER_SIZE] = { 0, };
            WriteCommand(hFileHandle, 0x68, SWIP_REG_ADDR_INFO_INTERFACE, 0, NULL);
            if (ReadData(hFileHandle, 0x69, SWIP_REG_ADDR_INFO_INTERFACE, 0x0E, ReadBuffer) > 0)
            {
                // Found and validated
                closedir(pDirectory);
                m_hDeviceHandle = hFileHandle;
                m_bConnect = TRUE;
                return m_hDeviceHandle;
            }
        }

        close(hFileHandle);
        hFileHandle = 0;
    }

    closedir(pDirectory);
    m_bConnect = FALSE;
    return FALSE;
#else
    return TRUE;
#endif
}

int CInterface::ReadVersionOnly(tSWIPFirmwareSectionVersion_t* pVersion)
{
    if (!pVersion)
    {
        return FALSE;
    }

    if (!m_bConnect && !OpenDeviceFast())
    {
        return FALSE;
    }

    // Quick version read
    // Buffer must be at least TARGET_BUFFER_SIZE (64) bytes:
    // ReadData() always calls read(..., TARGET_BUFFER_SIZE) regardless of Length.
    unsigned char readBuf[TARGET_BUFFER_SIZE];
    memset(readBuf, 0, sizeof(readBuf));
    
    WriteCommand(m_hDeviceHandle, 0x68, SWIP_REG_ADDR_INFO_VERSION, 0, NULL);
    if (ReadData(m_hDeviceHandle, 0x69, SWIP_REG_ADDR_INFO_VERSION, sizeof(readBuf), readBuf) > 0)
    {
        // Skip 4 byte header
        memcpy(pVersion, &readBuf[4], sizeof(tSWIPFirmwareSectionVersion_t));
        return TRUE;
    }

    return FALSE;
}

int CInterface::GetCurrentProductID(uint16_t* pid)
{
    if (!pid)
    {
        return FALSE;
    }

#ifndef WIN_TEST
    if (!m_bConnect && !OpenDeviceFast())
    {
        return FALSE;
    }

    struct hidraw_devinfo DeviceInfo = { 0, };
    int nError = ioctl(m_hDeviceHandle, HIDIOCGRAWINFO, &DeviceInfo);
    if (nError >= 0)
    {
        *pid = DeviceInfo.product;
        return TRUE;
    }
    return FALSE;
#else
    // For Windows test, return the target PID
    *pid = TARGET_DEVICE_PID;
    return TRUE;
#endif
}
