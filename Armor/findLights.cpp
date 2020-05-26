/*
*	@Author: Mountain
*	@Date:	 2020.04.22
*	@Brief:  This cpp file define the function 'int findLights()' which is used to find lights
*/

#include"Armor.h"

/**
* @brief: find all the possible lights of armor  检测所有可能的灯条
*/
void ArmorDetector::findLights() {
	vector<vector<Point>> lightContours;  //candidate contours of lights roiIng中的候选灯条轮廓
	Mat contourImg; //image for the useage of findContours avoiding the unexpected change of itself 给findContours用的图像，防止findContours改变roiImg
	srcImg_binary.copyTo(contourImg); //a copy of roiImg, contourImg
	findContours(contourImg, lightContours, 0, 2); //CV_RETR_EXTERNAL = 0, CV_CHAIN_APPROX_SIMPLE = 2       最耗时的操作，优化方向
	RotatedRect lightRect;  //RotatedRect for fitEllipse 拟合椭圆来的灯条旋转矩形
	LightBar light;  //template light 临时灯条
	for (const auto& lightContour : lightContours) {
		if (lightContour.size() < 6) continue; //if contour's size is less than 6 , then it can not used to fitEllipse 轮廓点数小于6，不可拟合椭圆
		if (contourArea(lightContour) < armorParam.min_area) continue; //minarea of lightContour to filter some small blobs 面积筛选滤去小发光点

		lightRect = fitEllipse(lightContour); //lightContour fits into a RotatedRect 拟合椭圆
		light = LightBar(lightRect);//construct to a lightBar 构造为灯条

		if (abs(light.angle) > armorParam.max_angle) continue; //angle filter 角度筛选，滤去一些竖直偏角偏大的

		lights.emplace_back(light);
	}
	if (lights.size() < 2) {
		state = LIGHTS_NOT_FOUND; //if lights is less than 2, then set state not found lights 灯条少于两条则设置状态为没找到灯条
		return; //exit
	}

	// sort the lightBars from left to right 将灯条从左到右排序
	sort(lights.begin(), lights.end(),
		[](LightBar & a1, LightBar & a2) {
		return a1.center.x < a2.center.x; });
	state = LIGHTS_FOUND;
	return;
}
