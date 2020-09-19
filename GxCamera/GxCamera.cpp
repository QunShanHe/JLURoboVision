//--------------------------------------------------------------------------------
/**
\file     GxCamera.cpp
\brief    GxCamera class defination

\date     2020-05-14
\author   Mountain

*/
//----------------------------------------------------------------------------------

#include"GxCamera.h"

extern pthread_mutex_t Globalmutex; // threads conflict due to image-updating
extern pthread_cond_t GlobalCondCV; // threads conflict due to image-updating
extern bool imageReadable;          // threads conflict due to image-updating
//extern cv::Mat src;                     // Transfering buffer

GxCamera::GxCamera()
{
    g_hDevice = NULL;                     ///< Device handle
    g_pDeviceSN = "KJ0190120002";
    g_bColorFilter = false;                        ///< Color filter support flag
    g_i64ColorFilter = GX_COLOR_FILTER_NONE;    ///< Color filter of device
    g_bAcquisitionFlag = true;                    ///< Thread running flag
    g_nAcquisitonThreadID = 0;                ///< Thread ID of Acquisition thread

    g_nPayloadSize = 0;                         ///< Payload size
}

GxCamera::~GxCamera()
{

}

/// Initialize libary
GX_STATUS GxCamera::initLib()
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    printf("\n");
    printf("-------------------------------------------------------------\n");
    printf("You've entered camera function which create a thread to acquire color image continuously and save it into extern frame.\n");
    printf("version: 1.0.1901.9311\n");
    printf("-------------------------------------------------------------\n");
    printf("\n");
    printf("Initializing......");
    printf("\n\n");
    //Initialize libary
    status = GXInitLib();
    GX_VERIFY(status);
}


/// Open device and display device information
GX_STATUS GxCamera::openDevice(const char* CameraSN)
{
    //return GX_STATUS_SUCCESS;
    GX_STATUS status = GX_STATUS_SUCCESS;
    uint32_t ui32DeviceNum = 0;
    g_pDeviceSN = const_cast<char*>(CameraSN);

    //Get device enumerated number
    status = GXUpdateDeviceList(&ui32DeviceNum, 1000);
    GX_VERIFY_EXIT(status);

    //If no device found, app exit<char*>(const char*);
    if(ui32DeviceNum <= 0)
    {
        printf("<No device found>\n");
        GXCloseLib();
        exit(status);
    }

    //Init OpenParam , Open device in exclusive mode by SN
    GX_OPEN_PARAM stOpenParam;
    stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
    stOpenParam.openMode = GX_OPEN_SN;
    stOpenParam.pszContent = g_pDeviceSN;

    //Open device
    status = GXOpenDevice(&stOpenParam, &g_hDevice);
    GX_VERIFY_EXIT(status);


    //Get Device Info
    printf("***********************************************\n");
    //Get libary version
    printf("<Libary Version : %s>\n", GXGetLibVersion());
    size_t nSize = 0;
    //Get string length of Vendor name
    status = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME, &nSize);
    GX_VERIFY_EXIT(status);
    //Alloc memory for Vendor name
    char *pszVendorName = new char[nSize];
    //Get Vendor name
    status = GXGetString(g_hDevice, GX_STRING_DEVICE_VENDOR_NAME, pszVendorName, &nSize);
    if (status != GX_STATUS_SUCCESS)
    {
        delete[] pszVendorName;
        pszVendorName = NULL;
        GX_VERIFY_EXIT(status);
    }

    printf("<Vendor Name : %s>\n", pszVendorName);
    //Release memory for Vendor name
    delete[] pszVendorName;
    pszVendorName = NULL;

    //Get string length of Model name
    status = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, &nSize);
    GX_VERIFY_EXIT(status);
    //Alloc memory for Model name
    char *pszModelName = new char[nSize];
    //Get Model name
    status = GXGetString(g_hDevice, GX_STRING_DEVICE_MODEL_NAME, pszModelName, &nSize);
    if (status != GX_STATUS_SUCCESS)
    {
        delete[] pszModelName;
        pszModelName = NULL;
        GX_VERIFY_EXIT(status);
    }

    printf("<Model Name : %s>\n", pszModelName);
    //Release memory for Model name
    delete[] pszModelName;
    pszModelName = NULL;

    //Get string length of Serial number
    status = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, &nSize);
    GX_VERIFY_EXIT(status);
    //Alloc memory for Serial number
    char *pszSerialNumber = new char[nSize];
    //Get Serial Number
    status = GXGetString(g_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, pszSerialNumber, &nSize);
    if (status != GX_STATUS_SUCCESS)
    {
        delete[] pszSerialNumber;
        pszSerialNumber = NULL;
        GX_VERIFY_EXIT(status);
    }

    printf("<Serial Number : %s>\n", pszSerialNumber);
    //Release memory for Serial number
    delete[] pszSerialNumber;
    pszSerialNumber = NULL;

    //Get string length of Device version
    status = GXGetStringLength(g_hDevice, GX_STRING_DEVICE_VERSION, &nSize);
    GX_VERIFY_EXIT(status);
    char *pszDeviceVersion = new char[nSize];
    //Get Device Version
    status = GXGetString(g_hDevice, GX_STRING_DEVICE_VERSION, pszDeviceVersion, &nSize);
    if (status != GX_STATUS_SUCCESS)
    {
        delete[] pszDeviceVersion;
        pszDeviceVersion = NULL;
        GX_VERIFY_EXIT(status);
    }
    printf("<Device Version : %s>\n", pszDeviceVersion);

    //Get Device Maxium width * height
    int64_t width, height;
    GXGetInt(g_hDevice, GX_INT_WIDTH_MAX, &width);
    GXGetInt(g_hDevice, GX_INT_HEIGHT_MAX, &height);
    printf("<Camera maxImage : %d * %d>\n", width, height);

    //Release memory for Device version
    delete[] pszDeviceVersion;
    pszDeviceVersion = NULL;
    printf("***********************************************\n");

    //Get the type of Bayer conversion. whether is a color camera.

    status = GXIsImplemented(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_bColorFilter);
    GX_VERIFY_EXIT(status);

    //This app only support color cameras
    if (!g_bColorFilter)
    {
        printf("<This app only support color cameras! App Exit!>\n");
        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();
        exit(0);
    }
    else
    {
        status = GXGetEnum(g_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &g_i64ColorFilter);
        GX_VERIFY_EXIT(status);
    }
    return GX_STATUS_SUCCESS;
}


/// For client
/// Set camera exposure and gain params
void GxCamera::setExposureGainParam(bool AutoExposure, bool AutoGain, double ExposureTime, double AutoExposureTimeMin, double AutoExposureTimeMax, double Gain, double AutoGainMin, double AutoGainMax, int64_t GrayValue)
{
    exposure_gain.m_bAutoExposure = AutoExposure;
    exposure_gain.m_bAutoGain = AutoGain;
    exposure_gain.m_dExposureTime = ExposureTime;
    exposure_gain.m_dAutoExposureTimeMin = AutoExposureTimeMin;
    exposure_gain.m_dAutoExposureTimeMax = AutoExposureTimeMax;
    exposure_gain.m_dGain = Gain;
    exposure_gain.m_dAutoGainMin = AutoGainMin;
    exposure_gain.m_dAutoGainMax = AutoGainMax;
    exposure_gain.m_i64GrayValue = GrayValue;
}
/// Set camera exposure and gain
GX_STATUS GxCamera::setExposureGain()
{
    GX_STATUS status;

    // Set Exposure
    if(exposure_gain.m_bAutoExposure)
    {
        status = GXSetEnum(g_hDevice,GX_ENUM_EXPOSURE_AUTO,GX_EXPOSURE_AUTO_CONTINUOUS);
        GX_VERIFY(status);
        status = GXSetFloat(g_hDevice,GX_FLOAT_AUTO_EXPOSURE_TIME_MAX,exposure_gain.m_dAutoExposureTimeMax);
        GX_VERIFY(status);
        status = GXSetFloat(g_hDevice,GX_FLOAT_AUTO_EXPOSURE_TIME_MIN,exposure_gain.m_dAutoExposureTimeMin);
        GX_VERIFY(status);
    }
    else
    {
        status = GXSetEnum(g_hDevice,GX_ENUM_EXPOSURE_MODE,GX_EXPOSURE_MODE_TIMED);
        GX_VERIFY(status);
        status = GXSetEnum(g_hDevice,GX_ENUM_EXPOSURE_AUTO,GX_EXPOSURE_AUTO_OFF);
        GX_VERIFY(status);
        status = GXSetFloat(g_hDevice,GX_FLOAT_EXPOSURE_TIME,exposure_gain.m_dExposureTime);
        GX_VERIFY(status);
    }

    // Set Gain
    if(exposure_gain.m_bAutoGain)
    {
        status = GXSetEnum(g_hDevice,GX_ENUM_GAIN_AUTO,GX_GAIN_AUTO_CONTINUOUS);
        GX_VERIFY(status);
        status = GXSetFloat(g_hDevice,GX_FLOAT_AUTO_GAIN_MAX,exposure_gain.m_dAutoGainMax);
        GX_VERIFY(status);
        status = GXSetFloat(g_hDevice,GX_FLOAT_AUTO_GAIN_MIN,exposure_gain.m_dAutoGainMin);
        GX_VERIFY(status);
    }
    else
    {
        status = GXSetEnum(g_hDevice,GX_ENUM_GAIN_AUTO,GX_GAIN_AUTO_OFF);
        GX_VERIFY(status);
        status = GXSetEnum(g_hDevice,GX_ENUM_GAIN_SELECTOR,GX_GAIN_SELECTOR_ALL);
        GX_VERIFY(status);
        status = GXSetFloat(g_hDevice,GX_FLOAT_GAIN,exposure_gain.m_dGain);
        GX_VERIFY(status);
    }

    // Set Expected Gray Value
    status = GXSetInt(g_hDevice,GX_INT_GRAY_VALUE,exposure_gain.m_i64GrayValue);
    GX_VERIFY(status);

    return GX_STATUS_SUCCESS;
}

/// For client
/// Set camera roi params
void GxCamera::setRoiParam(int64_t Width, int64_t Height, int64_t OffsetX, int64_t OffsetY)
{
    roi.m_i64Width = Width;
    roi.m_i64Height = Height;
    roi.m_i64OffsetX = OffsetX;
    roi.m_i64OffsetY = OffsetY;
}
/// Set camera roi
GX_STATUS GxCamera::setRoi()
{
    GX_STATUS status = GX_STATUS_SUCCESS;
    //设 置 一 个 offset 偏 移 为 (X,Y) ,WidthXHeight 尺 寸 的 区 域
    status = GXSetInt(g_hDevice, GX_INT_WIDTH,64);
    GX_VERIFY(status);
    status = GXSetInt(g_hDevice, GX_INT_HEIGHT,64);
    GX_VERIFY(status);
    status = GXSetInt(g_hDevice, GX_INT_OFFSET_X, roi.m_i64OffsetX);
    GX_VERIFY(status);
    status = GXSetInt(g_hDevice, GX_INT_OFFSET_Y, roi.m_i64OffsetY);
    GX_VERIFY(status);
    status = GXSetInt(g_hDevice, GX_INT_WIDTH, roi.m_i64Width);
    GX_VERIFY(status);
    status = GXSetInt(g_hDevice, GX_INT_HEIGHT, roi.m_i64Height);
    GX_VERIFY(status);
}


/// For client
/// Set camera white balance params
void GxCamera::setWhiteBalanceParam(bool WhiteBalanceOn, GX_AWB_LAMP_HOUSE_ENTRY lightSource)
{
    //自动白平衡光照环境
    // GX_AWB_LAMP_HOUSE_ADAPTIVE 自适应
    // GX_AWB_LAMP_HOUSE_FLUORESCENCE 荧光灯
    // GX_AWB_LAMP_HOUSE_INCANDESCENT 白炽灯
    // GX_AWB_LAMP_HOUSE_U30 光源温度3000k
    // GX_AWB_LAMP_HOUSE_D50 光源温度5000k
    // GX_AWB_LAMP_HOUSE_D65 光源温度6500k
    // GX_AWB_LAMP_HOUSE_D70 光源温度7000k
    white_balance.m_bWhiteBalance = WhiteBalanceOn;
    white_balance.lightSource = lightSource;
}
/// Set camera WhiteBalance
GX_STATUS GxCamera::setWhiteBalance()
{

    //选择白平衡通道
    GX_STATUS status;

    if(white_balance.m_bWhiteBalance)
    {
        status = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED);
        //status = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN);
        //status = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_RAT IO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE);

        //设置自动白平衡感兴趣区域(整个roi)
        status = GXSetInt(g_hDevice, GX_INT_AWBROI_WIDTH, roi.m_i64Width);
        status = GXSetInt(g_hDevice, GX_INT_AWBROI_HEIGHT, roi.m_i64Height);
        status = GXSetInt(g_hDevice, GX_INT_AWBROI_OFFSETX, 0);
        status = GXSetInt(g_hDevice, GX_INT_AWBROI_OFFSETY, 0);
        GX_VERIFY(status);

        //自动白平衡设置
        status = GXSetEnum(g_hDevice, GX_ENUM_AWB_LAMP_HOUSE, white_balance.lightSource);
        GX_VERIFY(status);

        //设置连续自动白平衡
        status = GXSetEnum(g_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS);
        GX_VERIFY(status);
    }
    else
    {
        status = GXSetEnum(g_hDevice,GX_ENUM_BALANCE_WHITE_AUTO,GX_BALANCE_WHITE_AUTO_OFF);
        GX_VERIFY(status);
    }

    return GX_STATUS_SUCCESS;
}


/// Main function to run the whole program
GX_STATUS GxCamera::acquisitionStart(cv::Mat* targetMatImg)
{

    //////////////////////////////////////////SOME SETTINGS/////////////////////////////////////////////
    threadParam.m_hDevice = g_hDevice;
    threadParam.m_pImage = targetMatImg;
    threadParam.g_AcquisitionFlag = &g_bAcquisitionFlag;

    GX_STATUS emStatus;
    //Set Roi
    emStatus = setRoi();
    GX_VERIFY_EXIT(emStatus);
    //Set Exposure and Gain
    emStatus = setExposureGain();
    GX_VERIFY_EXIT(emStatus);
    //Set WhiteBalance
    emStatus = setWhiteBalance();
    GX_VERIFY_EXIT(emStatus);


    //Set acquisition mode
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    GX_VERIFY_EXIT(emStatus);

    //Set trigger mode
    emStatus = GXSetEnum(g_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
    GX_VERIFY_EXIT(emStatus);

    //Set buffer quantity of acquisition queue
    uint64_t nBufferNum = ACQ_BUFFER_NUM;
    emStatus = GXSetAcqusitionBufferNumber(g_hDevice, nBufferNum);
    GX_VERIFY_EXIT(emStatus);

    bool bStreamTransferSize = false;
    emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, &bStreamTransferSize);
    GX_VERIFY_EXIT(emStatus);

    if(bStreamTransferSize)
    {
        //Set size of data transfer block
        emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_SIZE, ACQ_TRANSFER_SIZE);
        GX_VERIFY_EXIT(emStatus);
    }

    bool bStreamTransferNumberUrb = false;
    emStatus = GXIsImplemented(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, &bStreamTransferNumberUrb);
    GX_VERIFY_EXIT(emStatus);

    if(bStreamTransferNumberUrb)
    {
        //Set qty. of data transfer block
        emStatus = GXSetInt(g_hDevice, GX_DS_INT_STREAM_TRANSFER_NUMBER_URB, ACQ_TRANSFER_NUMBER_URB);
        GX_VERIFY_EXIT(emStatus);
    }

    //Device start acquisition
    emStatus = GXStreamOn(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GX_VERIFY_EXIT(emStatus);
    }

    //////////////////////////////////////////CREATE THREAD/////////////////////////////////////////////

    //Start acquisition thread, if thread create failed, exit this app
    int nRet = pthread_create(&g_nAcquisitonThreadID, NULL, ProcGetImage, (void*)&threadParam);
    if(nRet != 0)
    {
        GXCloseDevice(g_hDevice);
        g_hDevice = NULL;
        GXCloseLib();

        printf("<Failed to create the acquisition thread, App Exit!>\n");
        exit(nRet);
    }

    //Main loop
    bool bRun = true;
    while(bRun == true)
    {
        printf("????????????????loop is running???????????????????????\n");
        char chKey = getchar();
        if(chKey=='x'||chKey=='X')
        {
            break;
        }
    }

    //////////////////////////////////////////STOP THREAD/////////////////////////////////////////////

    //Stop Acquisition thread
    g_bAcquisitionFlag = false;
    pthread_join(g_nAcquisitonThreadID, NULL);

    //Device stop acquisition
    emStatus = GXStreamOff(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GX_VERIFY_EXIT(emStatus);
    }

    //Close device
    emStatus = GXCloseDevice(g_hDevice);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        g_hDevice = NULL;
        GXCloseLib();
        exit(0);
    }

    //Release libary
    emStatus = GXCloseLib();
    if(emStatus != GX_STATUS_SUCCESS)
    {
        GetErrorString(emStatus);
        exit(0);
    }

    printf("<App exit!>\n");
    system("pause");
    return 0;
}


//-------------------------------------------------
/**
\brief Acquisition thread function
\param pParam       thread param, used to transfer the ptr of Mat
\return void*
*/
//-------------------------------------------------
void *ProcGetImage(void* pAcquisitionThread)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    //Thread running flag setup
    AcquisitionThread* threadParam = (AcquisitionThread*)pAcquisitionThread;
    PGX_FRAME_BUFFER pFrameBuffer = NULL;
    uint32_t ui32FrameCount = 0;

    while(*(threadParam->g_AcquisitionFlag))
    {
        // Get a frame from Queue
        emStatus = GXDQBuf(threadParam->m_hDevice, &pFrameBuffer, 1000);
        if(emStatus != GX_STATUS_SUCCESS)
        {
            if (emStatus == GX_STATUS_TIMEOUT)
            {
                continue;
            }
            else
            {
                GetErrorString(emStatus);
                break;
            }
        }

        if(pFrameBuffer->nStatus != GX_FRAME_STATUS_SUCCESS)
        {
            printf("<Abnormal Acquisition: Exception code: %d>\n", pFrameBuffer->nStatus);
        }
        else
        {
            cv::Mat src;
            src.create(pFrameBuffer->nHeight,pFrameBuffer->nWidth,CV_8UC3);

            uchar* pBGRBuf = NULL;
            pBGRBuf = new uchar[pFrameBuffer->nHeight*pFrameBuffer->nWidth*3];

            //Convert raw8(bayer) image into BGR24 image
            VxInt32 emDXStatus = DX_OK;
            emDXStatus = DxRaw8toRGB24((unsigned char*)pFrameBuffer->pImgBuf, pBGRBuf, pFrameBuffer->nWidth, pFrameBuffer->nHeight,
                                RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(BAYERBG), false);
            if (emDXStatus != DX_OK)
            {
                printf("DxRaw8toRGB24 Failed, Error Code: %d\n", emDXStatus);
                delete[] pBGRBuf;
                pBGRBuf = NULL;
                continue;
            }
            else
            {
                memcpy(src.data,pBGRBuf,pFrameBuffer->nHeight*pFrameBuffer->nWidth*3);

                // producer 获取Mat图像接口
                pthread_mutex_lock(&Globalmutex);
                src.copyTo(*threadParam->m_pImage);
                imageReadable = true;
                pthread_cond_signal(&GlobalCondCV);
                pthread_mutex_unlock(&Globalmutex);

                delete[] pBGRBuf;
                pBGRBuf = NULL;
            }

            //输出采集到的图像信息
            //printf("<Successful acquisition: FrameCount: %u Width: %d Height: %d FrameID: %llu>\n",
            //        ui32FrameCount++, pFrameBuffer->nWidth, pFrameBuffer->nHeight, pFrameBuffer->nFrameID);

            emStatus = GXQBuf(threadParam->m_hDevice, pFrameBuffer);
            if(emStatus != GX_STATUS_SUCCESS)
            {
                GetErrorString(emStatus);
                break;
            }
       }
    }
    printf("<Acquisition thread Exit!>\n");
    return 0;
}

//----------------------------------------------------------------------------------
/**
\brief  Get description of input error code
\param  emErrorStatus  error code

\return void
*/
//----------------------------------------------------------------------------------
void GetErrorString(GX_STATUS emErrorStatus)
{
    char *error_info = NULL;
    size_t size = 0;
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Get length of error description
    emStatus = GXGetLastError(&emErrorStatus, NULL, &size);
    if(emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
        return;
    }

    // Alloc error resources
    error_info = new char[size];
    if (error_info == NULL)
    {
        printf("<Failed to allocate memory>\n");
        return ;
    }

    // Get error description
    emStatus = GXGetLastError(&emErrorStatus, error_info, &size);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        printf("<Error when calling GXGetLastError>\n");
    }
    else
    {
        printf("%s\n", (char*)error_info);
    }

    // Realease error resources
    if (error_info != NULL)
    {
        delete []error_info;
        error_info = NULL;
    }
}
