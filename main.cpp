#include"Armor/Armor.h"
#include"GxCamera/GxCamera.h"
#include"AngleSolver/AngleSolver.h"
#include<X11/Xlib.h>

pthread_t thread1;
pthread_t thread2;
void* imageUpdatingThread(void* PARAM);
void* armorDetectingThread(void* PARAM);


pthread_mutex_t Globalmutex;
pthread_cond_t GlobalCondCV;
bool imageReadable = false;
void* param;

Mat src = Mat::zeros(600,800,CV_8UC3);


//import Galaxy Camera
GxCamera camera;

//import armor detector
ArmorDetector detector;

//import angle solver
AngleSolver angleSolver;

int main(int argc, char** argv)
{
    //For MutiTHread
    XInitThreads();
    //Init mutex
    pthread_mutex_init(&Globalmutex,NULL);
    //Init cond
    pthread_cond_init(&GlobalCondCV,NULL);
    //Create thread 1 -- image acquisition thread
    pthread_create(&thread1,NULL,imageUpdatingThread,NULL);
    //Create thread 2 -- armor Detection thread
    pthread_create(&thread2,NULL,armorDetectingThread,NULL);
    //Wait for children thread
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    pthread_mutex_destroy(&Globalmutex);
    return 0;
}


void* imageUpdatingThread(void* PARAM)
{
    //init camrea lib
    camera.initLib();

    //   open device      SN号
    camera.openDevice("KJ0190120002");

    //Attention:   (Width-64)%2=0; (Height-64)%2=0; X%16=0; Y%2=0;
    //   ROI             Width           Height       X       Y
    camera.setRoiParam(   640,            480,        80,     120);

    //   ExposureGain          autoExposure  autoGain  ExposureTime  AutoExposureMin  AutoExposureMax  Gain(<=16)  AutoGainMin  AutoGainMax  GrayValue
    camera.setExposureGainParam(    false,     true,      5000,          1000,              3000,         16,         5,            10,        127);

    //   WhiteBalance             Applied?       light source type
    camera.setWhiteBalanceParam(    true,    GX_AWB_LAMP_HOUSE_ADAPTIVE);

    //   Acquisition Start!
    camera.acquisitionStart(&src);
}

void* armorDetectingThread(void* PARAM)
{
    char ch;
    int targetNum = 2;

    //Set armor detector prop
    //detector.loadSVM("/home/mountain/Git/JLURoboVision/123svm.xml");
    detector.loadSVM("/home/robo-jlu/Git/JLURoboVision/123svm.xml");
    detector.setEnemyColor(BLUE); //here set enemy color

    //Set angle solver prop
    //angleSolver.setCameraParam("/home/mountain/Git/JLURoboVision/camera_params.xml", 1);
    angleSolver.setCameraParam("/home/robo-jlu/Git/JLURoboVision/camera_params.xml", 1);
    //angleSolver.setArmorSize(SMALL_ARMOR,700,800);
    //angleSolver.setArmorSize(BIG_ARMOR,700,800);
    angleSolver.setArmorSize(SMALL_ARMOR,43,40);
    angleSolver.setArmorSize(BIG_ARMOR,43,40);
    angleSolver.setBulletSpeed(15000);
    usleep(1000000);

    double t,t1;
    do
    {

        //FPS
        t = getTickCount();

        //consumer gets image
        pthread_mutex_lock(&Globalmutex);
        while (!imageReadable) {
            pthread_cond_wait(&GlobalCondCV,&Globalmutex);
        }
        detector.setImg(src);
        imageReadable = false;
        pthread_mutex_unlock(&Globalmutex);



        //装甲板检测识别子核心集成函数
        detector.run(src);

        //给角度解算传目标装甲板值的实例
        double yaw=0,pitch=0,distance=0;
        if(detector.isFoundArmor())
        {
            vector<Point2f> contourPoints;
            Point2f centerPoint;
            ArmorType type;
            detector.getTargetInfo(contourPoints, centerPoint, type);
            angleSolver.getAngle(contourPoints,centerPoint,SMALL_ARMOR,yaw,pitch,distance);
            cout<<"Yaw: "<<yaw<<"Pitch: "<<pitch<<"Distance: "<<distance<<endl;
        }

        //串口在此获取信息 yaw pitch distance，同时设定目标装甲板数字
        //cout<<"Yaw: "<<yaw<<"Pitch: "<<pitch<<"Distance: "<<distance<<endl;
        //操作手用，实时设置目标装甲板数字
        detector.setTargetNum(targetNum);


        //********************** DEGUG **********************//
        //FPS
        t1=(getTickCount()-t)/getTickFrequency();
        printf("***********************FPS:%f\n",1/t1);

        //装甲板检测识别调试参数是否输出
        //param:
        //		1.showSrcImg_ON,		  是否展示原图
        //		2.bool showSrcBinary_ON,  是否展示二值图
        //		3.bool showLights_ON,	  是否展示灯条图
        //		4.bool showArmors_ON,	  是否展示装甲板图
        //		5.bool textLights_ON,	  是否输出灯条信息
        //		6.bool textArmors_ON,	  是否输出装甲板信息
        //		7.bool textScores_ON	  是否输出打击度信息
        //					   1  2  3  4  5  6  7
        detector.showDebugInfo(0, 1, 1, 1, 0, 0, 0);

        if(detector.isFoundArmor())
        {
            //角度解算调试参数是否输出
            //param:
            //		1.showCurrentResult,	  是否展示当前解算结果
            //		2.bool showTVec,          是否展示目标坐标
            //		3.bool showP4P,           是否展示P4P算法计算结果
            //		4.bool showPinHole,       是否展示PinHole算法计算结果
            //		5.bool showCompensation,  是否输出补偿结果
            //		6.bool showCameraParams	  是否输出相机参数
            //					      1  2  3  4  5  6
            angleSolver.showDebugInfo(1, 1, 1, 1, 1, 0);
        }

        ch = waitKey(1);
        if (ch == 'q' || ch == 27) break;
    } while (true);
}
