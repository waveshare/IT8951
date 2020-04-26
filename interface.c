#include "interface.h"

//Global variable
IT8951DevInfo gstI80DevInfo;
uint8_t* gpFrameBuf; //Host Source Frame buffer
uint32_t gulImgBufAddr; //IT8951 Image buffer address

//-----------------------------------------------------------
//Host controller function 1---Wait for host data Bus Ready
//-----------------------------------------------------------
void LCDWaitForReady()
{
    uint8_t ulData = bcm2835_gpio_lev(HRDY);
    while(ulData == 0)
    {
        ulData = bcm2835_gpio_lev(HRDY);
    }
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

    bcm2835_spi_transfer(wPreamble>>8u);
    bcm2835_spi_transfer(wPreamble);

    LCDWaitForReady();

    bcm2835_spi_transfer(usCmdCode>>8u);
    bcm2835_spi_transfer(usCmdCode);

    bcm2835_gpio_write(CS,HIGH);
}

//-----------------------------------------------------------
//Host controller function 3---Write Data to host data Bus
//-----------------------------------------------------------
void LCDWriteData(uint16_t usData)
{
    //Set Preamble for Write Data
    uint16_t wPreamble	= 0x0000;

    LCDWaitForReady();

    bcm2835_gpio_write(CS,LOW);

    bcm2835_spi_transfer(wPreamble>>8u);
    bcm2835_spi_transfer(wPreamble);

    LCDWaitForReady();

    bcm2835_spi_transfer(usData>>8u);
    bcm2835_spi_transfer(usData);

    bcm2835_gpio_write(CS,HIGH);
}

void LCDWriteNData(uint16_t* pwBuf, uint32_t ulSizeWordCnt)
{
    uint32_t i;

    uint16_t wPreamble	= 0x0000;

    LCDWaitForReady();

    bcm2835_gpio_write(CS,LOW);

    bcm2835_spi_transfer(wPreamble>>8u);
    bcm2835_spi_transfer(wPreamble);

    LCDWaitForReady();

    for(i=0;i<ulSizeWordCnt;i++)
    {
        bcm2835_spi_transfer(pwBuf[i]>>8u);
        bcm2835_spi_transfer(pwBuf[i]);
    }

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

    bcm2835_spi_transfer(wPreamble>>8u);
    bcm2835_spi_transfer(wPreamble);

    LCDWaitForReady();

    wRData=bcm2835_spi_transfer(0x00);//dummy
    wRData=bcm2835_spi_transfer(0x00);//dummy

    LCDWaitForReady();

    wRData = bcm2835_spi_transfer(0x00)<<8u;
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

    bcm2835_spi_transfer(wPreamble>>8u);
    bcm2835_spi_transfer(wPreamble);

    LCDWaitForReady();

    pwBuf[0]=bcm2835_spi_transfer(0x00);//dummy
    pwBuf[0]=bcm2835_spi_transfer(0x00);//dummy

    LCDWaitForReady();

    for(i=0;i<ulSizeWordCnt;i++)
    {
        pwBuf[i] = bcm2835_spi_transfer(0x00)<<8u;
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

//-----------------------------------------------------------
//Host Cmd 6---MEM_BST_RD_T
//-----------------------------------------------------------
void IT8951MemBurstReadTrigger(uint32_t ulMemAddr , uint32_t ulReadSize)
{
    uint16_t usArg[4];
    //Setting Arguments for Memory Burst Read
    usArg[0] = (uint16_t)(ulMemAddr & 0x0000FFFF); //addr[15:0]
    usArg[1] = (uint16_t)( (ulMemAddr >> 16) & 0x0000FFFF ); //addr[25:16]
    usArg[2] = (uint16_t)(ulReadSize & 0x0000FFFF); //Cnt[15:0]
    usArg[3] = (uint16_t)( (ulReadSize >> 16) & 0x0000FFFF ); //Cnt[25:16]
    //Send Cmd and Arg
    LCDSendCmdArg(IT8951_TCON_MEM_BST_RD_T , usArg , 4);
}
//-----------------------------------------------------------
//Host Cmd 7---MEM_BST_RD_S
//-----------------------------------------------------------
void IT8951MemBurstReadStart()
{
    LCDWriteCmdCode(IT8951_TCON_MEM_BST_RD_S);
}
//-----------------------------------------------------------
//Host Cmd 8---MEM_BST_WR
//-----------------------------------------------------------
void IT8951MemBurstWrite(uint32_t ulMemAddr , uint32_t ulWriteSize)
{
    uint16_t usArg[4];
    //Setting Arguments for Memory Burst Write
    usArg[0] = (uint16_t)(ulMemAddr & 0x0000FFFF); //addr[15:0]
    usArg[1] = (uint16_t)( (ulMemAddr >> 16u) & 0x0000FFFF ); //addr[25:16]
    usArg[2] = (uint16_t)(ulWriteSize & 0x0000FFFF); //Cnt[15:0]
    usArg[3] = (uint16_t)( (ulWriteSize >> 16u) & 0x0000FFFF ); //Cnt[25:16]
    //Send Cmd and Arg
    LCDSendCmdArg(IT8951_TCON_MEM_BST_WR , usArg , 4);
}
//-----------------------------------------------------------
//Host Cmd 9---MEM_BST_END
//-----------------------------------------------------------
void IT8951MemBurstEnd(void)
{
    LCDWriteCmdCode(IT8951_TCON_MEM_BST_END);
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
//Host Cmd 11---LD_IMG_AREA
//-----------------------------------------------------------
void IT8951LoadImgAreaStart(IT8951LdImgInfo* pstLdImgInfo ,IT8951AreaImgInfo* pstAreaImgInfo)
{
    uint16_t usArg[5];
    //Setting Argument for Load image start
    usArg[0] = (pstLdImgInfo->usEndianType << 8 )
               |(pstLdImgInfo->usPixelFormat << 4)
               |(pstLdImgInfo->usRotate);
    usArg[1] = pstAreaImgInfo->usX;
    usArg[2] = pstAreaImgInfo->usY;
    usArg[3] = pstAreaImgInfo->usWidth;
    usArg[4] = pstAreaImgInfo->usHeight;
    //Send Cmd and Args
    LCDSendCmdArg(IT8951_TCON_LD_IMG_AREA , usArg , 5);
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

    printf("sending command\n");
    //Send I80 CMD
    LCDWriteCmdCode(USDEF_I80_CMD_GET_DEV_INFO);

    printf("burst read\n");
    //Burst Read Request for SPI interface only
    LCDReadNData(pusWord, sizeof(IT8951DevInfo)/2);//Polling HRDY for each words(2-bytes) if possible

    printf("parsing info\n");
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
    uint16_t status = IT8951ReadReg(LUTAFSR);
    while(status) {
        bcm2835_delay(50);
        status = IT8951ReadReg(LUTAFSR);
    };
}

//-----------------------------------------------------------
//Display function 2---Load Image Area process
//-----------------------------------------------------------
void IT8951HostAreaPackedPixelWrite(IT8951LdImgInfo* pstLdImgInfo,IT8951AreaImgInfo* pstAreaImgInfo)
{
    uint32_t i,j;
    //Source buffer address of Host
    uint16_t* pusFrameBuf = (uint16_t*)pstLdImgInfo->ulStartFBAddr;

    //Set Image buffer(IT8951) Base address
    IT8951SetImgBufBaseAddr(pstLdImgInfo->ulImgBufBaseAddr);
    //Send Load Image start Cmd
    IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);
    //Host Write Data
    for(j=0;j< pstAreaImgInfo->usHeight;j++)
    {
        for(i=0;i< pstAreaImgInfo->usWidth/2;i++)
        {
            //Write a Word(2-Bytes) for each time
            LCDWriteData(*pusFrameBuf);
            pusFrameBuf++;
        }
    }
    //Send Load Img End Command
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


//-----------------------------------------------------------
// INTERFACE : functions for interacting with epaper
//-----------------------------------------------------------
uint8_t Initialize()
{
    printf("bcm2835 setup\n");
    if (!bcm2835_init())
    {
        printf("bcm2835_init error \n");
        return 1;
    }

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);   		//default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);               		//default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);		//default

    bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(HRDY, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_write(CS, HIGH);

    printf("****** IT8951 ******\n");

    bcm2835_gpio_write(RESET, LOW);
    bcm2835_delay(100);
    bcm2835_gpio_write(RESET, HIGH);

    printf("getting device info\n");
    //Get Device Info
    GetIT8951SystemInfo(&gstI80DevInfo);

    printf("allocating host frame buffer\n");
    gpFrameBuf =     malloc(gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);
    if (!gpFrameBuf)
    {
        perror("malloc error!\n");
        return 1;
    }

    gulImgBufAddr = gstI80DevInfo.usImgBufAddrL | (gstI80DevInfo.usImgBufAddrH << 16);

    printf("enabling packed mode\n");
    //Set to Enable I80 Packed mode
    IT8951WriteReg(I80CPCR, 0x0001);

    printf("setting vcom\n");
    if (VCOM != IT8951GetVCOM())
    {
        IT8951SetVCOM(VCOM);
        printf("VCOM = -%.02fV\n",(float)IT8951GetVCOM()/1000);
    }

    return 0;
}

IT8951DevInfo GetDeviceInfo() {
    return gstI80DevInfo;
}

int GetWidth() {
    return gstI80DevInfo.usPanelW;
}

int GetHeight() {
    return gstI80DevInfo.usPanelH;
}


void Close()
{
    free(gpFrameBuf);

    bcm2835_spi_end();
    bcm2835_close();
}

void DisplayImage(const uint8_t * image) {
    IT8951LdImgInfo stLdImgInfo;
    IT8951AreaImgInfo stAreaImgInfo;

    //Prepare image
//    for(uint16_t i=0; i<gstI80DevInfo.usPanelH; i++) {
//        for(uint16_t j=0; j<gstI80DevInfo.usPanelW; j++) {
//            int idx = i*gstI80DevInfo.usPanelW + j;
//            *(gpFrameBuf + idx) = *(image + idx);
//        }
//    }

    //Check TCon is free ? Wait TCon Ready (optional)
    IT8951WaitForDisplayReady();

    //Load Image and Display
    //Setting Load image information
    stLdImgInfo.ulStartFBAddr    = (uint32_t)image;
    stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
    stLdImgInfo.usPixelFormat    = IT8951_8BPP;
    stLdImgInfo.usRotate         = IT8951_ROTATE_0;
    stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
    //Set Load Area
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
    stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;

    //Load Image from Host to IT8951 Image Buffer
    IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
    //Display Area ?V (x,y,w,h) with mode 0 for initial White to clear Panel
    IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}
