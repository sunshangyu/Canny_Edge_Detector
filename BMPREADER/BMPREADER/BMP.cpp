/*
File:  BMP.cpp
Disc:  BMP file all function
Author: sunshangyu
Modify:
*/
#include "BMP.h"
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int BitMap::loadBmpFile(const string& filename)
{
	//ifstream bmpfile(filename.c_str());
	ifstream bmpfile;
	bmpfile.open(filename.c_str(), ios::binary);//读入图片的字节流 

	if (!bmpfile) // can't open file filename
	{
		return -1;
	}

	// release previous data if any;
	releaseData();

	bmpfile.read((char*)&bmfHeader_bfType, sizeof(bmfHeader_bfType));//读取图片的类型 

	if (bmfHeader_bfType != 0x4D42) //not a bmp file
	{
		return -1;
	}

	// read file header ( except bfType we have read )
	bmpfile.read((char*)&bmfHeader, sizeof(BitMapFileHeader));//读取文件信息头，其中不包括图像的Type类型 

															  // read info header
															  // attention: according to bmp specification, size of bmfHeader should be 40 bytes
	bmpfile.read((char*)&bmiHeader, sizeof(BitMapInfoHeader));//读取图像信息头 

	imageSize = bmfHeader.bfSize - bmfHeader.bfOffBits;//计算图像本身的大小 ，不包括图像信息头 
	int i = 0;

	paletteSize = bmfHeader.bfOffBits - sizeof(BitMapFileHeader) - sizeof(BitMapInfoHeader) - 2;//调色板的大小 

	pPalette = new RGBQuad[paletteSize / sizeof(RGBQuad)];//申请了一个调色板的空间，是一个RGBQuad的数组 

	bmpfile.read((char*)pPalette, paletteSize);//读取调色板

	switch (bmiHeader.biBitCount)
	{
	case 1:  // monoColor Image
		break;

	case 4:  // 16 Color Image
		break;

	case 8:  // 256 Color Image
		pData = new unsigned char[imageSize];
		bmpfile.read((char*)pData, imageSize);
		dataType = RGBA;
		break;

	case 16: // 16 Bits, 64k Colors Image
		break;

	case 24: // True Color Image
		pData = new unsigned char[imageSize];
		bmpfile.read((char*)pData, imageSize);
		dataType = RGB;
		break;
	default:
		return -1; // Not support now
		break;
	}
	return 0;
}
//调整亮度 
void BitMap::change_light_level(double level)
{
	if (bmiHeader.biBitCount == 24)
		for (int i = 0; i < imageSize; i++)
			pData[i] = pData[i] < (255 / level) ? (pData[i] * level) : 255;
	if (bmiHeader.biBitCount == 8)
		for (int i = 0; i < paletteSize / sizeof(RGBQuad); i++)
		{
			pPalette[i].rgbBlue = pPalette[i].rgbBlue < (255 / level) ? (pPalette[i].rgbBlue * level) : 255;
			pPalette[i].rgbGreen = pPalette[i].rgbGreen < (255 / level) ? (pPalette[i].rgbGreen * level) : 255;
			pPalette[i].rgbRed = pPalette[i].rgbRed < (255 / level) ? (pPalette[i].rgbRed * level) : 255;
		}
}
//变白操作
void BitMap::dowhite()
{
	int width;
	if (bmiHeader.biBitCount == 24)
		width = bmiHeader.biWidth * 3;
	if (bmiHeader.biBitCount == 8)
		width = bmiHeader.biWidth;
	int height = bmiHeader.biHeight;
	for (int i = 0; i < height / 2; i++)
		for (int j = width / 2; j < width; j++)
			pData[i * width + j] = 255;
}
int BitMap::saveBmpFile(const string& filename)
{
	ofstream out(filename, ios::out | ios::app | ios::binary);
	out.write((char*)&bmfHeader_bfType, sizeof(bmfHeader_bfType));
	out.write((char*)&bmfHeader, sizeof(BitMapFileHeader));
	out.write((char*)&bmiHeader, sizeof(BitMapInfoHeader));
	out.write((char*)pPalette, paletteSize);
	out.write((char*)pData, imageSize);
	out.close();
	return 0;
}
//局部平滑法
void BitMap::local_smoothing_method(int level)
{
	int sign = 1;
	int width = bmiHeader.biWidth;
	int height = bmiHeader.biHeight;
	if (bmiHeader.biBitCount == 24)
	{
		width = (width * 3 / 4 + 1) * 4;
		sign = 3;
	}
	unsigned char* pData_copy = new unsigned char[imageSize + 1];
	memcpy(pData_copy, (unsigned char*)pData, imageSize + 1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int count_0 = 0;
			int count_all = 0;
			for (int a = -1 * (level / 2); a <= (level / 2); a++)
			{
				for (int b = -1 * (level / 2) * sign; b <= (level / 2) * sign; b = b + sign)
				{
					if (i + a < 0 || i + a >= height || j + b < 0 || j + b >= width)count_0++;
					else count_all += pData_copy[(i + a) * width + (j + b)];
				}
			}
			pData[i * width + j] = count_all / (level * level - count_0);
		}
	}
}
//超限像素平滑法
void BitMap::unlimited_pixel_smoothing_method(int level, int T)
{
	int sign = 1;
	int width = bmiHeader.biWidth;
	int height = bmiHeader.biHeight;
	if (bmiHeader.biBitCount == 24)
	{
		width = (width * 3 / 4 + 1) * 4;
		sign = 3;
	}
	unsigned char* pData_copy = new unsigned char[imageSize + 1];
	memcpy(pData_copy, (unsigned char*)pData, imageSize + 1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int count_0 = 0;
			int count_all = 0;
			for (int a = -1 * (level / 2); a <= (level / 2); a++)
			{
				for (int b = -1 * (level / 2) * sign; b <= (level / 2) * sign; b = b + sign)
				{
					if (i + a < 0 || i + a >= height || j + b < 0 || j + b >= width)count_0++;
					else count_all += pData_copy[(i + a) * width + (j + b)];
				}
			}
			int tee = count_all / (level * level - count_0);
			int ab = abs(tee - pData[i * width + j]);
			if (ab > T)pData[i * width + j] = tee;
		}
	}
}
void BitMap::format()
{
	dataType = FORMATTED_RGB;
}
//自适应中值滤波，默认minSize = 3，minSize和maxSize均为奇数
void BitMap::adaptive_median_filtering(int minSize, int maxSize)
{
	//声明几个变量
	int Z_min = 0, Z_max = 0, Z_med = 0, Z_xy = 0;
	//图像可能是24位的，也可能是8位的
	int sign = 1;
	int width = bmiHeader.biWidth;
	int height = bmiHeader.biHeight;
	if (bmiHeader.biBitCount == 24)
	{
		width = (width * 3 / 4 + 1) * 4;
		sign = 3;
	}
	//1. 进行一份原图的拷贝
	unsigned char* pData_copy = new unsigned char[imageSize + 1];
	memcpy(pData_copy, (unsigned char*)pData, imageSize + 1);
	//2. 滤波处理
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int current_window = minSize;
			//自适应过程
			while (current_window <= maxSize)
			{
				vector<unsigned char> those_pixel;
				//遍历领域
				for (int a = -1 * (current_window / 2); a <= (current_window / 2); a++)
				{
					for (int b = -1 * (current_window / 2) * sign; b <= (current_window / 2) * sign; b = b + sign)
					{
						//如果超过了边界，则不考虑
						if (i + a < 0 || i + a >= height || j + b < 0 || j + b >= width)continue;
						//将像素的灰度值填入一个vector容器
						those_pixel.push_back(pData_copy[(i + a) * width + (j + b)]);
					}
				}
				sort(those_pixel.begin(), those_pixel.end());
				int Z_min = those_pixel.front();//Sxy中的最小灰度值
				int Z_max = those_pixel.back();//Sxy中的最大灰度值
				int Z_med = those_pixel[(those_pixel.size() - 1) / 2];//Sxy中的灰度值的中值
				int Z_xy = pData_copy[i * width + j];//坐标(x,y)处的灰度值
				int A1 = Z_med - Z_min;
				int A2 = Z_med - Z_max;
				if (A1 > 0 && A2 < 0)//第一层噪声检测，Z_med不是噪声
				{
					int B1 = Z_xy - Z_min;
					int B2 = Z_xy - Z_max;
					if (B1 > 0 && B2 < 0)//第二层噪声监测，Z_med确实不是噪声
						pData[i * width + j] = Z_xy;
					else
						pData[i * width + j] = Z_med;
					break;
				}
				those_pixel.clear();
				current_window += 2;
			}
			//在第一层噪声监测中判断为噪声，并且无论如何增大窗口尺寸，仍然判断为噪声，则用Z_med代替当前的值
			if (current_window > maxSize)pData[i * width + j] = Z_med;
		}
	}
}
/*
步骤：
1. 彩色图像转换为灰度图像
2. 对图像进行高斯模糊
3. 计算图像梯度，根据梯度计算图像边缘幅值与角度
4. 非最大信号压制处理（边缘细化）
5. 双阈值边缘连接处理
*/
//以下是Canny边缘提取的相关函数，这里就默认8位的图像是灰度图像
void BitMap::Canny_Edge_Detector()
{
	//考虑到对齐，这是8位图像的大小
	int size_8 = bmiHeader.biHeight * ((int)((bmiHeader.biWidth + 3) / 4) * 4);
///1. 彩色图像转换为灰度图像
	//如果需要将一个24位的真彩色转换成一个8位的灰度图像，需要申请一份空间，其图像本身空间大小是24位图像的三分之一
	unsigned char * pDataGray = new unsigned char[size_8];
	ConvertRGB2GRAY(pData, pDataGray);
///2. 对图像进行高斯模糊
	//定义卷积核大小
	int size = 5;
	//卷积核数组
	double ** gaus = new double *[size];
	//动态生成矩阵
	for (int i = 0; i < size; i++)gaus[i] = new double[size];
	//生成 5 * 5 大小高斯卷积核，Sigma=1
	GetGaussianKernel(gaus, 5, 1);
	//再次申请一份8位灰度图像的空间，存储高斯模糊后的图片
	unsigned char * imageGaussian = new unsigned char[size_8];
	//高斯滤波
	GaussianFilter(pDataGray, imageGaussian, gaus, 5);
///3. 计算图像梯度，根据梯度计算图像边缘幅值与角度
	unsigned char * SobelX = new unsigned char[size_8];
	unsigned char * SobelY = new unsigned char[size_8];
	//定义梯度方向角数组
	double * pointDirection = new double[((bmiHeader.biWidth + 3) / 4 * 4 - 1) * (bmiHeader.biHeight - 1)];
	//计算X、Y方向梯度和方向角
	SobelGradDirction(imageGaussian, SobelX, SobelY, pointDirection);
	//计算X、Y方向梯度融合幅值
	unsigned char * SobelXY = new unsigned char[size_8];
	SobelAmplitude(SobelX, SobelY, SobelXY);
///4. 非最大信号压制处理（边缘细化）
	//局部非极大值抑制SobelXY
	unsigned char * imageLocalMax = new unsigned char[size_8];
	LocalMaxValue(SobelXY, imageLocalMax, pointDirection);
///5. 双阈值边缘连接处理
	DoubleThreshold(imageLocalMax, 60, 90);
	//使用一个新的名称来代替imageLocalMax，目的是方便检测运行过程是否正确
	unsigned char * FinalImage = imageLocalMax;
	//双阈值中间像素连接处理
	DoubleThresholdLink(FinalImage, 60, 90);
	//调整图像，如果图像原本是24位的，则将其信息调整为8位
///6.调整图像文件头和图像头相关信息
	if (bmiHeader.biBitCount == 24)
	{
		imageSize = size_8;
		bmfHeader.bfOffBits = 1078;
		bmfHeader.bfSize = bmfHeader.bfOffBits + imageSize;
		bmiHeader.biBitCount = 8;
		bmiHeader.biSizeImage = imageSize;
		bmiHeader.biXPelsPerMeter = 0;
		bmiHeader.biYPelsPerMeter = 0;
		//申请了一个调色板的空间，是一个RGBQuad的数组 
		struct RGBQuad * temp_color = new struct RGBQuad[256];
		for (int i = 0; i < 256; i++)
		{
			temp_color[i].rgbBlue = temp_color[i].rgbGreen = temp_color[i].rgbRed = i;
			temp_color[i].rgbReserved = 0;
		}
		paletteSize = 1024;
		pPalette = temp_color;

	}
	pData = FinalImage;
	///非最大信号压制处理（边缘细化）
	///双阈值边缘连接处理
	///二值化图像输出结果
}
//24位的RGB真彩色图像改成8位灰度图像，第一个参数为原图像，第二个参数为转换后的图像
void BitMap::ConvertRGB2GRAY(unsigned char* pData, unsigned char* pDataGray)
{

	//如果图像本身是8位图像则不做什么处理了，在这里就默认8位图像就是灰度图像
	if (bmiHeader.biBitCount == 8)return;
	//对齐操作
	int width = bmiHeader.biWidth;
	int width1 = (width + 3) / 4 * 4;
	int width2 = (width * 3 + 3) / 4 * 4;
	//进行转换，gray  =  R * 0.299 + G * 0.587 + B * 0.114
	for (int i = 0; i < bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < width; j++)
		{
			pDataGray[i * width1 + j] = 0.114 * pData[i *  width2 + 3 * j]
				+ 0.587 * pData[i * width2 + 3 * j + 1]
				+ 0.299 * pData[i * width2 + 3 * j + 2];
		}
	}
}
//高斯卷积核生成函数，gaus是指向含有N个double类型数组的指针，size是高斯卷积核的尺寸，sigma是卷积核的标准差
void BitMap::GetGaussianKernel(double ** gaus, const int size, const double sigma)
{
	//圆周率π赋值
	const double PI = 4.0 * atan(1.0);
	int center = size / 2;
	double sum = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gaus[i][j] = (1 / (2 * PI * sigma * sigma)) *
				exp(-((i - center) * (i - center) + (j - center) * (j - center)) / (2 * sigma * sigma));
			sum += gaus[i][j];
		}
	}
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			gaus[i][j] /= sum;
	return;
}
//高斯滤波
//高斯模糊的滤波操作，pDataGray是待滤波原始图像，imageGaussian是滤波后输出图像，gaus是一个指向含有N个double类型数组的指针，size是滤波核的尺寸
void BitMap::GaussianFilter(unsigned char * pDataGray, unsigned char * imageGaussian, double ** gaus, int size)
{
	//将二维的高斯卷积核置为一维
	double gausArray[100];
	for (int i = 0; i < size * size; i++)gausArray[i] = 0;
	int array = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gausArray[array] = gaus[i][j];
			array++;
		}
	}
	//对齐操作
	int width = bmiHeader.biWidth;
	width = (width + 3) / 4 * 4;
	//滤波操作，每一个像素点都被计算了size平方次，从而达成了卷积操作
	for (int i = 0; i < bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int k = 0;
			for (int a = -size / 2; a <= size / 2; a++)
			{
				for (int b = -size / 2; b <= size / 2; b++)
				{
					//以下处理针对滤波后图像边界处理，为超出边界的值赋值为边界值
					int row = i + a;
					int col = j + b;
					row = row < 0 ? 0 : row;
					row = row >= bmiHeader.biHeight ? bmiHeader.biHeight - 1 : row;
					col = col < 0 ? 0 : col;
					col = col >= width ? width - 1 : col;
					//卷积和
					imageGaussian[i * width + j] += gausArray[k] * pDataGray[row * width + col];
					k++;
				}
			}
		}
	}
}
//Sobel算子计算X、Y方向梯度和梯度方向角
//imageGaussian原始经过高斯模糊的灰度图像，SobelX是X方向梯度图像，SobelY是Y方向梯度图像，pointDrection是梯度方向角数组指针
void BitMap::SobelGradDirction(unsigned char * imageGaussian, unsigned char * SobelX, unsigned char * SobelY, double * pointDirection)
{
	//对齐操作
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	//初始化梯度方向角数组
	for (int i = 0; i < (bmiHeader.biHeight - 1) * (width - 1); i++)pointDirection[i] = 0;
	int k = 0;
	for (int i = 1; i < (bmiHeader.biHeight - 1); i++)
	{
		for (int j = 1; j < (width - 1); j++)
		{
			//通过指针遍历图像上每一个像素 
			double gradX = imageGaussian[(i - 1) * width + j + 1]
				+ imageGaussian[i * width + j + 1] * 2
				+ imageGaussian[(i + 1) * width + j + 1]
				- imageGaussian[(i - 1) * width + j - 1]
				- imageGaussian[i * width + j - 1] * 2
				- imageGaussian[(i + 1) * width + j - 1];
			SobelX[i * width + j] = abs(gradX);
			double gradY = imageGaussian[(i + 1) * width + j - 1]
				+ imageGaussian[(i + 1) * width + j] * 2
				+ imageGaussian[(i + 1) * width + j + 1]
				- imageGaussian[(i - 1) * width + j - 1]
				- imageGaussian[(i - 1) * width + j] * 2
				- imageGaussian[(i - 1) * width + j + 1];
			SobelY[i * width + j] = abs(gradY);
			if (gradX == 0) { gradX = 0.00000000000000001; }
			//将弧度转换为度
			pointDirection[k] = atan(gradY / gradX) * 57.3;
			if (pointDirection[k] <= 0)pointDirection[k] += 180;
			k++;
		}
	}
}
//计算X、Y方向梯度融合幅值，SobelX，SobelY分别是XY方向梯度图像，SobelXY是XY梯度融合幅值
void BitMap::SobelAmplitude(unsigned char * SobelX, unsigned char * SobelY, unsigned char * SobelXY)
{
	//对齐操作
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	for (int i = 0; i< bmiHeader.biHeight; i++)
		for (int j = 0; j < width; j++)
			SobelXY[i * width + j] = (int)sqrt(pow(SobelX[i * width + j], 2) + pow(SobelY[i * width + j], 2));
}
//局部极大值抑制，SobelXY输入的Sobel梯度图像，imageLocalMax是输出的局部极大值抑制图像，pointDrection是图像上每个点的梯度方向数组指针
void BitMap::LocalMaxValue(unsigned char * SobelXY, unsigned char * imageLocalMax, double * pointDirection)
{
	//对齐操作
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	int k = 0;
	for (int i = 1; i < bmiHeader.biHeight - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			int value00 = SobelXY[(i - 1) * width + j - 1];
			int value01 = SobelXY[(i - 1) * width + j];
			int value02 = SobelXY[(i - 1) * width + j + 1];
			int value10 = SobelXY[i* width + j - 1];
			int value11 = SobelXY[i * width + j];
			int value12 = SobelXY[i * width + j + 1];
			int value20 = SobelXY[(i + 1) * width + j - 1];
			int value21 = SobelXY[(i + 1) * width + j];
			int value22 = SobelXY[(i + 1) * width + j + 1];
			if (pointDirection[k] == 0)imageLocalMax[i * width + j] = 0;
			else if (pointDirection[k] > 0 && pointDirection[k] <= 45)
			{
				if (value11 <= (value12 + (value02 - value12) * tan(pointDirection[k])) || (value11 <= (value10 + (value20 - value10) * tan(pointDirection[k]))))
					imageLocalMax[i * width + j] = 0;
				else imageLocalMax[i * width + j] = SobelXY[i * width + j];
			}
			else if (pointDirection[k] > 45 && pointDirection[k] <= 90)
			{
				if (value11 <= (value01 + (value02 - value01) / tan(pointDirection[k])) || value11 <= (value21 + (value20 - value21) / tan(pointDirection[k])))
					imageLocalMax[i * width + j] = 0;
				else imageLocalMax[i * width + j] = SobelXY[i * width + j];
			}
			else if (pointDirection[k] > 90 && pointDirection[k] <= 135)
			{
				if (value11 <= (value01 + (value00 - value01) / tan(180 - pointDirection[k])) || value11 <= (value21 + (value22 - value21) / tan(180 - pointDirection[k])))
					imageLocalMax[i * width + j] = 0;
				else imageLocalMax[i * width + j] = SobelXY[i * width + j];
			}
			else if (pointDirection[k] > 135 && pointDirection[k] <= 180)
			{
				if (value11 <= (value10 + (value00 - value10) * tan(180 - pointDirection[k])) || value11 <= (value12 + (value22 - value12) * tan(180 - pointDirection[k])))
					imageLocalMax[i * width + j] = 0;
				else imageLocalMax[i * width + j] = SobelXY[i * width + j];
			}
			k++;
		}
	}
}

//双阈值处理，imageLocalMax是非极大值抑制之后的图像，lowThreshold是低阈值，highThreshold是高阈值  
void BitMap::DoubleThreshold(unsigned char * imageLocalMax, double lowThreshold, double highThreshold)
{
	//对齐操作
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	for (int i = 0; i < bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (imageLocalMax[i * width + j] > highThreshold)
				imageLocalMax[i * width + j] = 255;
			if (imageLocalMax[i * width + j] < lowThreshold)
				imageLocalMax[i * width + j] = 0;
		}
	}
}
//双阈值中间像素连接处理，FinalImage是进行双阈值处理之后的图像，lowThreshold是低阈值，highThreshold是高阈值  
void BitMap::DoubleThresholdLink(unsigned char * FinalImage, double lowThreshold, double highThreshold)
{
	//对齐操作
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	for (int i = 1; i < bmiHeader.biHeight - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			if (FinalImage[i * width + j] > lowThreshold && FinalImage[i * width + j] < 255)
			{
				if (FinalImage[(i - 1) * width + j - 1] == 255 || FinalImage[(i - 1) * width + j] == 255 || FinalImage[(i - 1) * width + j + 1] == 255 ||
					FinalImage[i * width + j - 1] == 255 || FinalImage[i * width + j] == 255 || FinalImage[i * width + j + 1] == 255 ||
					FinalImage[(i + 1) * width + j - 1] == 255 || FinalImage[(i + 1) * width + j] == 255 || FinalImage[(i + 1) * width + j + 1] == 255)
				{
					FinalImage[i * width + j] = 255;
					DoubleThresholdLink(FinalImage, lowThreshold, highThreshold);
				}
				else
					FinalImage[i * width + j] = 0;
			}
		}
	}
}
