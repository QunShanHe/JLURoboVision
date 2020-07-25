# 吉林大学TARS-GO战队视觉代码JLURoboVision
---
## 介绍  
本代码是吉林大学TARS-GO战队Robomaster2020赛季步兵视觉算法，主要模块分为**装甲板识别**、**大风车能量机关识别**、**角度解算**、**相机驱动**及**串口/CAN通信**。  

---
## 目录
* [1. 功能介绍](#1功能介绍)
* [2. 效果展示](#2效果展示)
* [3. 依赖环境](#3依赖环境)
* [4. 整体框架](#4整体框架)
* [5. 实现方案](#5实现方案)
* [6. 通讯协议](#6通信协议)
* [7. 配置与调试](#7配置与调试)
* [8. 总结展望](#8总结展望)
---
## 1.功能介绍
|模块     |功能     |
| ------- | ------ |
|装甲板识别| 检测敌方机器人装甲板位置信息并识别其数字 |
|大风车能量机关识别| 检测待激活大风车扇叶目标位置信息 |
|角度解算| 根据上述位置信息解算目标相对枪管的yaw、pitch角度及距离 |
|相机驱动| 大恒相机SDK封装，实现相机参数控制及图像采集 |
|串口/CAN通信| 与下位机通信，传输机器人姿态信息及操作手反馈视觉的控制信息 |
---
## 2.效果展示
### 装甲板识别
装甲板识别采用基于OpenCV的传统算法实现装甲板位置检测，同时采用SVM实现装甲板数字识别。  
考虑战场实际情况，机器人可打击有效范围在1m~7m之间，在此范围内，本套算法**装甲板识别率达98%**，识别得到装甲板在图像中四个顶点、中心点的坐标信息。  
![图2.1 装甲板实时识别](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/RealtimeArmor.gif "装甲板实时识别")   
在640*480图像分辨率下，**装甲板识别帧率可达340fps左右，引入ROI之后可达420fps**。但考虑到识别帧率对于电控机械延迟的饱和，取消引入ROI操作，以此避免引入ROI之后无法及时探测全局视野情况的问题，加快机器人自瞄响应。  
**640*480**  
![图2.2 装甲板实时识别帧率](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/armor640480.gif "装甲板实时识别")  
**320*240**  
![图2.2 装甲板实时识别帧率](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/armor320240.gif "装甲板实时识别") 
装甲板数字识别采用SVM，通过装甲板位置信息裁剪二值化后的装甲板图像并透射变换，投入训练好的SVM模型中识别，**数字识别准确率可达98%**。  
![图2.3 装甲板数字识别](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/RealtimeArmor.gif "装甲板数字实时识别")  
### 大风车能量机关识别
![图2.4 大风车识别演示](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/windmill.gif "大风车识别演示")  
### 角度解算  
角度解算方面使用了两种解算方法分距离挡位运行。第一档使用P4P算法，第二档使用小孔成像原理的PinHole算法。  
此外还引入了相机-枪口的Y轴距离补偿及重力补偿。  
使用标定板测试，角度解算计算的距离误差在10%以内，角度基本与实际吻合。  
![图2.5 角度解算测试图](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/pos.jpg "角度解算测试图")  
![图2.5 角度解算实时测试](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/angle_solver.gif "角度解算实时测试图")  
 
---
## 3.依赖环境
### 硬件设备
|硬件|型号|参数|
|---|---|---|
|运算平台|Manifold2-G|Tx2|
|相机|大恒相机MER-050|分辨率640*480 曝光值3000~5000|
|镜头|M0814-MP2|焦距8mm 光圈值4|
### 软件设备
|软件类型|型号|
|---|---|
|OS|Ubuntu 16.04/Ubuntu18.04|
|IDE|Qt Creator-4.5.2|
|Library|OpenCV-3.4.0|
|DRIVE|Galaxy SDK|
---
## 4.整体框架
### 文件树  
```
JLURoboVision/
├── AngleSolver
│   └── AngleSolver.h（角度解算模块头文件）
│   ├── AngleSolver.cpp（角度解算模块源文件）
├── Armor
│   ├── Armor.h（装甲板识别模块头文件）
│   ├── LightBar.cpp（灯条类源文件）
│   ├── ArmorBox.cpp（装甲板类源文件）
│   ├── ArmorNumClassifier.cpp（装甲板数字识别类源文件）
│   ├── findLights.cpp（灯条监测相关函数源文件）
│   └── matchArmors.cpp（装甲板匹配相关函数源文件）
│   ├── ArmorDetector.cpp（装甲板识别子类源文件）
├── General
│   └── General.h（公有内容声明头文件）
├── GxCamera
│   ├── GxCamera.h（大恒相机类头文件）
│   ├── GxCamera.cpp（大恒相机类封装源文件）
│   └── include（相机SDK包含文件）
│       ├── DxImageProc.h
│       └── GxIAPI.h
└── Serial
│   ├──  Serial.h（串口头文件）
│   └──Serial.cpp（串口源文件）
├── main.cpp（main函数，程序主入口源文件）
├── 123svm.xml（SVM模型文件）
├── camera_params.xml（相机参数文件）
```
### 整体算法流程图  
![图4.1 自瞄算法流程图](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/Armor.png "自瞄流程图")
---
## 5.实现方案  
### 装甲板识别  
装甲板识别使用基于检测目标特征的OpenCV传统方法，实现检测识别的中心思想是找出图像中所有敌方颜色灯条，并使用找出的灯条一一拟合并筛选装甲板。  
主要步骤分为：**图像预处理**、**灯条检测**、**装甲板匹配**、**装甲板数字识别**及最终的**目标装甲板选择**。  
1. **图像预处理**  
为检测红/蓝灯条，需要进行颜色提取。颜色提取基本思路有BGR、HSV、通道相减法。  
然而，前两种方法由于需要遍历所有像素点，耗时较长，因此我们选择了**通道相减法**进行颜色提取。  
其原理是在**低曝光**（3000~5000）情况下，蓝色灯条区域的B通道值要远高于R通道值，使用B通道减去R通道再二值化，能提取出蓝色灯条区域，反之亦然。  
此外，我们还对颜色提取二值图进行一次掩膜大小3*3，形状MORPH_ELLIPSE的膨胀操作，用于图像降噪及灯条区域的闭合。  
![图5.1 颜色提取二值图](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/src_binary.jpg "预处理后二值图")  
2. **灯条检测**  
灯条检测主要是先对预处理后的二值图找轮廓（findContours），  
然后对初筛（面积）后的轮廓进行拟合椭圆（fitEllipse），  
使用得到的旋转矩形（RotatedRect）构造灯条实例（LightBar），  
在筛除偏移角过大的灯条后依据灯条中心从左往右排序。  
![图5.2 灯条识别图](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/Light_Monitor.jpg "灯条识别效果识别图")   
3. **装甲板匹配**  
分析装甲板特征可知，装甲板由两个长度相等互相平行的侧面灯条构成，  
因此我们对检测到的灯条进行两两匹配，  
通过判断两个灯条之间的位置信息：角度差大小、错位角大小、灯条长度差比率和X,Y方向投影差比率，  
从而分辨该装甲板是否为合适的装甲板（isSuitableArmor），  
然后将所有判断为合适的装甲板放入预选装甲板数组向量中。  
同时，为了消除“游离灯条”导致的误装甲板，我们针对此种情况编写了eraseErrorRepeatArmor函数，专门用于检测并删除错误装甲板。  
```
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
```
4. **装甲板数字识别**  
匹配好装甲板后，利用装甲板的顶点在原图的二值图（原图的灰度二值图）中剪切装甲板图，  
使用透射变换将装甲板图变换为SVM模型所需的Size，随后投入SVM识别装甲板数字。  
![图5.3 装甲板数字识别图](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/NumClassifier.png "装甲板数字识别效果图")
5. **目标装甲板选取**  
对上述各项装甲板信息（顶点中心点坐标与枪口锚点距离、面积大小、装甲板数字及其是否与操作手设定匹配）进行加权求和，  
从而获取最佳打击装甲板作为最终的目标装甲板。  
![图5.4 装甲板识别效果图](https://gitee.com/mountain123/JLURoboVision/raw/master/Assets/Armor_Monitor.png "装甲板识别效果图")  

---
### 大风车识别

### 角度解算  
角度解算部分使用了两种模型解算枪管直指向目标装甲板所需旋转的yaw和pitch角。  
第一个是**P4P解算**，第二个是**PinHole解算**。  
首先回顾一下相机成像原理，其成像原理公式如下：  
$$ s \begin{bmatrix} u \\ v \\ 1 \end{bmatrix} = \begin{bmatrix} f_x & 0 & c_x \\ 0 & f_y & c_y \\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} r_{11} & r_{12} & r_{13} & t_x \\ r_{21} & r_{22} & r_{23} & t_y \\ r_{31} & r_{32} & r_{33} & t_z \end{bmatrix} \begin{bmatrix} X \\ Y \\ Z \\ 1 \end{bmatrix}$$  
1. P4P解算原理  
由上述相机成像原理可得相机-物点的平移矩阵为：
$$ tVec = \begin{bmatrix} t_x \\ t_y \\ t_z \end{bmatrix} $$  
转角计算公式如下：  
$$ \tan pitch = \frac{t_y}{\sqrt{{t_y}^2 + {t_z}^2}} $$ 
$$ \tan yaw = \frac{t_x}{t_z} $$

2. 小孔成像原理
像素点与物理世界坐标系的关系：  
$$ x_{screen} = f_x(\frac{X}{Z}) + c_x $$
$$ y_{screen} = f_y(\frac{Y}{Z}) + c_y $$  
则转角计算公式如下：  
$$ \tan pitch = \frac{X}{Z} = \frac{x_{screen} - c_x}{f_x} $$
$$ \tan yaw = \frac{Y}{Z} = \frac{y_{screen} - c_y}{f_y} $$

---
## 6.通讯协议  
上下板之间的通信逻辑，主要由我们自定的通信协议体现：  
协议共有16个字节，包括帧头占用的1字节，校验位需要的1字节，数据位的12个字节，以及两个字节的标志位。可以满足上位机与主控板之间的通信需求，且尽量精简了数据包体量以提高传输速度。  
|Byte0|Byte1|Byte2|Byte3|Byte4|Byte5|Byte6|Byte7|
|:--|:--|:--|:--|:--|:--|:--|:--|
|0xAA|CRC_8|Yaw_data|Yaw_data|Yaw_data|Yaw_data|Pitch_data|Pitch_data|
|Byte8|Byte9|Byte10|Byte11|Byte12|Byte13|Byte14|Byte15|
|Pitch_data|Pitch_data|Dist_data|Dist_data|Dist_data|Dist_data|Targt)Num|Fire_cmd|
> * 0xAA -帧头 
> * Yaw_data : 8 bit char - 接收视觉解算出来的云台 yaw 值
> * Pitch_data : 8 bit char - 接收视觉解算出来的云台 pitch 值
> * （改为传360坐标系，x10保留一位小数，距离直接传数值，同样x10精度mm，帧头0XAA）
> * Dist_data : 8 bit char - 接收视觉解算目标到相机的距离值
> * Target_num : 8 bit char - 优先目标数字-前三位（数字0-8，没有6）（stm32 -> PC）
> * 模式选择 - 后五位（0 不处理，1-8留作模式选择， stm32 -> PC，1为自瞄，2为大风车）
> * Fire-cmd是否开火：8 bit char - 在视觉判定枪口（摄像头）对准目标在误差范围内时发送——0 为不开火，1 为开火
---
## 7.配置与调试
### 运行平台搭建  
1. Qt（及QtCreator）安装
2. OpenCV库安装及配置
3. 大恒相机驱动安装及配置

### 代码调试
1. 使用QtCreator打开JLURoboVision.pro
2. 检查camera_params.xml 及123svm.xml路径
3. 编译运行

### 单独模块调试  
可参考下列示例代码：  
[JLUVision_Demos](https://gitee.com/mountain123/JLUVision_Demos)各示例程序代码库  
[Armor_Demo](https://gitee.com/mountain123/JLUVision_Demos/tree/master/Armor_Demo)为装甲板识别模块演示程序，可在Linux(.pro)/Windows(.sln)运行。  
[AngleSolver_Armor_GxCamera](https://gitee.com/mountain123/JLUVision_Demos/tree/master/Anglesolver_Armor_GxCamera_Demo)为大恒相机采图+装甲板+角度解算演示程序，需要连接大恒相机在Linux下运行。  

---
## 8.总结展望
### zongjie
1. this code realize the most function of 
2. we have these features
3. equiped with many debugging tools
4. accelerate image processing
5. Armor score with operator setting target number
6. shibie lv  zhenlv budi

#### 码云特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  码云官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解码云上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是码云最有价值开源项目，是码云综合评定出的优秀开源项目
5.  码云官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  码云封面人物是一档用来展示码云会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
