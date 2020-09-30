#pragma once
#ifndef ENERGY_H

#include<iostream>
#include<opencv2/opencv.hpp>
#include<math.h>
#include"struct_define.h"

using namespace std;
using namespace cv;

#define PI 3.14;
#define ENEMY_BLUE 0//我方颜色为蓝色
#define ENEMY_RED  1//敌方颜色为红色

class Energy {
public:
	Energy();//构造函数
	~Energy();//析构函数

	bool is_big;//大符模式为true
	bool is_small;//小符模式为true
	void run(cv::Mat& src);//程序运行
	void showData(cv::Point target_point, cv::Point predict_point, cv::Point circle_center_point);//显示数据
	
private:
	WindmillParamFlow _flow;
	McuData mcu_data;

	bool show_energy; //是否显示图像
	bool show_process;//是否显示调试过程
	bool show_wrong;//是否显示报错
	bool show_data;//是否显示数据

	void initEnergy();//能量机关初始化
	void initEnergyPartParam();//能量机关参数初始化

	void clearAll();//清空所有容器vector
	void initImage(cv::Mat& src);//图像预处理
	void ArmorDilate(cv::Mat& src);//对装甲板的腐蚀和膨胀
	void FanDilate(cv::Mat& src);//对扇叶的腐蚀和膨胀
	void FlowStripDilate(cv::Mat& src);//对流动条的腐蚀和膨胀
	void FlowStripFanDilate(cv::Mat& src);//对流动条所在扇叶的腐蚀和膨胀
	void CenterRDilate(cv::Mat& src);//对中心R的腐蚀和膨胀

	void showArmors(std::string windows_name, const cv::Mat& src); //显示图像中所有装甲板
	void showFlowStripFan(std::string windows_name, const cv::Mat& src);//显示图像中流动条扇叶
	void showFlowStrip(std::string windows_name, const cv::Mat& src);//显示图像中流动条
	void showCenterR(std::string windows_name, const cv::Mat& src);//显示图像中的中心R
	void showFans(std::string windows_name, const cv::Mat& src);//显示图像中的扇叶

	std::vector<cv::RotatedRect> fans;//图像中所有扇叶
	std::vector<cv::RotatedRect> target_armors;//可能的目标装甲板
	std::vector<cv::RotatedRect> armors;//图像中所有可能装甲板
	std::vector<cv::RotatedRect> flow_strip_fans;//可能的流动扇叶
	std::vector<cv::RotatedRect> flow_strips;//可能的流动条

	cv::RotatedRect centerR;//风车中心字母R
	cv::RotatedRect center_ROI;//风车中心候选区
	cv::RotatedRect flow_strip_fan;//图像中流动条所在扇叶
	cv::RotatedRect flow_strip;//图像中的流动条
	cv::RotatedRect target_armor;//目标装甲板

	cv::Point circle_center_point;//风车圆心坐标
	cv::Point target_point;//目标装甲板中心坐标
	cv::Point predict_point;//预测的击打点坐标

	int  findArmors(const cv::Mat& src);//寻找大风车装甲板
	bool findCenterR(const cv::Mat& src);//寻找图中可能的风车中心字母R
	bool findFlowStrip(const cv::Mat& src);//寻找图中的流动条
	bool findCenterROI(const cv::Mat& src);//框取中心R候选区
	bool findFlowStripFan(const cv::Mat& src);//寻找图中的流动条所在扇叶
	bool findFlowStripWeak(const cv::Mat& src);//弱识别寻找图中的流动条
	bool findTargetInFlowStripFan();//在已发现的流动条区域中寻找待击打装甲板
	int  findFans(const cv::Mat& src);//

	bool isValidArmorContour(const vector<cv::Point>& armor_contour);//装甲板矩形尺寸要求
	bool isValidFanContour(cv::Mat& src, const vector<cv::Point>& fan_contour);//扇叶矩形尺寸要求
	bool isValidCenterRContour(const vector<cv::Point>& center_R_contour);//风车中心选区尺寸要求
	bool isValidFlowStripContour(const vector<cv::Point>& flow_strip_contour);//流动条扇叶矩形尺寸要求
	bool isValidFlowStripFanContour(cv::Mat& src, const vector<cv::Point>& flow_strip_fan_contour);//流动条扇叶矩形尺寸要求

	int fans_cnt;//扇叶个数
	int last_fans_cnt;//上一帧的扇叶个数
	int energy_rotation_direction;//风车旋转方向
	int clockwise_rotation_init_cnt;//装甲板顺时针旋转次数
	int anticlockwise_rotation_init_cnt;//装甲板逆时针旋转次数
	bool energy_rotation_init;//若仍在判断风车旋转方向，则为true
	double pointDistance(cv::Point point_1, cv::Point point_2);//计算两点距离
	void getTargetPolarAngle();//获得目标装甲板极坐标角度
	void initRotation();//对能量机关旋转方向进行初始化
	void getPredictPoint(cv::Point target_point);//获取预测点坐标
	void rotate(cv::Point target_point);// 计算预测的击打点坐标
	void showPredictPoint(std::string windows_name, const cv::Mat& src);//显示预测点
	float predict_rad;//预测提前角
	float predict_rad_norm;//预测提前角的绝对值
	float target_polar_angle;//待击打装甲板的极坐标角度
	float last_target_polar_angle_judge_rotation;//上一帧待击打装甲板的极坐标角度（用于判断旋向）


};



#endif //ENERGY_H