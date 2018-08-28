

/*
in vs2005, set the project "C/C++" -> "code generation" -> "struct member alignment" to "1 Byte (Zp1)"
otherwise bfType of BitMapFileHeader would be aligned to 4 bytes, which make the file load wrong
*/
#ifndef BMP_H
#define BMP_H
#include <string>
static int Flag = 0;

using namespace std;

// bfType is moved into BitMap class, as a member : unsigned short bmfHeader_bfType. see declaration of class BitMap
// the reason is that bfType should be size of 2 bytes according to BMP file specification, but in Visual C ++, the 
// default alignment size of struct is 8 bytes. two ways to address this 
//     1: set project setting in VC++ -> C/C++ -> Code Generation -> struct member alignment -> 1 Byte (Zp1)
//     2: in code, add "#pragma pack (2)" before "struct BitMapFileHeader"
// both methods are not clear for teaching consideration so I just drag bfType out of the struct BitMapFileHeader
struct BitMapFileHeader//文件头 
{
	// unsigned short bfType;  moved to class BitMap
	unsigned int   bfSize;//文件的总大小 
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;//图象开始偏移量 
};

struct BitMapInfoHeader//图像信息头 
{
	unsigned int biSize;//信息头大小 
	int biWidth;//图象的宽度 
	int biHeight;//图象的高度 
	unsigned short biPlanes;//图象位平面数目 
	unsigned short biBitCount;//一个像素占据的位数 
	unsigned int biCompression;//压缩类型,暂不考虑 
	unsigned int biSizeImage;//压缩图象大小,暂不考虑 
	int biXPelsPerMeter;//水平分辨率
	int biYPelsPerMeter;//垂直分辨率
	unsigned int biClrUsed;//彩色数目，颜色的数目 
	unsigned int biClrImportant;//“重要的颜色”数目 
};

struct RGBQuad//颜色对照表 
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
};

class BitMap//图象本身 
{
public:
	enum DataType { UNKNOW, RGB, RGBA, FORMATTED_RGB };

	DataType dataType;//十六进制？存储的类型？ 

	unsigned short bmfHeader_bfType;//图象类型 “BM” 
	struct BitMapFileHeader bmfHeader;//图象的文件信息头 
	struct BitMapInfoHeader bmiHeader;//图象的图象信息头 
	struct RGBQuad * pPalette;//指向颜色对照表的指针
	int paletteSize;//调色板的大小
	unsigned char * pData;
	int imageSize;//图象大小 

	BitMap() :dataType(UNKNOW), pPalette(NULL), pData(NULL)
	{
		resetBMFHeader();
		resetBMIHeader();
	}

	~BitMap()
	{
		releasePalette();
		releaseImageData();
	}

	int loadBmpFile(const string& filename);//载入图片
	int saveBmpFile(const string& filename);//载出图片
	void change_light_level(double level);//改变亮度
	void local_smoothing_method(int level);//局部平滑法
	void unlimited_pixel_smoothing_method(int level, int T);//超限像素平滑法
	void adaptive_median_filtering(int minSize, int maxSize);//自适应中值滤波
	void dowhite();//图片变白
	void format();
	void Canny_Edge_Detector();//canny边缘检测
	void ConvertRGB2GRAY(unsigned char* pData, unsigned char* pDatagray);//如果图片是24位真彩色，就把它转换为8为灰度图像
	//高斯卷积核生成函数，gaus是指向含有N个double类型数组的指针，size是高斯卷积核的尺寸，sigma是卷积核的标准差
	void GetGaussianKernel(double ** gaus, const int size, const double sigma);
	//高斯模糊的滤波操作，pDataGray是待滤波原始图像，imageGaussian是滤波后输出图像，gaus是一个指向含有N个double类型数组的指针，size是滤波核的尺寸
	void GaussianFilter(unsigned char * pDataGray, unsigned char * imageGaussian, double ** gaus, int size);
	//Sobel算子计算X、Y方向梯度和梯度方向角，imageGaussian原始灰度图像，SobelX是X方向梯度图像，SobelY是Y方向梯度图像，pointDrection是梯度方向角数组指针
	void SobelGradDirction(unsigned char * imageGaussian, unsigned char * SobelX, unsigned char * SobelY, double * pointDirection);
	//计算X、Y方向梯度融合幅值，SobelX，SobelY分别是XY方向梯度图像，SobelXY是XY梯度融合幅值
	void SobelAmplitude(unsigned char * SobelX, unsigned char * SobelY, unsigned char * SobelXY);
	//局部极大值抑制，SobelXY输入的Sobel梯度图像，imageLocalMax是输出的局部极大值抑制图像，pointDrection是图像上每个点的梯度方向数组指针
	void LocalMaxValue(unsigned char * SobelXY, unsigned char * imageLocalMax, double * pointDirection);
	//双阈值处理，imageLocalMax是非极大值抑制之后的图像，lowThreshold是低阈值，highThreshold是高阈值  
	void DoubleThreshold(unsigned char * imageLocalMax, double lowThreshold, double highThreshold);
	//双阈值中间像素连接处理，FinalImage是进行双阈值处理之后的图像，lowThreshold是低阈值，highThreshold是高阈值  
	void DoubleThresholdLink(unsigned char * FinalImage, double lowThreshold, double highThreshold);

	inline void resetBMFHeader();
	inline void resetBMIHeader();
	inline void releasePalette();
	inline void releaseImageData();
	inline void releaseData();
};


//==============================================================================

inline void BitMap::resetBMFHeader()
{
	bmfHeader_bfType = 0;
	bmfHeader.bfSize = 0;
	bmfHeader.bfReserved1 = 0;
	bmfHeader.bfReserved2 = 0;
	bmfHeader.bfOffBits = 0;
}

inline void BitMap::resetBMIHeader()
{
	bmiHeader.biSize = 0;
	bmiHeader.biWidth = 0;
	bmiHeader.biHeight = 0;
	bmiHeader.biPlanes = 0;
	bmiHeader.biBitCount = 0;
	bmiHeader.biCompression = 0;
	bmiHeader.biSizeImage = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
}

inline void BitMap::releasePalette()
{
	if (pPalette)
	{
		delete pPalette;
		pPalette = NULL;
	}

}
inline void BitMap::releaseImageData()
{
	if (pData)
	{
		delete pData;
		pData = NULL;
	}
}


inline void BitMap::releaseData()
{
	dataType = UNKNOW;

	resetBMFHeader();
	resetBMIHeader();
	releasePalette();
	releaseImageData();
}


#endif

