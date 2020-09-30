#pragma once
#ifndef STRUCT_DEFINE_H
#define STRUCT_DEFINE_H
#include<opencv2/opencv.hpp>
#include<iostream>


struct WindmillParamFlow {
	int RED_GRAY_THRESH;//敌方红色时的阈值
	int BLUE_GRAY_THRESH;//敌方蓝色时的阈值
	float armor_contour_area_max;//装甲板的相关筛选参数
	float armor_contour_area_min;
	float armor_contour_length_max;
	float armor_contour_length_min;
	float armor_contour_width_max;
	float armor_contour_width_min;
	float armor_contour_hw_ratio_max;
	float armor_contour_hw_ratio_min;

	float flow_strip_fan_contour_area_max;//流动条所在扇叶的相关筛选参数
	float flow_strip_fan_contour_area_min;
	float flow_strip_fan_contour_length_max;
	float flow_strip_fan_contour_length_min;
	float flow_strip_fan_contour_width_max;
	float flow_strip_fan_contour_width_min;
	float flow_strip_fan_contour_hw_ratio_max;
	float flow_strip_fan_contour_hw_ratio_min;
	float flow_strip_fan_contour_area_ratio_max;
	float flow_strip_fan_contour_area_ratio_min;

	float Strip_Fan_Distance_max;//流动条到装甲板距离参数
	float Strip_Fan_Distance_min;

	float flow_strip_contour_area_max;//流动条相关参数筛选
	float flow_strip_contour_area_min;
	float flow_strip_contour_length_max;
	float flow_strip_contour_length_min;
	float flow_strip_contour_width_max;
	float flow_strip_contour_width_min;
	float flow_strip_contour_hw_ratio_max;
	float flow_strip_contour_hw_ratio_min;
	float flow_strip_contour_area_ratio_min;
	float flow_strip_contour_intersection_area_min;

	long target_intersection_contour_area_min;

	float twin_point_max;

	float Center_R_Control_area_max;//中心R的相关参数筛选
	float Center_R_Control_area_min;
	float Center_R_Control_length_max;
	float Center_R_Control_length_min;
	float Center_R_Control_width_max;
	float Center_R_Control_width_min;
	float Center_R_Control_radio_max;
	float Center_R_Control_radio_min;
	float Center_R_Control_area_radio_min;
	float Center_R_Control_area_intersection_area_min;

	float flow_area_max;//扇叶相关参数筛选
	float flow_area_min;
	float flow_length_max;
	float flow_length_min;
	float flow_width_max;
	float flow_width_min;
	float flow_aim_max;
	float flow_aim_min;
	float flow_area_ratio_min;
};

struct McuData {
	float curr_yaw;      // 当前云台yaw角度
	float curr_pitch;    // 当前云台pitch角
	uint8_t state;       // 当前状态，自瞄-大符-小符
	uint8_t mark;        // 云台角度标记位
	uint8_t anti_top;    // 是否为反陀螺模式
	uint8_t enemy_color; // 敌方颜色
	int delta_x;         // 能量机关x轴补偿量
	int delta_y;         // 能量机关y轴补偿量
};












#endif