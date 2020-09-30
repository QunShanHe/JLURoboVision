#include"Energy.h"

//----------------------------------------------------------------------------------------------------------------------
// 此函数对能量机关旋转方向进行初始化
// ---------------------------------------------------------------------------------------------------------------------
void Energy::initRotation() {
    if (target_polar_angle >= -180 && last_target_polar_angle_judge_rotation >= -180
        && fabs(target_polar_angle - last_target_polar_angle_judge_rotation) < 30) {
        //target_polar_angle和last_target_polar_angle_judge_rotation的初值均为1000，大于-180表示刚开始几帧不要
        //若两者比较接近，则说明没有切换目标，因此可以用于顺逆时针的判断
        if (target_polar_angle < last_target_polar_angle_judge_rotation) clockwise_rotation_init_cnt++;
        else if (target_polar_angle > last_target_polar_angle_judge_rotation) anticlockwise_rotation_init_cnt++;
    }
    //由于刚开始圆心判断不准，角度变化可能计算有误，因此需要在角度正向或逆向变化足够大时才可确定是否为顺逆时针
    if (clockwise_rotation_init_cnt == 15) {
        energy_rotation_direction = 1;//顺时针变化30次，确定为顺时针
        cout << "rotation: " << energy_rotation_direction << endl;
        energy_rotation_init = false;
    }
    else if (anticlockwise_rotation_init_cnt == 15) {
        energy_rotation_direction = -1;//逆时针变化30次，确定为顺时针
        cout << "rotation: " << energy_rotation_direction << endl;
        energy_rotation_init = false;
    }
    last_target_polar_angle_judge_rotation = target_polar_angle;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数获取预测点坐标
// ---------------------------------------------------------------------------------------------------------------------
void Energy::getPredictPoint(cv::Point target_point) {
        if (energy_rotation_direction == 1) predict_rad = predict_rad_norm;
        else if (energy_rotation_direction == -1) predict_rad = -predict_rad_norm;
        rotate(target_point);
  
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于显示预测点
// ---------------------------------------------------------------------------------------------------------------------
void Energy::showPredictPoint(std::string windows_name, const cv::Mat& src) {
    if (src.empty())return;
    static Mat image2show;

    if (src.type() == CV_8UC1) // 黑白图像
    {
        cvtColor(src, image2show, COLOR_GRAY2RGB);

    }
    else if (src.type() == CV_8UC3) //RGB 彩色
    {
        image2show = src.clone();
    }
    cv::Point2f point = predict_point;
    cv::circle(image2show, point, 4, cv::Scalar(255, 0, 255));//在图像中画出特征点，2是圆的半径
    imshow(windows_name, image2show);
}
