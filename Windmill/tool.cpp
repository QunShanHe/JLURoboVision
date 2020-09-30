#include"Energy.h"

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于计算两点之间距离
// ---------------------------------------------------------------------------------------------------------------------
double Energy::pointDistance(cv::Point point_1, cv::Point point_2) {
	double distance = 0;
	distance = sqrt(pow(static_cast<double>(point_1.x - point_2.x), 2) + pow(static_cast<double>(point_1.y - point_2.y), 2));
	return distance;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数获取目标装甲板极坐标角度
// ---------------------------------------------------------------------------------------------------------------------
void Energy::getTargetPolarAngle() {
	target_polar_angle = static_cast<float>(180 / 3.14 * atan2((-1 * (target_point.y - circle_center_point.y)), (target_point.x - circle_center_point.x)));
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于计算预测的击打点坐标
// ---------------------------------------------------------------------------------------------------------------------
void Energy::rotate(cv::Point target_point) {
    int x1, x2, y1, y2;
    //    为了减小强制转换的误差
    x1 = circle_center_point.x * 100;
    x2 = target_point.x * 100;
    y1 = circle_center_point.y * 100;
    y2 = target_point.y * 100;

    predict_point.x = static_cast<int>(
        (x1 + (x2 - x1) * cos(-predict_rad * 3.14 / 180.0) - (y1 - y2) * sin(-predict_rad * 3.14 / 180.0)) / 100);
    predict_point.y = static_cast<int>(
        (y1 - (x2 - x1) * sin(-predict_rad * 3.14 / 180.0) - (y1 - y2) * cos(-predict_rad * 3.14 / 180.0)) / 100);
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于显示数据
// ---------------------------------------------------------------------------------------------------------------------
void Energy::showData(cv::Point target_point, cv::Point predict_point, cv::Point circle_center_point) {
    cout <<" ****************************************"<<endl;
    cout << "circle_center_point=" << circle_center_point << endl;
    cout << " target_point=" << target_point << endl;
    cout << "predict_point=" << predict_point << endl;
}