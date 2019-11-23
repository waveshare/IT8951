#include "IT8951.h"
#include <stdio.h>
#include <time.h>

//Global varivale
IT8951DevInfo gstI80DevInfo;
uint8_t* gpFrameBuf; //Host Source Frame buffer
uint32_t gulImgBufAddr; //IT8951 Image buffer address

void EPD_Clear(uint8_t Color)
{
    memset(gpFrameBuf, Color, gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);
}
//-----------------------------------------------------------
//Host controller function 1---Wait for host data Bus Ready
//-----------------------------------------------------------
void LCDWaitForReady()
{
    while(bcm2835_gpio_lev(HRDY) == 0);
}

//-----------------------------------------------------------
//Host controller function 2---Write command code to host data Bus
//-----------------------------------------------------------
void LCDWriteCmdCode(uint16_t usCmdCode)
{
	//Set Preamble for Write Command
	uint16_t wPreamble = 0x6000; 
	
    LCDWaitForReady();

	bcm2835_gpio_write(CS,LOW);

    uint8_t data[4] = {wPreamble >> 8, wPreamble, usCmdCode>>8, usCmdCode};
    bcm2835_spi_transfern((char*)data, 4);
//    bcm2835_spi_transfer(wPreamble>>8);
//    bcm2835_spi_transfer(wPreamble);
//
//    LCDWaitForReady();
//
//    bcm2835_spi_transfer(usCmdCode>>8);
//    bcm2835_spi_transfer(usCmdCode);

	bcm2835_gpio_write(CS,HIGH); 
}

//-----------------------------------------------------------
//Host controller function 3---Write Data to host data Bus
//-----------------------------------------------------------
void LCDWriteData(uint16_t usData)
{
    LCDWaitForReady();

	bcm2835_gpio_write(CS,LOW);

    uint8_t buffer[4] = {0, 0, usData >> 8, usData};

    bcm2835_spi_transfern((char*)buffer, 4);

	bcm2835_gpio_write(CS,HIGH); 
}

void LCDWriteNData(uint8_t *data, uint32_t len)
{
//    LCDWaitForReady();

    bcm2835_gpio_write(CS,LOW);

	bcm2835_spi_transfern((char*)data, len + 2);
//    LCDWaitForReady();

    bcm2835_gpio_write(CS,HIGH);
}

//-----------------------------------------------------------
//Host controller function 4---Read Data from host data Bus
//-----------------------------------------------------------
uint16_t LCDReadData()
{
	uint16_t wRData; 
	
	uint16_t wPreamble = 0x1000;

	LCDWaitForReady();

	bcm2835_gpio_write(CS,LOW);
		
	bcm2835_spi_transfer(wPreamble>>8);
	bcm2835_spi_transfer(wPreamble);

	LCDWaitForReady();
	
	wRData=bcm2835_spi_transfer(0x00);//dummy
	wRData=bcm2835_spi_transfer(0x00);//dummy
	
	LCDWaitForReady();
	
	wRData = bcm2835_spi_transfer(0x00)<<8;
	wRData |= bcm2835_spi_transfer(0x00);
		
	bcm2835_gpio_write(CS,HIGH); 
		
	return wRData;
}

//-----------------------------------------------------------
//  Read Burst N words Data
//-----------------------------------------------------------
void LCDReadNData(uint16_t* pwBuf, uint32_t ulSizeWordCnt)
{
	uint32_t i;
	
	uint16_t wPreamble = 0x1000;

	LCDWaitForReady();
	
	bcm2835_gpio_write(CS,LOW);

	bcm2835_spi_transfer(wPreamble>>8);
	bcm2835_spi_transfer(wPreamble);
	
	LCDWaitForReady();
	
	pwBuf[0]=bcm2835_spi_transfer(0x00);//dummy
	pwBuf[0]=bcm2835_spi_transfer(0x00);//dummy
	
	LCDWaitForReady();
	
	for(i=0;i<ulSizeWordCnt;i++)
	{
		pwBuf[i] = bcm2835_spi_transfer(0x00)<<8;
		pwBuf[i] |= bcm2835_spi_transfer(0x00);
	}
	
	bcm2835_gpio_write(CS,HIGH); 
}

//-----------------------------------------------------------
//Host controller function 5---Write command to host data Bus with aruments
//-----------------------------------------------------------
void LCDSendCmdArg(uint16_t usCmdCode,uint16_t* pArg, uint16_t usNumArg)
{
     uint16_t i;
     //Send Cmd code
     LCDWriteCmdCode(usCmdCode);
     //Send Data

    for(i=0;i<usNumArg;i++)
    {
        LCDWriteData(pArg[i]);
    }
}

//-----------------------------------------------------------
//Host Cmd 1---SYS_RUN
//-----------------------------------------------------------
void IT8951SystemRun()
{
    LCDWriteCmdCode(IT8951_TCON_SYS_RUN);
}

//-----------------------------------------------------------
//Host Cmd 2---STANDBY
//-----------------------------------------------------------
void IT8951StandBy()
{
    LCDWriteCmdCode(IT8951_TCON_STANDBY);
}

//-----------------------------------------------------------
//Host Cmd 3---SLEEP
//-----------------------------------------------------------
void IT8951Sleep()
{
    LCDWriteCmdCode(IT8951_TCON_SLEEP);
}

//-----------------------------------------------------------
//Host Cmd 4---REG_RD
//-----------------------------------------------------------
uint16_t IT8951ReadReg(uint16_t usRegAddr)
{
	uint16_t usData;
	
	//Send Cmd and Register Address
	LCDWriteCmdCode(IT8951_TCON_REG_RD);
	LCDWriteData(usRegAddr);
	//Read data from Host Data bus
	usData = LCDReadData();
	return usData;
}
//-----------------------------------------------------------
//Host Cmd 5---REG_WR
//-----------------------------------------------------------
void IT8951WriteReg(uint16_t usRegAddr,uint16_t usValue)
{
	//Send Cmd , Register Address and Write Value
	LCDWriteCmdCode(IT8951_TCON_REG_WR);
	LCDWriteData(usRegAddr);
	LCDWriteData(usValue);
}

uint16_t IT8951GetVCOM(void)
{
	uint16_t vcom;
	
	LCDWriteCmdCode(USDEF_I80_CMD_VCOM);
	LCDWriteData(0);
	//Read data from Host Data bus
	vcom = LCDReadData();
	return vcom;
}

void IT8951SetVCOM(uint16_t vcom)
{
	LCDWriteCmdCode(USDEF_I80_CMD_VCOM);
	LCDWriteData(1);
	//Read data from Host Data bus
	LCDWriteData(vcom);
}

//-----------------------------------------------------------
//Host Cmd 10---LD_IMG
//-----------------------------------------------------------
void IT8951LoadImgStart(IT8951LdImgInfo* pstLdImgInfo)
{
    uint16_t usArg;
    //Setting Argument for Load image start
    usArg = (pstLdImgInfo->usEndianType << 8 )
    |(pstLdImgInfo->usPixelFormat << 4)
    |(pstLdImgInfo->usRotate);
    //Send Cmd
    LCDWriteCmdCode(IT8951_TCON_LD_IMG);
    //Send Arg
    LCDWriteData(usArg);
}

//-----------------------------------------------------------
//Host Cmd 12---LD_IMG_END
//-----------------------------------------------------------
void IT8951LoadImgEnd(void)
{
    LCDWriteCmdCode(IT8951_TCON_LD_IMG_END);
}

void GetIT8951SystemInfo(void* pBuf)
{
	uint16_t* pusWord = (uint16_t*)pBuf;
	IT8951DevInfo* pstDevInfo;

	//Send I80 CMD
	LCDWriteCmdCode(USDEF_I80_CMD_GET_DEV_INFO);
 
	//Burst Read Request for SPI interface only
	LCDReadNData(pusWord, sizeof(IT8951DevInfo)/2);//Polling HRDY for each words(2-bytes) if possible
	
	//Show Device information of IT8951
	pstDevInfo = (IT8951DevInfo*)pBuf;
	printf("Panel(W,H) = (%d,%d)\r\n",
	pstDevInfo->usPanelW, pstDevInfo->usPanelH );
	printf("Image Buffer Address = %X\r\n",
	pstDevInfo->usImgBufAddrL | (pstDevInfo->usImgBufAddrH << 16));
	//Show Firmware and LUT Version
	printf("FW Version = %s\r\n", (uint8_t*)pstDevInfo->usFWVersion);
	printf("LUT Version = %s\r\n", (uint8_t*)pstDevInfo->usLUTVersion);
}

//-----------------------------------------------------------
//Initial function 2---Set Image buffer base address
//-----------------------------------------------------------
void IT8951SetImgBufBaseAddr(uint32_t ulImgBufAddr)
{
	uint16_t usWordH = (uint16_t)((ulImgBufAddr >> 16) & 0x0000FFFF);
	uint16_t usWordL = (uint16_t)( ulImgBufAddr & 0x0000FFFF);
	//Write LISAR Reg
	IT8951WriteReg(LISAR + 2 ,usWordH);
	IT8951WriteReg(LISAR ,usWordL);
}

//-----------------------------------------------------------
// 3.6. Display Functions
//-----------------------------------------------------------

//-----------------------------------------------------------
//Display function 1---Wait for LUT Engine Finish
//                     Polling Display Engine Ready by LUTNo
//-----------------------------------------------------------
void IT8951WaitForDisplayReady()
{
	//Check IT8951 Register LUTAFSR => NonZero Busy, 0 - Free
	while(IT8951ReadReg(LUTAFSR));
}

//-----------------------------------------------------------
//Display function 2---Load Image Area process
//-----------------------------------------------------------
void IT8951HostAreaPackedPixelWrite(IT8951LdImgInfo* pstLdImgInfo)
{
	//Source buffer address of Host
	uint8_t* pusFrameBuf = (uint8_t*)pstLdImgInfo->ulStartFBAddr;

	//Send Load Image start Cmd


    clock_t t;
    t = clock();

    IT8951LoadImgStart(pstLdImgInfo);

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

    LCDWriteNData(pusFrameBuf, 1872 * 1404 / 2);
    printf("LCDWriteNData took %f seconds to execute \n", time_taken);
	IT8951LoadImgEnd();
}

//-----------------------------------------------------------
//Display functions 3---Application for Display panel Area
//-----------------------------------------------------------
void IT8951DisplayArea(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode)
{
	//Send I80 Display Command (User defined command of IT8951)
	LCDWriteCmdCode(USDEF_I80_CMD_DPY_AREA); //0x0034
	//Write arguments

    LCDWriteData(usX);
    LCDWriteData(usY);
    LCDWriteData(usW);
    LCDWriteData(usH);
    LCDWriteData(usDpyMode);
}

IT8951LdImgInfo stLdImgInfo;

//-----------------------------------------------------------
//Test function 1---Software Initial
//-----------------------------------------------------------
uint8_t *IT8951_Init(int expected_width, int expected_height, int reverted)
{
	if (!bcm2835_init()) 
	{
		printf("bcm2835_init error \n");
		return NULL;
	}
	
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);   		//default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);               		//default
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);		//default
	
	bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);  
	bcm2835_gpio_fsel(HRDY, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);
	
	bcm2835_gpio_write(CS, HIGH);

	printf("****** IT8951 ******\n");
	
	bcm2835_gpio_write(RESET, LOW);
	bcm2835_delay(50);
	bcm2835_gpio_write(RESET, HIGH);

	//Get Device Info
    GetIT8951SystemInfo(&gstI80DevInfo);

    if (expected_width != gstI80DevInfo.usPanelW || expected_height != gstI80DevInfo.usPanelH) {
        perror("Screen width and height is different from the expected one. It could compromise the following commands");
    }
	gpFrameBuf = malloc(expected_width * expected_height / 2 + 2);
	if (!gpFrameBuf)
	{
		perror("malloc error!\n");
		return NULL;
	}
	
 	gulImgBufAddr = gstI80DevInfo.usImgBufAddrL | (gstI80DevInfo.usImgBufAddrH << 16);
 	
 	//Set to Enable I80 Packed mode
 	IT8951WriteReg(I80CPCR, 0x0001);

	if (VCOM != IT8951GetVCOM())
	{
		IT8951SetVCOM(VCOM);
		printf("VCOM = -%.02fV\n",(float)IT8951GetVCOM()/1000);
	}


    //Setting Load image information
    stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
    stLdImgInfo.usEndianType     = IT8951_LDIMG_B_ENDIAN;
    stLdImgInfo.usPixelFormat    = IT8951_4BPP;
    stLdImgInfo.usRotate         = reverted ? IT8951_ROTATE_180 : IT8951_ROTATE_0;
    stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;

    IT8951SetImgBufBaseAddr(gulImgBufAddr);

	return gpFrameBuf + 2;
}

void IT8951_Cancel()
{
	free(gpFrameBuf);

	bcm2835_spi_end();
	bcm2835_close();
}


void IT8951_Display4BppBuffer()
{
	//Load Image from Host to IT8951 Image Buffer
    clock_t t;
    t = clock();
    IT8951HostAreaPackedPixelWrite(&stLdImgInfo);
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

    printf("IT8951HostAreaPackedPixelWrite took %f seconds to execute \n", time_taken);

	//Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform
    IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);

//    IT8951WaitForDisplayReady();
}
