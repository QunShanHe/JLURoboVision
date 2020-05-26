#include "Armor.h"

ArmorParam armorParam = ArmorParam();

ArmorDetector::ArmorDetector(){}

ArmorDetector::~ArmorDetector(){}

void ArmorDetector::resetDetector()
{
	state = LIGHTS_NOT_FOUND;
	lights.clear();
	armors.clear();
}

/**
* @brief: set enemyColor  设置敌方颜色
*/
void ArmorDetector::setEnemyColor(Color enemyColor){
	this->enemyColor = enemyColor;
}

/**
* @brief: set enemyNum  设置目标装甲板数字
*/
void ArmorDetector::setTargetNum(const int & targetNum)
{
	this->targetNum = targetNum;
}

/**
* @brief: load source image and set roi if roiMode is open and found target in last frame 载入源图像并设置ROI区域（当ROI模式开启，且上一帧找到目标装甲板时）
* @param: const Mat& src     源图像的引用
*/
void ArmorDetector::setImg(Mat & src){
	src.copyTo(srcImg);  //deep copy src to srcImg 深（值）拷贝给srcImg
	classifier.loadImg(srcImg); //srcImg for classifier, warp perspective  载入classifier类成员的srcImg，用于透射变换剪切出装甲板图
	srcImg_binary = Mat::zeros(srcImg.size(), CV_8UC1); //color feature image

	//pointer visits all the data of srcImg, the same to bgr channel split 通道相减法的自定义形式，利用指针访问，免去了split、substract和thresh操作，加速了1.7倍
	//data of Mat  bgr bgr bgr bgr
	uchar *pdata = (uchar*)srcImg.data;
	uchar *qdata = (uchar*)srcImg_binary.data;
	int srcData = srcImg.rows * srcImg.cols;
	if (enemyColor == RED)
	{
		for (int i = 0; i < srcData; i++)
		{
			if (*(pdata + 2) - *pdata > armorParam.color_threshold)
				*qdata = 255;
			pdata += 3;
			qdata++;
		}
	}
	else if (enemyColor == BLUE)
	{
		for (int i = 0; i < srcData; i++)
		{
			if (*pdata - *(pdata+2) > armorParam.color_threshold)
				*qdata = 255;
			pdata += 3;
			qdata++;
		}
	}

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3)); //kernel for dilate;  shape:ellipse size:Size(3,3) 膨胀操作使用的掩膜
	dilate(srcImg_binary, srcImg_binary, kernel); //dilate the roiImg_binary which can make the lightBar area more smooth 对roiIng_binary进行膨胀操作，试得灯条区域更加平滑有衔接
}

/**
 *@brief: load SVM model 载入svm模型
 *@param: the path of svm xml file and the size of training images svm模型路径及训练集的图像大学
 */
void ArmorDetector::loadSVM(const char * model_path, Size armorImgSize)
{
	classifier.loadSvmModel(model_path, armorImgSize);
}

/**
 *@brief: an integrative function to run the Detector 集成跑ArmorDetector
 */
void ArmorDetector::run(Mat & src) {
	//firstly, load and set srcImg  首先，载入并处理图像
	this->setImg(src); //globally srcImg and preprocess it into srcImg_binary 载入Detector的全局源图像 并对源图像预处理成

	//secondly, reset detector before we findLights or matchArmors(clear lights and armors we found in the last frame and reset the state as LIGHTS_NOT_FOUND) 
	//随后，重设detector的内容，清空在上一帧中找到的灯条和装甲板，同时检测器状态重置为LIGHTS_NOT_FOUND（最低状态）
	resetDetector();

	//thirdly, find all the lights in the current frame (srcImg)
	//第三步，在当前图像中找出所有的灯条
	findLights();

	//forthly, if the state is LIGHTS_FOUND (detector found more than two lights) , we match each two lights into an armor
	//第四步，如果状态为LIGHTS_FOUND（找到多于两个灯条），则
	if (state == LIGHTS_FOUND)
	{
		//match each two lights into an armor and if the armor is a suitable one, emplace back it into armors
		//将每两个灯条匹配为一个装甲板，如果匹配出来的装甲板是合适的，则压入armors中
		matchArmors();

		//if the state is ARMOR_FOUND(detector has matched suitable armor), set target armor and last armor
		//如果找到了灯条，则设置好目标装甲板和上一个装甲板
		if (state == ARMOR_FOUND) {
			setTargetArmor();
		}
	}
}

/**
 *@brief: get the vertices and type of target Armor for angle solver 将detector的结果输出
 */
void ArmorDetector::getTargetInfo(vector<Point2f>& armorVertices, ArmorType & type)
{
	armorVertices = targetArmor.armorVertices;
	type = targetArmor.type;
}





///////////////////////////////////////////////////////////  functions  for   debugging      //////////////////////////////////////////////////////////////////

/**
 *@brief: show all the lights found in a copy of srcImg  在图像中显示找到的所有灯条
 */
void showLights(Mat & image, const vector<LightBar> & lights)
{
	Mat lightDisplay;//image for the use of dialaying the lights 显示灯条用的图像
	image.copyTo(lightDisplay);//get a copy of srcImg 获取源图像的拷贝
	//if detector finds lights 如果找到了灯条
	if (!lights.empty())
	{
		putText(lightDisplay, "LIGHTS FOUND!", Point(100, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 1, 8, false); //title LIGHT_FOUND 大标题 “找到了灯条”
		for (auto light : lights)
		{
			Point2f lightVertices[4];
			light.lightRect.points(lightVertices);
			//draw all the lights' contours 画出所有灯条的轮廓
			for (size_t i = 0; i < 4; i++)
			{
				line(lightDisplay, lightVertices[i], lightVertices[(i + 1) % 4], Scalar(255, 0, 255), 1, 8, 0);
			}

			//draw the lights's center point 画出灯条中心
			circle(lightDisplay, light.center, 2, Scalar(0, 255, 0), 2, 8, 0);

			//show the lights' center point x,y value 显示灯条的中心坐标点
			putText(lightDisplay, to_string(int(light.center.x)), light.center, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
			putText(lightDisplay, to_string(int(light.center.y)), light.center + Point2f(0, 15), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0), 1, 8, false);
		}
	}
	//if detector does not find lights 如果没找到灯条
	else
	{
		putText(lightDisplay, "LIGHTS NOT FOUND!", Point(100, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 1, 8, false);//title LIGHT_NOT_FOUND 大标题 “没找到灯条”
	}
	//show the result image 显示结果图
	imshow("Lights Monitor", lightDisplay);
}

/**
 *@brief: show all the armors matched in a copy of srcImg  在图像中显示找到的所有装甲板
 */
void showArmors(Mat & image, const vector<ArmorBox> & armors, const ArmorBox & targetArmor)
{
	Mat armorDisplay; //Image for the use of displaying armors 展示装甲板的图像
	image.copyTo(armorDisplay); //get a copy of srcImg 源图像的拷贝 
	// if armors is not a empty vector (ARMOR_FOUND) 如果找到了装甲板
	if (!armors.empty())
	{
		putText(armorDisplay, "ARMOR FOUND!", Point(100, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 0), 1, 8, false); //title FOUND 大标题 “找到了装甲板”
		//draw all the armors' vertices and center 画出所有装甲板的顶点边和中心
		for (auto armor : armors)
		{
			//draw the center 画中心
			circle(armorDisplay, armor.center, 2, Scalar(0, 255, 0), 2);
			for (size_t i = 0; i < 4; i++)
			{
				line(armorDisplay, armor.armorVertices[i], armor.armorVertices[(i + 1) % 4], Scalar(255, 255, 0), 2, 8, 0);
			}
			//display its center point x,y value 显示中点坐标
			putText(armorDisplay, to_string(int(armor.center.x)), armor.center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 255), 1, 8, false);
			putText(armorDisplay, to_string(int(armor.center.y)), armor.center + Point2f(0, 15), FONT_HERSHEY_PLAIN, 1, Scalar(255, 0, 255), 1, 8, false);
            putText(armorDisplay, to_string(int(armor.armorNum)), armor.center + Point2f(15, 30), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1, 8, false);
		}
		//connect all the vertices to be the armor contour 画出装甲板轮廓
		for (size_t i = 0; i < 4; i++)
		{
			line(armorDisplay, targetArmor.armorVertices[i], targetArmor.armorVertices[(i + 1) % 4], Scalar(255, 255, 255), 2, 8, 0);
		}
	}
	//if armors is a empty vector (ARMOR_NOT FOUND) 如果没找到装甲板
	else
	{
		putText(armorDisplay, "ARMOR NOT FOUND!", Point(100, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);//title NOT FOUND 大标题 “没找到装甲板”
	}
	//show the result armors image 显示结果图
	imshow("Armor Monitor", armorDisplay);
}

/**
 *@brief: show all the lights information in console  在控制台输出找到灯条的中心和角度
 */
void textLights(vector<LightBar> & lights)
{
	cout << "\n################## L I G H T S ##################" << endl;
	if (lights.empty()) {
		cout << "LIGHTS NOT FOUND!" << endl;
	}
	else
	{
		cout << "LIGHTS FOUND!" << endl;
		for (size_t i = 0; i < lights.size(); i++)
		{
			cout << "#############################" << endl;
			cout << "Light Center:" << lights[i].center << endl;
			cout << "Light Angle:" << lights[i].angle << endl;
		}
		cout << "#################################################" << endl;
	}
}

/**
 *@brief: show all the armors information in console  在控制台输出找到装甲板的中心、数字、匹配信息
 */
void textArmors(vector<ArmorBox> & armors)
{
	cout << "\n$$$$$$$$$$$$$$$$$$ A R M O R S $$$$$$$$$$$$$$$$$$" << endl;
	if (armors.empty()) {
		cout << "ARMORS NOT FOUND!" << endl;
	}
	else
	{
		cout << "ARMOR FOUND!" << endl;
		for (size_t i = 0; i < armors.size(); i++)
		{
			cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
			cout << "Armor Center: " << armors[i].center << endl;
			cout << "Armor Number: " << armors[i].armorNum << endl;
			if (armors[i].type == SMALL_ARMOR) cout << "Armor Type: SMALL ARMOR" << endl;
			else if(armors[i].type == BIG_ARMOR) cout << "Armor Type: BIG ARMOR" << endl;
			cout << "\n###### matching information ######" << endl;
			cout << "Angle difference: " << armors[i].getAngleDiff() << endl;
			cout << "Deviation Angle: " << armors[i].getDeviationAngle() << endl;
			cout << "X Dislocation Ration: " << armors[i].getDislocationX() << endl;
			cout << "Y Dislocation Ration: " << armors[i].getDislocationY() << endl;
			cout << "Length Ration: " << armors[i].getLengthRation() << endl;
		}
		cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	}

}

/**
 *@brief: show all the armors score information in console  在控制台输出找到装甲板的打击度信息
 */
void textScores(vector<ArmorBox> & armors, ArmorBox & lastArmor)
{
	if (!armors.empty())
	{
		cout << "\n@@@@@@@@@@@@@@@@@@ S C O R E S @@@@@@@@@@@@@@@@@@" << endl;
		for (size_t i = 0; i < armors.size(); i++)
		{
			float score = 0;  // shooting value of armor的打击度
			cout << "Armor Center: " << armors[i].center << endl;
			cout << "Area: " << armors[i].armorRect.area() << endl;
			score += armors[i].armorRect.area(); //area value of a a_armor面积得分


			if (lastArmor.armorNum != 0) {  //if lastArmor.armorRect is not a default armor means there is a true targetArmor in the last frame 上一帧图像中存在目标装甲板
				float a_distance = getPointsDistance(armors[i].center, lastArmor.center); //distance score to the lastArmor(if exist) 装甲板距离得分，算负分
				cout << "Distance: " << a_distance << endl;
				score -= a_distance * 2;
			}
		}
		cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
	}
}

/**
 *@brief: lights, armors, lights to armors every information in one 所有调试用数据输出
 */
void ArmorDetector::showDebugInfo(bool showSrcImg_ON, bool showSrcBinary_ON, bool showLights_ON, bool showArmors_ON, bool textLights_ON, bool textArmors_ON, bool textScores_ON)
{
	if (showSrcImg_ON)
		imshow("srcImg", srcImg);
	if (showSrcBinary_ON)
		imshow("srcImg_Binary", srcImg_binary);
	if (showLights_ON)
		showLights(srcImg, lights);
	if (showArmors_ON)
		showArmors(srcImg, armors, targetArmor);
	if (textLights_ON)
		textLights(lights);
	if (textArmors_ON)
		textArmors(armors);
	if (textScores_ON)
		textScores(armors, lastArmor);
}
