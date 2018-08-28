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
	bmpfile.open(filename.c_str(), ios::binary);//����ͼƬ���ֽ��� 

	if (!bmpfile) // can't open file filename
	{
		return -1;
	}

	// release previous data if any;
	releaseData();

	bmpfile.read((char*)&bmfHeader_bfType, sizeof(bmfHeader_bfType));//��ȡͼƬ������ 

	if (bmfHeader_bfType != 0x4D42) //not a bmp file
	{
		return -1;
	}

	// read file header ( except bfType we have read )
	bmpfile.read((char*)&bmfHeader, sizeof(BitMapFileHeader));//��ȡ�ļ���Ϣͷ�����в�����ͼ���Type���� 

															  // read info header
															  // attention: according to bmp specification, size of bmfHeader should be 40 bytes
	bmpfile.read((char*)&bmiHeader, sizeof(BitMapInfoHeader));//��ȡͼ����Ϣͷ 

	imageSize = bmfHeader.bfSize - bmfHeader.bfOffBits;//����ͼ����Ĵ�С ��������ͼ����Ϣͷ 
	int i = 0;

	paletteSize = bmfHeader.bfOffBits - sizeof(BitMapFileHeader) - sizeof(BitMapInfoHeader) - 2;//��ɫ��Ĵ�С 

	pPalette = new RGBQuad[paletteSize / sizeof(RGBQuad)];//������һ����ɫ��Ŀռ䣬��һ��RGBQuad������ 

	bmpfile.read((char*)pPalette, paletteSize);//��ȡ��ɫ��

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
//�������� 
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
//��ײ���
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
//�ֲ�ƽ����
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
//��������ƽ����
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
//����Ӧ��ֵ�˲���Ĭ��minSize = 3��minSize��maxSize��Ϊ����
void BitMap::adaptive_median_filtering(int minSize, int maxSize)
{
	//������������
	int Z_min = 0, Z_max = 0, Z_med = 0, Z_xy = 0;
	//ͼ�������24λ�ģ�Ҳ������8λ��
	int sign = 1;
	int width = bmiHeader.biWidth;
	int height = bmiHeader.biHeight;
	if (bmiHeader.biBitCount == 24)
	{
		width = (width * 3 / 4 + 1) * 4;
		sign = 3;
	}
	//1. ����һ��ԭͼ�Ŀ���
	unsigned char* pData_copy = new unsigned char[imageSize + 1];
	memcpy(pData_copy, (unsigned char*)pData, imageSize + 1);
	//2. �˲�����
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int current_window = minSize;
			//����Ӧ����
			while (current_window <= maxSize)
			{
				vector<unsigned char> those_pixel;
				//��������
				for (int a = -1 * (current_window / 2); a <= (current_window / 2); a++)
				{
					for (int b = -1 * (current_window / 2) * sign; b <= (current_window / 2) * sign; b = b + sign)
					{
						//��������˱߽磬�򲻿���
						if (i + a < 0 || i + a >= height || j + b < 0 || j + b >= width)continue;
						//�����صĻҶ�ֵ����һ��vector����
						those_pixel.push_back(pData_copy[(i + a) * width + (j + b)]);
					}
				}
				sort(those_pixel.begin(), those_pixel.end());
				int Z_min = those_pixel.front();//Sxy�е���С�Ҷ�ֵ
				int Z_max = those_pixel.back();//Sxy�е����Ҷ�ֵ
				int Z_med = those_pixel[(those_pixel.size() - 1) / 2];//Sxy�еĻҶ�ֵ����ֵ
				int Z_xy = pData_copy[i * width + j];//����(x,y)���ĻҶ�ֵ
				int A1 = Z_med - Z_min;
				int A2 = Z_med - Z_max;
				if (A1 > 0 && A2 < 0)//��һ��������⣬Z_med��������
				{
					int B1 = Z_xy - Z_min;
					int B2 = Z_xy - Z_max;
					if (B1 > 0 && B2 < 0)//�ڶ���������⣬Z_medȷʵ��������
						pData[i * width + j] = Z_xy;
					else
						pData[i * width + j] = Z_med;
					break;
				}
				those_pixel.clear();
				current_window += 2;
			}
			//�ڵ�һ������������ж�Ϊ��������������������󴰿ڳߴ磬��Ȼ�ж�Ϊ����������Z_med���浱ǰ��ֵ
			if (current_window > maxSize)pData[i * width + j] = Z_med;
		}
	}
}
/*
���裺
1. ��ɫͼ��ת��Ϊ�Ҷ�ͼ��
2. ��ͼ����и�˹ģ��
3. ����ͼ���ݶȣ������ݶȼ���ͼ���Ե��ֵ��Ƕ�
4. ������ź�ѹ�ƴ�����Եϸ����
5. ˫��ֵ��Ե���Ӵ���
*/
//������Canny��Ե��ȡ����غ����������Ĭ��8λ��ͼ���ǻҶ�ͼ��
void BitMap::Canny_Edge_Detector()
{
	//���ǵ����룬����8λͼ��Ĵ�С
	int size_8 = bmiHeader.biHeight * ((int)((bmiHeader.biWidth + 3) / 4) * 4);
///1. ��ɫͼ��ת��Ϊ�Ҷ�ͼ��
	//�����Ҫ��һ��24λ�����ɫת����һ��8λ�ĻҶ�ͼ����Ҫ����һ�ݿռ䣬��ͼ����ռ��С��24λͼ�������֮һ
	unsigned char * pDataGray = new unsigned char[size_8];
	ConvertRGB2GRAY(pData, pDataGray);
///2. ��ͼ����и�˹ģ��
	//�������˴�С
	int size = 5;
	//���������
	double ** gaus = new double *[size];
	//��̬���ɾ���
	for (int i = 0; i < size; i++)gaus[i] = new double[size];
	//���� 5 * 5 ��С��˹����ˣ�Sigma=1
	GetGaussianKernel(gaus, 5, 1);
	//�ٴ�����һ��8λ�Ҷ�ͼ��Ŀռ䣬�洢��˹ģ�����ͼƬ
	unsigned char * imageGaussian = new unsigned char[size_8];
	//��˹�˲�
	GaussianFilter(pDataGray, imageGaussian, gaus, 5);
///3. ����ͼ���ݶȣ������ݶȼ���ͼ���Ե��ֵ��Ƕ�
	unsigned char * SobelX = new unsigned char[size_8];
	unsigned char * SobelY = new unsigned char[size_8];
	//�����ݶȷ��������
	double * pointDirection = new double[((bmiHeader.biWidth + 3) / 4 * 4 - 1) * (bmiHeader.biHeight - 1)];
	//����X��Y�����ݶȺͷ����
	SobelGradDirction(imageGaussian, SobelX, SobelY, pointDirection);
	//����X��Y�����ݶ��ںϷ�ֵ
	unsigned char * SobelXY = new unsigned char[size_8];
	SobelAmplitude(SobelX, SobelY, SobelXY);
///4. ������ź�ѹ�ƴ�����Եϸ����
	//�ֲ��Ǽ���ֵ����SobelXY
	unsigned char * imageLocalMax = new unsigned char[size_8];
	LocalMaxValue(SobelXY, imageLocalMax, pointDirection);
///5. ˫��ֵ��Ե���Ӵ���
	DoubleThreshold(imageLocalMax, 60, 90);
	//ʹ��һ���µ�����������imageLocalMax��Ŀ���Ƿ��������й����Ƿ���ȷ
	unsigned char * FinalImage = imageLocalMax;
	//˫��ֵ�м��������Ӵ���
	DoubleThresholdLink(FinalImage, 60, 90);
	//����ͼ�����ͼ��ԭ����24λ�ģ�������Ϣ����Ϊ8λ
///6.����ͼ���ļ�ͷ��ͼ��ͷ�����Ϣ
	if (bmiHeader.biBitCount == 24)
	{
		imageSize = size_8;
		bmfHeader.bfOffBits = 1078;
		bmfHeader.bfSize = bmfHeader.bfOffBits + imageSize;
		bmiHeader.biBitCount = 8;
		bmiHeader.biSizeImage = imageSize;
		bmiHeader.biXPelsPerMeter = 0;
		bmiHeader.biYPelsPerMeter = 0;
		//������һ����ɫ��Ŀռ䣬��һ��RGBQuad������ 
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
	///������ź�ѹ�ƴ�����Եϸ����
	///˫��ֵ��Ե���Ӵ���
	///��ֵ��ͼ��������
}
//24λ��RGB���ɫͼ��ĳ�8λ�Ҷ�ͼ�񣬵�һ������Ϊԭͼ�񣬵ڶ�������Ϊת�����ͼ��
void BitMap::ConvertRGB2GRAY(unsigned char* pData, unsigned char* pDataGray)
{

	//���ͼ������8λͼ������ʲô�����ˣ��������Ĭ��8λͼ����ǻҶ�ͼ��
	if (bmiHeader.biBitCount == 8)return;
	//�������
	int width = bmiHeader.biWidth;
	int width1 = (width + 3) / 4 * 4;
	int width2 = (width * 3 + 3) / 4 * 4;
	//����ת����gray  =  R * 0.299 + G * 0.587 + B * 0.114
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
//��˹��������ɺ�����gaus��ָ����N��double���������ָ�룬size�Ǹ�˹����˵ĳߴ磬sigma�Ǿ���˵ı�׼��
void BitMap::GetGaussianKernel(double ** gaus, const int size, const double sigma)
{
	//Բ���ʦи�ֵ
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
//��˹�˲�
//��˹ģ�����˲�������pDataGray�Ǵ��˲�ԭʼͼ��imageGaussian���˲������ͼ��gaus��һ��ָ����N��double���������ָ�룬size���˲��˵ĳߴ�
void BitMap::GaussianFilter(unsigned char * pDataGray, unsigned char * imageGaussian, double ** gaus, int size)
{
	//����ά�ĸ�˹�������Ϊһά
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
	//�������
	int width = bmiHeader.biWidth;
	width = (width + 3) / 4 * 4;
	//�˲�������ÿһ�����ص㶼��������sizeƽ���Σ��Ӷ�����˾������
	for (int i = 0; i < bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int k = 0;
			for (int a = -size / 2; a <= size / 2; a++)
			{
				for (int b = -size / 2; b <= size / 2; b++)
				{
					//���´�������˲���ͼ��߽紦��Ϊ�����߽��ֵ��ֵΪ�߽�ֵ
					int row = i + a;
					int col = j + b;
					row = row < 0 ? 0 : row;
					row = row >= bmiHeader.biHeight ? bmiHeader.biHeight - 1 : row;
					col = col < 0 ? 0 : col;
					col = col >= width ? width - 1 : col;
					//�����
					imageGaussian[i * width + j] += gausArray[k] * pDataGray[row * width + col];
					k++;
				}
			}
		}
	}
}
//Sobel���Ӽ���X��Y�����ݶȺ��ݶȷ����
//imageGaussianԭʼ������˹ģ���ĻҶ�ͼ��SobelX��X�����ݶ�ͼ��SobelY��Y�����ݶ�ͼ��pointDrection���ݶȷ��������ָ��
void BitMap::SobelGradDirction(unsigned char * imageGaussian, unsigned char * SobelX, unsigned char * SobelY, double * pointDirection)
{
	//�������
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	//��ʼ���ݶȷ��������
	for (int i = 0; i < (bmiHeader.biHeight - 1) * (width - 1); i++)pointDirection[i] = 0;
	int k = 0;
	for (int i = 1; i < (bmiHeader.biHeight - 1); i++)
	{
		for (int j = 1; j < (width - 1); j++)
		{
			//ͨ��ָ�����ͼ����ÿһ������ 
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
			//������ת��Ϊ��
			pointDirection[k] = atan(gradY / gradX) * 57.3;
			if (pointDirection[k] <= 0)pointDirection[k] += 180;
			k++;
		}
	}
}
//����X��Y�����ݶ��ںϷ�ֵ��SobelX��SobelY�ֱ���XY�����ݶ�ͼ��SobelXY��XY�ݶ��ںϷ�ֵ
void BitMap::SobelAmplitude(unsigned char * SobelX, unsigned char * SobelY, unsigned char * SobelXY)
{
	//�������
	int width = (bmiHeader.biWidth + 3) / 4 * 4;
	for (int i = 0; i< bmiHeader.biHeight; i++)
		for (int j = 0; j < width; j++)
			SobelXY[i * width + j] = (int)sqrt(pow(SobelX[i * width + j], 2) + pow(SobelY[i * width + j], 2));
}
//�ֲ�����ֵ���ƣ�SobelXY�����Sobel�ݶ�ͼ��imageLocalMax������ľֲ�����ֵ����ͼ��pointDrection��ͼ����ÿ������ݶȷ�������ָ��
void BitMap::LocalMaxValue(unsigned char * SobelXY, unsigned char * imageLocalMax, double * pointDirection)
{
	//�������
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

//˫��ֵ����imageLocalMax�ǷǼ���ֵ����֮���ͼ��lowThreshold�ǵ���ֵ��highThreshold�Ǹ���ֵ  
void BitMap::DoubleThreshold(unsigned char * imageLocalMax, double lowThreshold, double highThreshold)
{
	//�������
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
//˫��ֵ�м��������Ӵ���FinalImage�ǽ���˫��ֵ����֮���ͼ��lowThreshold�ǵ���ֵ��highThreshold�Ǹ���ֵ  
void BitMap::DoubleThresholdLink(unsigned char * FinalImage, double lowThreshold, double highThreshold)
{
	//�������
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
