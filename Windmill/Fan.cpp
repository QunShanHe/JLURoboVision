#include"Energy.h"

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于寻找图像内所有的大风车扇叶
// ---------------------------------------------------------------------------------------------------------------------
int Energy::findFans(const cv::Mat& src) {
    if (src.empty()) {
        if (show_wrong) cout << "empty!" << endl;
        return 0;
    }
    static Mat src_bin;
    src_bin = src.clone();
    if (src.type() == CV_8UC3) {
        cvtColor(src_bin, src_bin, CV_BGR2GRAY);//若读取三通道视频文件，需转换为单通道
    }
    std::vector<vector<Point> > fan_contours;
    FanDilate(src_bin);//图像膨胀，防止图像断开并更方便寻找
    if (show_process)imshow("fan struct", src_bin);
    findContours(src_bin, fan_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    for (auto& fan_contour : fan_contours) {
        if (!isValidFanContour(src_bin, fan_contour)) {
            continue;
        }
        fans.emplace_back(cv::minAreaRect(fan_contour));
    }
    if (fans.size() < last_fans_cnt) {
        last_fans_cnt = static_cast<int>(fans.size());
        return -1;//寻找到的扇叶比上一帧少，说明该帧有误，返回-1
    }
    last_fans_cnt = static_cast<int>(fans.size());
    return static_cast<int>(fans.size());
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于判断找到的矩形候选区是否为扇叶
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::isValidFanContour(cv::Mat& src, const vector<cv::Point>& fan_contour) {
    double cur_contour_area = contourArea(fan_contour);
    if (cur_contour_area > _flow.flow_area_max ||
        cur_contour_area < _flow.flow_area_min) {
        //cout<<cur_contour_area<<" "<<energy_fan_param_.CONTOUR_AREA_MIN<<" "<<energy_fan_param_.CONTOUR_AREA_MAX<<endl;
        //cout<<"area fail."<<endl;
        return false;
        //选区面积大小不合适
    }
    RotatedRect cur_rect = minAreaRect(fan_contour);
    Size2f cur_size = cur_rect.size;
    float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;//将矩形的长边设置为长
    float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;//将矩形的短边设置为宽
    if (length < _flow.flow_length_min || width <_flow.flow_width_min ||
        length > _flow.flow_length_max || width > width > _flow.flow_width_max) {
        //cout<<"length width fail."<<endl;
//        cout << "length: " << length << '\t' << "width: " << width << '\t' << cur_rect.center << endl;
        return false;
        //矩形边长不合适
    }
    float length_width_ratio = length / width;//计算矩形长宽比
    if (length_width_ratio > _flow.flow_aim_max ||
        length_width_ratio < _flow.flow_aim_min) {
        //cout<<"length width ratio fail."<<endl;
//        cout << "HW: " << length_width_ratio << '\t' << cur_rect.center << endl;
        return false;
        //长宽比不合适
    }
    //    cout << cur_contour_area / cur_size.area() << endl;
    if (cur_contour_area / cur_size.area() < _flow.flow_area_ratio_min) {
        //        cout << cur_contour_area / cur_size.area() << endl;
        //        cout << "area ratio: " << cur_contour_area / cur_size.area() << '\t' << cur_rect.center << endl;
        return false;//轮廓对矩形的面积占有率不合适
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于显示图像中所有扇叶
// ---------------------------------------------------------------------------------------------------------------------
void Energy::showFans(std::string windows_name, const cv::Mat& src) {
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
    for (const auto& fan : fans) {
        Point2f vertices[4];      //定义矩形的4个顶点
        fan.points(vertices);   //计算矩形的4个顶点
        for (int i = 0; i < 4; i++)
            line(image2show, vertices[i], vertices[(i + 1) % 4], Scalar(255, 0, 0), 2);
    }
    imshow(windows_name, image2show);
}
