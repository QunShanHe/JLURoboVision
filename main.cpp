#include"Armor/Armor.h"
#include"GxCamera/GxCamera.h"
#include<X11/Xlib.h>
using namespace cv;

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

int main(int argc, char** argv)
{

    XInitThreads();
    pthread_mutex_init(&Globalmutex,NULL);
    pthread_cond_init(&GlobalCondCV,NULL);
    pthread_create(&thread1,NULL,imageUpdatingThread,NULL);
    pthread_create(&thread2,NULL,armorDetectingThread,NULL);
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
    camera.setRoiParam(   400,            300,        0,      0);

    //   ExposureGain          autoExposure  autoGain  ExposureTime  AutoExposureMin  AutoExposureMax  Gain(<=16)  AutoGainMin  AutoGainMax  GrayValue
    camera.setExposureGainParam(    false,     false,      2000,          1000,              3000,         16,         5,            10,        127);

    //   WhiteBalance             Applied?       light source type
    camera.setWhiteBalanceParam(    true,    GX_AWB_LAMP_HOUSE_ADAPTIVE);

    //   Acquisition Start!
    camera.acquisitionStart(&src);
}

void* armorDetectingThread(void* PARAM)
{
    char ch;
    int targetNum = 2;
    detector.loadSVM("/home/mountain/Git/JLURoboVision/123svm.xml");  //for nano
    detector.setEnemyColor(BLUE); //here set enemy color
    usleep(1000000);

    double t,t1;
    do
    {

        //FPS
        t = getTickCount();

        //consumer
        pthread_mutex_lock(&Globalmutex);
        while (!imageReadable) {
            pthread_cond_wait(&GlobalCondCV,&Globalmutex);
        }
        detector.setImg(src);
        imageReadable = false;
        pthread_mutex_unlock(&Globalmutex);


        //操作手用，实时设置目标装甲板数字
        detector.setTargetNum(targetNum);

        //装甲板检测识别子核心集成函数
        detector.run(src);

        //给角度解算传目标装甲板值的实例
        vector<Point2f> imagePoints;
        ArmorType type;
        detector.getTargetInfo(imagePoints, type);

        //FPS(fps, 'E', src.clone());
        t1=(getTickCount()-t)/getTickFrequency();
        printf("*******************************************************fps:%f\n",1/t1);

        //装甲板检测识别调试参数是否输出
        //param:
        //		1.showSrcImg_ON,			  是否展示原图
        //		2.bool showSrcBinary_ON,  是否展示二值图
        //		3.bool showLights_ON,	  是否展示灯条图
        //		4.bool showArmors_ON,	  是否展示装甲板图
        //		5.bool textLights_ON,	  是否输出灯条信息
        //		6.bool textArmors_ON,	  是否输出装甲板信息
        //		7.bool textScores_ON		  是否输出打击度信息
        //					   1  2  3  4  5  6  7
        detector.showDebugInfo(0, 1, 1, 1, 0, 0, 0);

        ch = waitKey(1);
        if (ch == 'q' || ch == 27) break;
    } while (true);

}
