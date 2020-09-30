#include"Energy.h"

using namespace std;
using namespace cv;

void Energy::run(cv::Mat& src) {
	clearAll();
	initImage(src);
	if (show_process)imshow("bin", src);
	if (findArmors(src) < 1)return;//寻找所有大风车装甲板
	if (show_energy) showArmors("armor", src);
	if (!findFlowStripFan(src)) {//寻找图中流动条所在的扇叶
		if (!findFlowStripWeak(src)) return;
	}
	else {
		if (show_energy)showFlowStripFan("strip fan", src);
		if (!findTargetInFlowStripFan()) return;//在流动条区域内寻找装甲板
		if (!findFlowStrip(src)) return;//寻找流动条
	}
	if (show_energy) showFlowStrip("strip", src);
	findCenterROI(src);//寻找R标范围(缩小R标检测范围，可提高检测速度，降低误识别率）
	if (!findCenterR(src)) return;//寻找中心R
	if (show_energy) showCenterR("R", src);
	fans_cnt = findFans(src);//寻找图像中所有扇叶
	if (show_energy) showFans("fans", src);

	getTargetPolarAngle();//将待打击装甲板坐标转化为极坐标
	if (energy_rotation_init) {
		initRotation();
		return;
	}
	getPredictPoint(target_point);
	if (show_energy) showPredictPoint("Predict", src);
	if (show_data) showData(target_point,predict_point,circle_center_point);
}



int main() {
	Energy energy;
	Mat src;
	VideoCapture cap("H:/Photo/red_big.avi");
	cap.set(CAP_PROP_FRAME_WIDTH, 640);			//设置相机采样宽度
	cap.set(CAP_PROP_FRAME_HEIGHT, 480);		//设置相机采样高度
	cap.set(CAP_PROP_BRIGHTNESS, 20);			//设置相机采样亮度
	cap.set(CV_CAP_PROP_FPS, 200);				//设置相机采样帧率
	do {
		//时间消耗测试
		double t, tc;
		t = getTickCount();
		cap >> src;
		resize(src, src, Size(640, 480));
		energy.run(src);
		//时间消耗测试
		tc = (getTickCount() - t) / getTickFrequency();
		printf("time consume %.5f\n", tc);    //显示出耗费时间的多少
		waitKey(1);
	} while (true);
}