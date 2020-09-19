#include"General/General.h"
#include"Armor/Armor.h"
#include"AngleSolver/AngleSolver.h"
#include"Serial/Serial.h"
#include<unistd.h>

//import armor detector
ArmorDetector detector;

//import angle solver
AngleSolver angleSolver;

int targetNum = 2;
Color ENEMYCOLOR = BLUE;
bool bRun = true;
double fps;

void* armorDetectingThread(void* PARAM)
{
    //Set armor detector prop
    detector.loadSVM("/home/mountain/Git/JLURoboVision/General/123svm.xml");

    //Set angle solver prop
    angleSolver.setCameraParam("/home/mountain/Git/JLURoboVision/General/camera_params.xml", 1);
    angleSolver.setArmorSize(SMALL_ARMOR,135,125);
    angleSolver.setArmorSize(BIG_ARMOR,230,127);
    angleSolver.setBulletSpeed(15000);
    usleep(1000000);

    double t,t1;
    do
    {
        // FPS
        t = getTickCount();

        detector.setEnemyColor(ENEMYCOLOR); //here set enemy color

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
        Point2f centerPoint;
        if(detector.isFoundArmor())
        {
            vector<Point2f> contourPoints;
            ArmorType type;
            detector.getTargetInfo(contourPoints, centerPoint, type);
            angleSolver.getAngle(contourPoints,centerPoint,SMALL_ARMOR,yaw,pitch,distance);
        }

        //串口在此获取信息 yaw pitch distance，同时设定目标装甲板数字
        Serial(yaw,pitch,true,detector.isFoundArmor());
        //操作手用，实时设置目标装甲板数字
        detector.setTargetNum(targetNum);

        //FPS
        t1=(getTickCount()-t)/getTickFrequency();
        printf("Armor Detecting FPS: %f\n",1/t1);
        if(detector.isFoundArmor()){
            printf("Found Target! Center(%d,%d)\n",centerPoint.x,centerPoint.y);
            cout<<"Yaw: "<<yaw<<"Pitch: "<<pitch<<"Distance: "<<distance<<endl;
        }

#ifdef DEBUG_MODE
        //********************** DEGUG **********************//
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
        detector.showDebugInfo(1, 1, 1, 1, 0, 0, 0);

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
            angleSolver.showDebugInfo(1, 0, 0, 0, 0, 0);
        }

        char chKey = waitKey(1);
        switch (chKey) {
        case '1':
            targetNum = 1;
            break;
        case '2':
            targetNum = 2;
            break;
        case '3':
            targetNum = 3;
            break;
        case 'b':
        case 'B':
            ENEMYCOLOR = BLUE;
            break;
        case 'r':
        case 'R':
            ENEMYCOLOR = RED;
            break;
        case 'q':
        case 'Q':
        case 27:
            bRun = false;
            break;
        default:
            break;
        }

#endif

    } while (bRun);
}
