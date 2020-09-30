#include"Energy.h"
extern McuData mcu_data;

//----------------------------------------------------------------------------------------------------------------------
// 此函数为构造函数
// ---------------------------------------------------------------------------------------------------------------------
Energy::Energy() {
	initEnergy();
	initEnergyPartParam();//对能量机关参数进行初始化
}
//----------------------------------------------------------------------------------------------------------------------
// 此函数为能量机关析构函数，设置为默认
// ---------------------------------------------------------------------------------------------------------------------
Energy::~Energy() = default;

//----------------------------------------------------------------------------------------------------------------------
// 此函数对能量机关成员变量进行初始化
// ---------------------------------------------------------------------------------------------------------------------
void Energy::initEnergy() {
	show_energy=true;//是否显示图像
	show_process= false;//是否显示调试过程
	show_wrong = false;//是否显示报错
	show_data = true;//是否显示数据

	fans_cnt = 0;//扇叶个数初始化
    last_fans_cnt=0;//上一帧的扇叶个数
	last_target_polar_angle_judge_rotation = -1000;//上一帧待击打装甲板的极坐标角度（用于判断旋向）
	clockwise_rotation_init_cnt = 0;//装甲板顺时针旋转次数
	anticlockwise_rotation_init_cnt = 0;//装甲板逆时针旋转次数
	energy_rotation_init = true;//若仍在判断风车旋转方向，则为true
	predict_rad = 0;//预测提前角
	predict_rad_norm = 25;// 预测提前角的绝对值
	predict_point = Point(0, 0);//预测打击点初始化
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数对能量机关参数进行初始化
// ---------------------------------------------------------------------------------------------------------------------
void Energy::initEnergyPartParam() {
	_flow.BLUE_GRAY_THRESH = 100;//敌方红色时的阈值
	_flow.RED_GRAY_THRESH = 180;//敌方蓝色时的阈值

	_flow.armor_contour_area_max = 500; //装甲板的相关筛选参数
	_flow.armor_contour_area_min = 180;
	_flow.armor_contour_length_max = 50;
	_flow.armor_contour_length_min = 10;
	_flow.armor_contour_width_max = 30;
	_flow.armor_contour_width_min = 0;
	_flow.armor_contour_hw_ratio_max = 3;
	_flow.armor_contour_hw_ratio_min = 1;

	_flow.flow_strip_fan_contour_area_max = 2000;//流动条所在扇叶的相关筛选参数
	_flow.flow_strip_fan_contour_area_min = 500;
	_flow.flow_strip_fan_contour_length_max = 100;
	_flow.flow_strip_fan_contour_length_min = 60;
	_flow.flow_strip_fan_contour_width_max = 52;
	_flow.flow_strip_fan_contour_width_min = 20;
	_flow.flow_strip_fan_contour_hw_ratio_max = 2.8;
	_flow.flow_strip_fan_contour_hw_ratio_min = 1.2;
	_flow.flow_strip_fan_contour_area_ratio_max = 0.58;
	_flow.flow_strip_fan_contour_area_ratio_min = 0.34;

	_flow.Strip_Fan_Distance_max = 56;//流动条到装甲板距离参数
	_flow.Strip_Fan_Distance_min = 28;

	_flow.flow_strip_contour_area_max = 700;//流动条相关参数筛选
	_flow.flow_strip_contour_area_min = 50;
	_flow.flow_strip_contour_length_max = 55;
	_flow.flow_strip_contour_length_min = 40;//32
	_flow.flow_strip_contour_width_max = 20;
	_flow.flow_strip_contour_width_min = 4;
	_flow.flow_strip_contour_hw_ratio_min = 3;
	_flow.flow_strip_contour_hw_ratio_max = 7;
	_flow.flow_strip_contour_area_ratio_min = 0.6;
	_flow.flow_strip_contour_intersection_area_min = 100;

	_flow.target_intersection_contour_area_min = 40;//重合面积

	_flow.twin_point_max = 20;

	_flow.Center_R_Control_area_max = 200;//中心R标筛选相关参数
	_flow.Center_R_Control_area_min = 40;
	_flow.Center_R_Control_length_max = 20;
	_flow.Center_R_Control_length_min = 6;
	_flow.Center_R_Control_width_max = 20;
	_flow.Center_R_Control_width_min = 6;
	_flow.Center_R_Control_radio_max = 2;
	_flow.Center_R_Control_radio_min = 1;
	_flow.Center_R_Control_area_radio_min = 0.6;
	_flow.Center_R_Control_area_intersection_area_min = 10;

	_flow.flow_area_max = 5000;//扇叶筛选相关参数
	_flow.flow_area_min = 1500;
	_flow.flow_length_max = 100;
	_flow.flow_length_min = 45;
	_flow.flow_width_max = 52;
	_flow.flow_width_min = 10;
	_flow.flow_aim_max = 3.5;
	_flow.flow_aim_min = 1.2;
	_flow.flow_area_ratio_min = 0.6;
}


//----------------------------------------------------------------------------------------------------------------------
// 清空各个vector容器
// ---------------------------------------------------------------------------------------------------------------------
void Energy::clearAll() {
    fans.clear();
    armors.clear();
    flow_strip_fans.clear();
    target_armors.clear();
    flow_strips.clear();
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数对图像进行预处理操作
// ---------------------------------------------------------------------------------------------------------------------
void Energy::initImage(cv::Mat& src) {
	if (src.type() == CV_8UC3) {
		cvtColor(src, src, COLOR_BGR2GRAY);
	}
	if (mcu_data.enemy_color == ENEMY_BLUE) {
		threshold(src, src, _flow.BLUE_GRAY_THRESH, 255, THRESH_BINARY);
	}
	else if (mcu_data.enemy_color == ENEMY_RED) {
		threshold(src, src, _flow.RED_GRAY_THRESH, 255, THRESH_BINARY);
	}
	
	if (show_process) imshow("bin", src);
	if (show_energy || show_process)waitKey(1);

}
