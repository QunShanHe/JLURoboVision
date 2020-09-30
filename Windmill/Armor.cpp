#include"Energy.h"


//----------------------------------------------------------------------------------------------------------------------
// 此函数用于寻找图像内所有的大风车装甲板模块
// ---------------------------------------------------------------------------------------------------------------------
int Energy::findArmors(const cv::Mat& src) {
	if (src.empty()) {
        if (show_wrong) cout << "empty!" << endl;
        return 0;
    }
	static Mat dst;
	dst = src.clone();
	if (src.type() == CV_8UC3) {
		cvtColor(dst, dst, CV_BGR2GRAY);//若读取三通道视频文件，需转换为单通道
	}
	std::vector<vector<Point> > armor_contours;
	std::vector<vector<Point> > armor_contours_external;//用总轮廓减去外轮廓，只保留内轮廓，除去流动条的影响。

	ArmorDilate(dst);//图像膨胀，防止图像断开并更方便寻找
	if (show_process) imshow("Armorstuct", dst);
	findContours(dst, armor_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	findContours(dst, armor_contours_external, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i < armor_contours_external.size(); i++)//去除外轮廓
	{
		int external_contour_size = armor_contours_external[i].size();
		for (int j = 0; j < armor_contours.size(); j++) {
			int all_size = armor_contours[j].size();
			if (external_contour_size == all_size) {
				swap(armor_contours[j], armor_contours[armor_contours.size() - 1]);
				armor_contours.pop_back();//清除掉流动条
				break;
			}
		}

	}
	for (auto armor_contour : armor_contours) {
		if (!isValidArmorContour(armor_contour)) {
			continue;
		}
		armors.emplace_back(cv::minAreaRect(armor_contour));//回传所有装甲板到armors容器中
	}
	return static_cast<int>(armors.size());
}


//----------------------------------------------------------------------------------------------------------------------
// 此函数用于判断找到的装甲板尺寸是否合格
// ---------------------------------------------------------------------------------------------------------------------
bool Energy::isValidArmorContour(const vector<cv::Point>& armor_contour) {
	double cur_contour_area = contourArea(armor_contour);
	if (cur_contour_area > _flow.armor_contour_area_max ||
		cur_contour_area < _flow.armor_contour_area_min) {
		return false;
	}
	RotatedRect cur_rect = minAreaRect(armor_contour);
	Size2f cur_size = cur_rect.size;
	float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
	float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;
	if (length < _flow.armor_contour_length_min || width < _flow.armor_contour_width_min ||
		length >  _flow.armor_contour_length_max || width > _flow.armor_contour_width_max) {
		return false;
	}
	float length_width_ratio = length / width;
	if (length_width_ratio > _flow.armor_contour_hw_ratio_max ||
		length_width_ratio < _flow.armor_contour_hw_ratio_min) {
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数用于显示图像中所有装甲板
// ---------------------------------------------------------------------------------------------------------------------
void Energy::showArmors(std::string windows_name, const cv::Mat& src) {
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
	for (const auto& armor : armors) {
		Point2f vertices[4];      //定义矩形的4个顶点
		armor.points(vertices);   //计算矩形的4个顶点
		for (int i = 0; i < 4; i++)
			line(image2show, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 255), 2);
	}
	imshow(windows_name, image2show);
}