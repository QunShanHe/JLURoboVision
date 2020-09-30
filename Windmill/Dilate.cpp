#include"Energy.h"

//----------------------------------------------------------------------------------------------------------------------
// 此函数对图像进行腐蚀与膨胀操作
// ---------------------------------------------------------------------------------------------------------------------
void Energy::ArmorDilate(cv::Mat& src) {
	Mat element_dilate_1 = getStructuringElement(MORPH_RECT, Size(5, 5));
	Mat element_erode_1 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_2 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_3 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_3 = getStructuringElement(MORPH_RECT, Size(1, 1));

	dilate(src, src, element_dilate_1);
	erode(src, src, element_erode_1);

}

//----------------------------------------------------------------------------------------------------------------------
// 此函数对图像进行腐蚀与膨胀操作
// ---------------------------------------------------------------------------------------------------------------------
void Energy::FlowStripFanDilate(cv::Mat& src) {
	Mat element_dilate_1 = getStructuringElement(MORPH_RECT, Size(5, 5));
	Mat element_erode_1 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_2 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_3 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_3 = getStructuringElement(MORPH_RECT, Size(1, 1));

	dilate(src, src, element_dilate_1);
	erode(src, src, element_erode_1);
	erode(src, src, element_erode_2);
	erode(src, src, element_erode_3);
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数对图像进行腐蚀与膨胀操作
// ---------------------------------------------------------------------------------------------------------------------
void Energy::FanDilate(cv::Mat& src) {
	Mat element_dilate_1 = getStructuringElement(MORPH_RECT, Size(5, 5));
	Mat element_erode_1 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_2 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_3 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_3 = getStructuringElement(MORPH_RECT, Size(1, 1));

	dilate(src, src, element_dilate_1);
	erode(src, src, element_erode_1);
	erode(src, src, element_erode_2);
	erode(src, src, element_erode_3);
}



//----------------------------------------------------------------------------------------------------------------------
// 此函数对图像进行腐蚀与膨胀操作
// ---------------------------------------------------------------------------------------------------------------------
void Energy::FlowStripDilate(cv::Mat& src) {
	Mat element_dilate_1 = getStructuringElement(MORPH_RECT, Size(5, 5));
	Mat element_erode_1 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_2 = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat element_dilate_3 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_3 = getStructuringElement(MORPH_RECT, Size(1, 1));

	dilate(src, src, element_dilate_1);
	erode(src, src, element_erode_1);
	erode(src, src, element_erode_2);
	erode(src, src, element_erode_3);
}

//----------------------------------------------------------------------------------------------------------------------
// 此函数对图像进行腐蚀与膨胀操作
// ---------------------------------------------------------------------------------------------------------------------
void Energy::CenterRDilate(cv::Mat& src) {
	Mat element_dilate_1 = getStructuringElement(MORPH_RECT, Size(4, 4));
	Mat element_erode_1 = getStructuringElement(MORPH_RECT, Size(2, 1));
	Mat element_dilate_2 = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element_erode_2 = getStructuringElement(MORPH_RECT, Size(4, 4));

	erode(src, src, element_erode_1);
	dilate(src, src, element_dilate_1);
}