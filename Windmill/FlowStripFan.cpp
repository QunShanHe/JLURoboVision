#include"Energy.h"


//----------------------------------------------------------------------------------------------------------------------
// 此函数用于判断找到的矩形候选区是否为含流动条的扇叶
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::findFlowStripFan(const cv::Mat& src) {
    if (!src.data) {
        cout << "empty!" << endl;
        return 0;
    }
    static Mat src_bin;
    src_bin = src.clone();
    if (src.type() == CV_8UC3) {
        cvtColor(src_bin, src_bin, CV_BGR2GRAY);//若读取三通道视频文件，需转换为单通道
    }

    //ArmorDilate(src_bin);//图像膨胀，防止图像断开并更方便寻找
    vector<vector<Point> > flow_strip_fan_contours;
    FlowStripFanDilate(src_bin);//腐蚀膨胀
    if (show_process) imshow("flow strip fan struct", src_bin);
                            
    findContours(src_bin, flow_strip_fan_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<cv::RotatedRect> candidate_flow_strip_fans;

    for (auto& flow_strip_fan_contour : flow_strip_fan_contours) {
        if (!isValidFlowStripFanContour(src_bin, flow_strip_fan_contour)) {
            continue;
        }
        flow_strip_fans.emplace_back(cv::minAreaRect(flow_strip_fan_contour));
    }
    if (flow_strip_fans.empty()) {
        cout << "flow strip fan false!" << endl;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于判断找到的流动条扇叶尺寸是否合格
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::isValidFlowStripFanContour(cv::Mat& src, const vector<cv::Point>& flow_strip_fan_contour) {
    double cur_contour_area = contourArea(flow_strip_fan_contour);
    if (cur_contour_area > _flow.flow_strip_fan_contour_area_max ||
        cur_contour_area < _flow.flow_strip_fan_contour_area_min) {
        return false;
    }
    RotatedRect cur_rect = minAreaRect(flow_strip_fan_contour);
    Size2f cur_size = cur_rect.size;
    float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
    float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;
    if (length < _flow.flow_strip_fan_contour_length_min
        || width < _flow.flow_strip_fan_contour_width_min
        || length > _flow.flow_strip_fan_contour_length_max
        || width > _flow.flow_strip_fan_contour_width_max) {
        return false;
    }
    float length_width_ratio = length / width;
    if (length_width_ratio > _flow.flow_strip_fan_contour_hw_ratio_max ||
        length_width_ratio < _flow.flow_strip_fan_contour_hw_ratio_min) {
        return false;
    }
    if (cur_contour_area / cur_size.area() < _flow.flow_strip_fan_contour_area_ratio_min
        || cur_contour_area / cur_size.area() > _flow.flow_strip_fan_contour_area_ratio_max) {
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------------------------------------------------
// 此函数用于弱识别寻找流动条
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::findFlowStripWeak(const cv::Mat& src) {
    if (src.empty()) {
        if (show_wrong) cout << "empty!" << endl;
        return false;
    }
    cv::Mat src_bin;
    src_bin = src.clone();

    if (src_bin.type() == CV_8UC1) // 黑白图像
    {
        cvtColor(src_bin, src_bin, COLOR_GRAY2RGB);

    }
    std::vector<cv::RotatedRect> candidate_armors = armors;
    for (auto& candidate_armor : candidate_armors) {
        Point2f vertices[4];
        candidate_armor.size.height *= 1.3;
        candidate_armor.size.width *= 1.3;
        candidate_armor.points(vertices);   //计算矩形的4个顶点
        for (int i = 0; i < 4; i++) {
            line(src_bin, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 0), 20);
        }
    }

    cvtColor(src_bin, src_bin, CV_BGR2GRAY);//若读取三通道视频文件，需转换为单通道

    FlowStripDilate(src_bin);//图像膨胀，防止图像断开并更方便寻找
    if (show_process)imshow("weak Dilate", src_bin);

    std::vector<vector<Point> > flow_strip_contours;
    findContours(src_bin, flow_strip_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    for (auto& flow_strip_contour : flow_strip_contours) {
        if (!isValidFlowStripContour(flow_strip_contour)) {
            continue;
        }
        flow_strips.emplace_back(cv::minAreaRect(flow_strip_contour));
    }
    if (flow_strips.empty()) {
        if (show_wrong)cout << "weak flow strip false!" << endl;
        return false;
    }
    else {
        for (const auto& candidate_flow_strip : flow_strips) {
            for (const auto& candidate_armor : armors) {
                if (pointDistance(candidate_flow_strip.center, candidate_armor.center) <
                    _flow.Strip_Fan_Distance_max ||
                    pointDistance(candidate_flow_strip.center, candidate_armor.center) >
                    _flow.Strip_Fan_Distance_min) {
                    continue;
                }
                float angle_armor = candidate_armor.size.width > candidate_armor.size.height ? candidate_armor.angle :
                    candidate_armor.angle - 90;
                float angle_strip = candidate_flow_strip.size.width > candidate_flow_strip.size.height ?
                    candidate_flow_strip.angle : candidate_flow_strip.angle - 90;

                if (abs(angle_armor - angle_strip) < 60 || abs(angle_armor - angle_strip) > 120) {
                    continue;
                }
                target_armor = candidate_armor;
                target_point = candidate_armor.center;
                flow_strip = candidate_flow_strip;
                return true;
            }
        }
        if (show_wrong)cout << "weak flow strip false!" << endl;
        return false;
    }
}


//----------------------------------------------------------------------------------------------------------------------
// 此函数用于判断流动条的尺寸是否合格
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::isValidFlowStripContour(const vector<cv::Point>& flow_strip_contour) {
    double cur_contour_area = contourArea(flow_strip_contour);
    if (cur_contour_area > _flow.flow_strip_contour_area_max || cur_contour_area < _flow.flow_strip_contour_area_min) {
        if (show_wrong) cout << "FlowStrip_cur_contour_area" << cur_contour_area << endl;
        if (show_wrong) cout << "FlowStrip area fail." << endl;
        return false;
    }//流动条面积筛选

    RotatedRect cur_rect = minAreaRect(flow_strip_contour);
    Size2f cur_size = cur_rect.size;
    float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
    float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;
    if (length > _flow.flow_strip_contour_length_max || width > _flow.flow_strip_contour_width_max
        || length < _flow.flow_strip_contour_length_min || width < _flow.flow_strip_contour_width_min) {
        if (show_wrong)cout << "length width fail." << endl;
        return false;
    }//流动条长宽筛选
    float length_width_aim = length / width;
    if (length_width_aim<_flow.flow_strip_contour_hw_ratio_min || length_width_aim>_flow.flow_strip_contour_hw_ratio_max) {
        if (show_wrong) cout << "length_width_aim fail." << endl;
        return false;
    }//长宽比筛选
    if (cur_contour_area / cur_size.area() < _flow.flow_strip_contour_area_ratio_min) {
        if (show_wrong) cout << "cur_contour_area / cur_size.area() fail." << endl;
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于显示图像中流动条扇叶
// ---------------------------------------------------------------------------------------------------------------------
void Energy::showFlowStripFan(std::string windows_name, const cv::Mat& src) {
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

    Point2f strip_fan_vertices[4];      //定义矩形的4个顶点
    flow_strip_fan.points(strip_fan_vertices);   //计算矩形的4个顶点for (int i = 0; i < 4; i++)
    for (int i = 0; i < 4; i++)
        line(image2show, strip_fan_vertices[i], strip_fan_vertices[(i + 1) % 4], Scalar(127, 127, 255), 2);
    imshow(windows_name, image2show);
}


//----------------------------------------------------------------------------------------------------------------------
// 此函数在流动条区域内寻找装甲板
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::findTargetInFlowStripFan() {
    Mat draw(480, 640, CV_8UC3, Scalar(0, 0, 0));
    for (auto& candidate_flow_strip_fan : flow_strip_fans) {
        Point2f vertices[4];//定义矩形的4个顶点
        candidate_flow_strip_fan.points(vertices); //计算矩形的4个顶点
        for (int i = 0; i < 4; i++)
            line(draw, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 255), 2);
        for (int i = 0; i < armors.size(); ++i) {
            std::vector<cv::Point2f> intersection;
            if (rotatedRectangleIntersection(armors.at(i), candidate_flow_strip_fan, intersection) == 0)
                continue;//此函数为opencv中寻找两个旋转矩形的交叉部分的API（即判断重合面积是否为0）
            double cur_contour_area = contourArea(intersection);
            if (cur_contour_area > _flow.target_intersection_contour_area_min) {
                target_armors.emplace_back(armors.at(i));
                //cout << "Armor:" << target_armor.center << " ";
            }//返回目标装甲板参数
        }
    }
    if (target_armors.empty()) {
        if (show_wrong) cout << "find target armor false" << endl;
        return false;
    }
    else {
        return true;
    }
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于寻找流动条
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::findFlowStrip(const cv::Mat& src) {
    if (src.empty()) {
        if (show_wrong) cout << "empty!" << endl;
        return false;
    }
    cv::Mat src_bin;
    src_bin = src.clone();

    if (src_bin.type() == CV_8UC1) // 黑白图像
    {
        cvtColor(src_bin, src_bin, COLOR_GRAY2RGB);

    }
    std::vector<cv::RotatedRect> candidate_target_armors = target_armors;
    for (auto& candidate_target_armor : candidate_target_armors) {
        Point2f vertices[4];
        candidate_target_armor.size.height *= 1.3;
        candidate_target_armor.size.width *= 1.3;
        candidate_target_armor.points(vertices);   //计算矩形的4个顶点
        for (int i = 0; i < 4; i++) {
            line(src_bin, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 0), 20);
        }
    }

    cvtColor(src_bin, src_bin, CV_BGR2GRAY);//若读取三通道视频文件，需转换为单通道

    FlowStripDilate(src_bin);//图像膨胀，防止图像断开并更方便寻找
    if (show_process)imshow("flow strip struct", src_bin);

    std::vector<vector<Point> > flow_strip_contours;
    findContours(src_bin, flow_strip_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    for (auto candidate_flow_strip_fan : flow_strip_fans) {
        for (auto& flow_strip_contour : flow_strip_contours) {
            if (!isValidFlowStripContour(flow_strip_contour)) {
                continue;
            }

            std::vector<cv::Point2f> intersection;
            RotatedRect cur_rect = minAreaRect(flow_strip_contour);

            if (rotatedRectangleIntersection(cur_rect, candidate_flow_strip_fan, intersection) == 0) {
                continue;
            }
            else if (contourArea(intersection) > _flow.flow_strip_contour_intersection_area_min) {
                flow_strips.emplace_back(cv::minAreaRect(flow_strip_contour));
                //                cout << "intersection: " << contourArea(intersection) << '\t' << cur_rect.center << endl;
            }
            else {
                continue;
            }
        }
    }
    if (flow_strips.empty()) {
        if (show_wrong)cout << "flow strip false!" << endl;
        return false;
    }
    else if (flow_strips.size() > 1) {
        if (show_wrong)cout << "Too many flow strips!" << endl;
        return false;
    }
    else {
        flow_strip = flow_strips.at(0);
        for (auto& candidate_flow_strip_fan : flow_strip_fans) {
            std::vector<cv::Point2f> intersection;
            if (rotatedRectangleIntersection(flow_strip, candidate_flow_strip_fan, intersection) == 0) {
                continue;
            }
            else if (contourArea(intersection) > _flow.flow_strip_contour_intersection_area_min) {
                flow_strip_fan = candidate_flow_strip_fan;
            }
        }
        int i = 0;
        for (i = 0; i < target_armors.size(); ++i) {
            std::vector<cv::Point2f> intersection;
            if (rotatedRectangleIntersection(target_armors.at(i), flow_strip_fan, intersection) == 0)
                continue;//返回0表示没有重合面积
            double cur_contour_area = contourArea(intersection);
            if (cur_contour_area > _flow.target_intersection_contour_area_min) {
                target_armor = target_armors.at(i);
                target_point = target_armor.center;
            }
        }
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于显示图像中流动条扇叶
// ---------------------------------------------------------------------------------------------------------------------
void Energy::showFlowStrip(std::string windows_name, const cv::Mat& src) {
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

    Point2f strip_vertices[4];      //定义矩形的4个顶点
    flow_strip.points(strip_vertices);   //计算矩形的4个顶点
    for (int i = 0; i < 4; i++)
        line(image2show, strip_vertices[i], strip_vertices[(i + 1) % 4], Scalar(0, 255, 0), 2);

    for (const auto& armor : armors) {
        if (pointDistance(armor.center, target_point) < _flow.twin_point_max) {
            Point2f vertices[4];      //定义矩形的4个顶点
            armor.points(vertices);   //计算矩形的4个顶点
            for (int i = 0; i < 4; i++)
                line(image2show, vertices[i], vertices[(i + 1) % 4], Scalar(255, 255, 0), 2);
        }
    }

    Point2f ROI_vertices[4];      //定义矩形的4个顶点
    center_ROI.points(ROI_vertices);   //计算矩形的4个顶点
    for (int i = 0; i < 4; i++)
        line(image2show, ROI_vertices[i], ROI_vertices[(i + 1) % 4], Scalar(0, 0, 255), 2);
    imshow(windows_name, image2show);
}