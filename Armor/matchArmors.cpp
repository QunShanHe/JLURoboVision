/*
*	@Author: Mountain
*	@Date:	 2020.04.24
*	@Brief:  This cpp file define the function 'int matchArmors()' which is used to match lights into armors
*/

#include"Armor.h"

void eraseErrorRepeatArmor(vector<ArmorBox> & armors);
bool armorCompare(const ArmorBox & a_armor, const ArmorBox & b_armor, const ArmorBox & lastArmor, const int & targetNum);

/**
* @brief: match lights into armors 将识别到的灯条拟合为装甲板
*/
void ArmorDetector::matchArmors(){
	for (int i = 0; i < lights.size() - 1; i++)
	{
		for (int j = i + 1; j < lights.size(); j++) //just ensure every two lights be matched once 从左至右，每个灯条与其他灯条一次匹配判断
		{
				ArmorBox armor = ArmorBox(lights[i], lights[j]); //construct an armor using the matchable lights 利用左右灯条构建装甲板
				if (armor.isSuitableArmor()) //when the armor we constructed just now is a suitable one,set extra information of armor 如果是合适的装甲板，则设置其他装甲板信息
				{
					armor.l_index = i; //set index of left light 左灯条的下标
					armor.r_index = j; //set index of right light 右灯条的下标
					classifier.getArmorImg(armor);// set armor image 装甲板的二值图
					classifier.setArmorNum(armor);//set armor number 装甲板数字
					armors.emplace_back(armor); //push into armors 将匹配好的装甲板push入armors中
				}
		}

		eraseErrorRepeatArmor(armors);//delete the error armor caused by error light 删除游离灯条导致的错误装甲板
	}
	if (armors.empty()) {
		state = ARMOR_NOT_FOUND; //if armors is empty then set state ARMOR_NOT_FOUND 如果armors目前仍为空，则设置状态为ARMOR_NOT_FOUND
		return; //exit function
	} 
	else {
		state = ARMOR_FOUND; //else set state ARMOR_FOUND 如果非空（有装甲板）则设置状态ARMOR_FOUND
		return; //exit function
	}
}

/**
 *@brief: set the privious targetArmor as lastArmor and then choose the most valuable armor from current armors as targetArmor
 *			将上一帧的目标装甲板作为lastArmor选择本帧图像中所有装甲板里面价值最大的装甲板作为目标装甲板
 */
void ArmorDetector::setTargetArmor()
{
	if (state == ARMOR_NOT_FOUND)  targetArmor = ArmorBox(); //not found armr then set a default armor as lastArmor 如果状态为没有找到装甲板，则将lastArmor设置为默认的ArmorBox
	else if (state == ARMOR_FOUND) {
		ArmorBox mva = armors[0]; //mva most valuable armor 最适合打击的装甲板
		for (int i = 1; i < armors.size(); i++) //for circle to select the mva 通过遍历装甲板s获取最佳打击装甲板
		{
			if (armorCompare(armors[i], mva, lastArmor, targetNum)) mva = armors[i];
		}
		targetArmor = mva; //set the mva as the targetArmor of this frame
	}
	lastArmor = targetArmor; //first set the targetArmor(of last frame) as lastArmor 将上一帧的targetArmor设置为本帧的lastArmor
}


/**
 *@brief: detect and delete error armor which is caused by the single lightBar 针对游离灯条导致的错误装甲板进行检测和删除
 */
void eraseErrorRepeatArmor(vector<ArmorBox> & armors)
{
	int length = armors.size();
	vector<ArmorBox>::iterator it = armors.begin();
	for (size_t i = 0; i < length; i++)
		for (size_t j = i + 1; j < length; j++)
		{
			if (armors[i].l_index == armors[j].l_index ||
				armors[i].l_index == armors[j].r_index ||
				armors[i].r_index == armors[j].l_index ||
				armors[i].r_index == armors[j].r_index)
			{
				armors[i].getDeviationAngle() > armors[j].getDeviationAngle() ? armors.erase(it + i) : armors.erase(it + j);
			}
		}
}


/**
 *@brief: get the distance of two points(a and b) 获取两点之间的距离 
 */
float getPointsDistance(const Point2f& a, const Point2f& b) {
	float delta_x = a.x - b.x;
	float delta_y = a.y - b.y;
	//return sqrtf(delta_x * delta_x + delta_y * delta_y);
	return sqrt(delta_x * delta_x + delta_y * delta_y);
}


/**
 *@:brief: accordingt to the armorNum priority to set the armorScore 根据优先级增加装甲板打击度
 */
void setNumScore(const int & armorNum,const int & targetNum, float & armorScore)
{
	if (targetNum == 0)
	{
		if (armorNum == 1) armorScore += 1000;
		else if (armorNum == 2) armorScore += 2000;
		else if (armorNum == 3) armorScore += 3000;
		else if (armorNum == 4) armorScore += 4000;
		else if (armorNum == 5) armorScore += 5000;
		else if (armorNum == 6) armorScore += 6000;
	}
	if (armorNum == targetNum) armorScore += 100000;
}


/**
 *@brief: compare a_armor to b_armor according to their distance to lastArmor(if exit, not a default armor) and their area and armorNum
 *		  比较a_armor装甲板与b_armor装甲板的打击度，判断a_armor是否比b_armor更适合打击（通过装甲板数字是否与目标装甲板数字匹配，装甲板与lastArmor的距离以及装甲板的面积大小判断）
 */
bool armorCompare(const ArmorBox & a_armor, const ArmorBox & b_armor, const ArmorBox & lastArmor, const int & targetNum)
{
	float a_score = 0;  // shooting value of a_armor a_armor的打击度
	float b_score = 0;  //shooting value of b_armor b_armor的打击度
	a_score += a_armor.armorRect.area(); //area value of a a_armor面积得分
	b_score += b_armor.armorRect.area(); //area value of b b_armor面积得分

	//number(robot type) priorty 设置a、b装甲板的分数
	setNumScore(a_armor.armorNum, targetNum, a_score);
	setNumScore(b_armor.armorNum, targetNum, b_score);
	
	if (lastArmor.armorNum != 0) {  //if lastArmor.armorRect is not a default armor means there is a true targetArmor in the last frame 上一帧图像中存在目标装甲板
		float a_distance = getPointsDistance(a_armor.center, lastArmor.center); //distance score to the lastArmor(if exist) 装甲板距离得分，算负分
		float b_distance = getPointsDistance(b_armor.center, lastArmor.center); //distance score to the lastArmor(if exist) 装甲板距离得分，算负分
		a_score -= a_distance * 2;
		b_score -= b_distance * 2;
	}
	return a_score > b_score; //judge whether a is more valuable according their score 根据打击度判断a是否比b更适合打击
}


