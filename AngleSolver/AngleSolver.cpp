#include "AngleSolver.h"

AngleSolver::AngleSolver()
{
}

AngleSolver::~AngleSolver()
{
}

void AngleSolver::setCameraParam(const cv::Mat & camera_matrix, const cv::Mat & distortion_coeff)
{
	camera_matrix.copyTo(CAMERA_MATRIX);
	distortion_coeff.copyTo(DISTORTION_COEFF);
}

int AngleSolver::setCameraParam(const char * filePath, int camId)
{
	FileStorage fsRead;
	fsRead.open(filePath, FileStorage::READ);
	if (!fsRead.isOpened())
	{
		cout << "Failed to open xml" << endl;
		return -1;
	}

	fsRead["Y_DISTANCE_BETWEEN_GUN_AND_CAM"] >> GUN_CAM_DISTANCE_Y;

	Mat camera_matrix;
	Mat distortion_coeff;
	switch (camId)
	{
	case 1:
		fsRead["CAMERA_MATRIX_1"] >> camera_matrix;
		fsRead["DISTORTION_COEFF_1"] >> distortion_coeff;
		break;
	case 2:
        fsRead["CAMERA_MATRIX_2"] >> camera_matrix;
		fsRead["DISTORTION_COEFF_2"] >> distortion_coeff;
		break;
	case 3:
        fsRead["CAMERA_MATRIX_3"] >> camera_matrix;
		fsRead["DISTORTION_COEFF_3"] >> distortion_coeff;
		break;
	default:
		cout << "WRONG CAMID GIVEN!" << endl;
		break;
	}
	setCameraParam(camera_matrix, distortion_coeff);
	fsRead.release();
	return 0;
}

void AngleSolver::setArmorSize(ArmorType type, double width, double height)
{
	double half_x = width / 2.0;
	double half_y = height / 2.0;
	switch (type)
	{
	case SMALL_ARMOR:
		SMALL_ARMOR_POINTS_3D.push_back(Point3f(-half_x, half_y, 0));   //tl top left
		SMALL_ARMOR_POINTS_3D.push_back(Point3f(half_x, half_y, 0));	//tr top right
		SMALL_ARMOR_POINTS_3D.push_back(Point3f(half_x, -half_y, 0));   //br below right
		SMALL_ARMOR_POINTS_3D.push_back(Point3f(-half_x, -half_y, 0));  //bl below left
		break;

	case BIG_ARMOR:
		BIG_ARMOR_POINTS_3D.push_back(Point3f(half_x, -half_y, 0));   //tl top left
		BIG_ARMOR_POINTS_3D.push_back(Point3f(half_x, half_y, 0));    //tr top right
		BIG_ARMOR_POINTS_3D.push_back(Point3f(-half_x, half_y, 0));   //bl below left
		BIG_ARMOR_POINTS_3D.push_back(Point3f(-half_x, -half_y, 0));  //br below right
		break;
	default: break;
	}
}

void AngleSolver::setBulletSpeed(int bulletSpeed)
{
	BULLET_SPEED = bulletSpeed;
}

void AngleSolver::setTarget(vector<Point2f> contourPoints, Point2f centerPoint, ArmorType type)
{
	targetContour = contourPoints;
	targetCenter = centerPoint;
	targetType = type;
}

void AngleSolver::solveAngles()
{
	Mat _rvec;
	switch (targetType)
	{
	case SMALL_ARMOR:
        solvePnP(SMALL_ARMOR_POINTS_3D, targetContour, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec, false, SOLVEPNP_ITERATIVE); break;
	case BIG_ARMOR:
        solvePnP(BIG_ARMOR_POINTS_3D, targetContour, CAMERA_MATRIX, DISTORTION_COEFF, _rvec, tVec, false, SOLVEPNP_ITERATIVE); break;
	default:
		break;
	}

	GUN_CAM_DISTANCE_Y = 0;
	tVec.at<double>(1, 0) -= GUN_CAM_DISTANCE_Y;
	double x_pos = tVec.at<double>(0, 0);
	double y_pos = tVec.at<double>(1, 0);
	double z_pos = tVec.at<double>(2, 0);
	distance = sqrt(x_pos * x_pos + y_pos * y_pos + z_pos * z_pos);
	

	// Target is too far, using PinHole solver
    if (distance > 5000)
	{
		PinHole_solver();
	}
	// Target is moderate, using PnP solver
	else
	{
		P4P_solver();
	}
}

void AngleSolver::P4P_solver()
{
	double x_pos = tVec.at<double>(0, 0);
	double y_pos = tVec.at<double>(1, 0);
	double z_pos = tVec.at<double>(2, 0);

	double tan_pitch = y_pos / sqrt(x_pos*x_pos + z_pos * z_pos);
	double tan_yaw = x_pos / z_pos;
	x_pitch = -atan(tan_pitch) * 180 / CV_PI;
    y_yaw = atan(tan_yaw) * 180 / CV_PI;
}

void AngleSolver::PinHole_solver()
{
	double fx = CAMERA_MATRIX.at<double>(0, 0);
	double fy = CAMERA_MATRIX.at<double>(1, 1);
	double cx = CAMERA_MATRIX.at<double>(0, 2);
	double cy = CAMERA_MATRIX.at<double>(1, 2);
	Point2f pnt;
	vector<cv::Point2f> in;
	vector<cv::Point2f> out;
	in.push_back(targetCenter);
	
	//对像素点去畸变
	undistortPoints(in, out, CAMERA_MATRIX, DISTORTION_COEFF, noArray(), CAMERA_MATRIX);
	pnt = out.front();

	//去畸变后的比值
	double rxNew = (pnt.x - cx) / fx;
	double ryNew = (pnt.y - cy) / fy;

	y_yaw = atan(rxNew) / CV_PI * 180;
	x_pitch = -atan(ryNew) / CV_PI * 180;
}

void AngleSolver::compensateAngle()
{
    compensateOffset();
    compensateGravity();
}

void AngleSolver::compensateOffset()
{
    float camera_target_height = distance * sin(x_pitch / 180 * CV_PI);
    float gun_target_height = camera_target_height + GUN_CAM_DISTANCE_Y;
    float gun_pitch_tan = gun_target_height / (distance * cos(x_pitch / 180 * CV_PI));
    x_pitch = atan(gun_pitch_tan) / CV_PI * 180;
}

void AngleSolver::compensateGravity()
{
    float compensateGravity_pitch_tan = tan(x_pitch/180*CV_PI) + (0.5*9.8*(distance / BULLET_SPEED)*(distance / BULLET_SPEED)) / cos(x_pitch/180*CV_PI);
    x_pitch = atan(compensateGravity_pitch_tan)/CV_PI*180;
}

void AngleSolver::getAngle(vector<Point2f>& contourPoints,Point2f cenerPoint, ArmorType type, double & yaw, double & pitch, double & evaluateDistance)
{
	setTarget(contourPoints, cenerPoint, type);
	solveAngles();
    compensateAngle();
	yaw = y_yaw;
	pitch = x_pitch;
	evaluateDistance = distance;
}

void AngleSolver::showDebugInfo(bool showCurrentResult, bool showTVec, bool showP4P, bool showPinHole, bool showCompensation, bool showCameraParams)
{
    if(showCurrentResult)
    {
        Mat angleImage = Mat::zeros(250,600,CV_8UC3);
        putText(angleImage, "Yaw: " + to_string(y_yaw), Point(100, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);
        putText(angleImage, "Pitch: " + to_string(x_pitch), Point(100, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);
        putText(angleImage, "Distance: " + to_string(distance), Point(100, 150), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);
        putText(angleImage, "X:" + to_string((int)(tVec.at<double>(0))), Point(50, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);
        putText(angleImage, "Y:" + to_string((int)(tVec.at<double>(1))), Point(250, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);
        putText(angleImage, "Z:" + to_string((int)(tVec.at<double>(2))), Point(400, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 255), 1, 8, false);
        imshow("AngleSolver",angleImage);
    }
    if(showTVec)
    {
        cout << "tVec:" << endl;
        cout << " X:" << tVec.at<double>(0, 0);
        cout << " Y:" << tVec.at<double>(1, 0);
        cout << " Z:" << tVec.at<double>(2, 0);
        cout << endl;
        cout << "-----------------------------------------------" << endl;
    }
    float yaw_temp = y_yaw, pitch_temp = x_pitch;
    if(showP4P)
    {
        P4P_solver();
        cout<<"P4P Solver:"<<endl;
        cout<<"Yaw: " << y_yaw << "Pitch: " << x_pitch <<endl;
        cout << "-----------------------------------------------" << endl;
        y_yaw = yaw_temp; x_pitch = pitch_temp;
    }
    if(showPinHole)
    {
        PinHole_solver();
        cout<<"PinHole Solver:"<<endl;
        cout<<"Yaw: " << y_yaw << "Pitch: " << x_pitch <<endl;
        cout << "-----------------------------------------------" << endl;
        y_yaw = yaw_temp; x_pitch = pitch_temp;
    }
    if(showCompensation)
    {
        solveAngles();
        float raw_pitch;
        raw_pitch = x_pitch;
        compensateOffset();
        cout<<"Gun-Camera Offset:"<<x_pitch - raw_pitch<<endl;
        cout<<"Yaw: " << y_yaw << "Pitch: " << x_pitch <<endl;
        cout << "-----------------------------------------------" << endl;

        solveAngles();
        compensateGravity();
        cout<<"Gravity:"<<x_pitch - raw_pitch<<endl;
        cout<<"Yaw: " << y_yaw << "Pitch: " << x_pitch <<endl;
        cout << "-----------------------------------------------" << endl;

        solveAngles();
        compensateAngle();
        cout<<"Compensation:"<<endl;
        cout<<"Pitch Compensation:"<<x_pitch - raw_pitch<<endl;
        cout<<"Yaw: " << y_yaw << "Pitch: " << x_pitch <<endl;
        cout << "-----------------------------------------------" << endl;
        y_yaw = yaw_temp; x_pitch = pitch_temp;
    }
    if(showCameraParams)
    {
        cout<<"CANERA_MATRIX:"<<endl;
        cout<<CAMERA_MATRIX<<endl;
        cout << "-----------------------------------------------" << endl;
        cout<<"DISTORTION_COEFF"<<endl;
        cout<<DISTORTION_COEFF<<endl;
        cout << "-----------------------------------------------" << endl;
    }
}
