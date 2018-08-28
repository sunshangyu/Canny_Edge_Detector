

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
struct BitMapFileHeader//�ļ�ͷ 
{
	// unsigned short bfType;  moved to class BitMap
	unsigned int   bfSize;//�ļ����ܴ�С 
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;//ͼ��ʼƫ���� 
};

struct BitMapInfoHeader//ͼ����Ϣͷ 
{
	unsigned int biSize;//��Ϣͷ��С 
	int biWidth;//ͼ��Ŀ�� 
	int biHeight;//ͼ��ĸ߶� 
	unsigned short biPlanes;//ͼ��λƽ����Ŀ 
	unsigned short biBitCount;//һ������ռ�ݵ�λ�� 
	unsigned int biCompression;//ѹ������,�ݲ����� 
	unsigned int biSizeImage;//ѹ��ͼ���С,�ݲ����� 
	int biXPelsPerMeter;//ˮƽ�ֱ���
	int biYPelsPerMeter;//��ֱ�ֱ���
	unsigned int biClrUsed;//��ɫ��Ŀ����ɫ����Ŀ 
	unsigned int biClrImportant;//����Ҫ����ɫ����Ŀ 
};

struct RGBQuad//��ɫ���ձ� 
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
};

class BitMap//ͼ���� 
{
public:
	enum DataType { UNKNOW, RGB, RGBA, FORMATTED_RGB };

	DataType dataType;//ʮ�����ƣ��洢�����ͣ� 

	unsigned short bmfHeader_bfType;//ͼ������ ��BM�� 
	struct BitMapFileHeader bmfHeader;//ͼ����ļ���Ϣͷ 
	struct BitMapInfoHeader bmiHeader;//ͼ���ͼ����Ϣͷ 
	struct RGBQuad * pPalette;//ָ����ɫ���ձ��ָ��
	int paletteSize;//��ɫ��Ĵ�С
	unsigned char * pData;
	int imageSize;//ͼ���С 

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

	int loadBmpFile(const string& filename);//����ͼƬ
	int saveBmpFile(const string& filename);//�س�ͼƬ
	void change_light_level(double level);//�ı�����
	void local_smoothing_method(int level);//�ֲ�ƽ����
	void unlimited_pixel_smoothing_method(int level, int T);//��������ƽ����
	void adaptive_median_filtering(int minSize, int maxSize);//����Ӧ��ֵ�˲�
	void dowhite();//ͼƬ���
	void format();
	void Canny_Edge_Detector();//canny��Ե���
	void ConvertRGB2GRAY(unsigned char* pData, unsigned char* pDatagray);//���ͼƬ��24λ���ɫ���Ͱ���ת��Ϊ8Ϊ�Ҷ�ͼ��
	//��˹��������ɺ�����gaus��ָ����N��double���������ָ�룬size�Ǹ�˹����˵ĳߴ磬sigma�Ǿ���˵ı�׼��
	void GetGaussianKernel(double ** gaus, const int size, const double sigma);
	//��˹ģ�����˲�������pDataGray�Ǵ��˲�ԭʼͼ��imageGaussian���˲������ͼ��gaus��һ��ָ����N��double���������ָ�룬size���˲��˵ĳߴ�
	void GaussianFilter(unsigned char * pDataGray, unsigned char * imageGaussian, double ** gaus, int size);
	//Sobel���Ӽ���X��Y�����ݶȺ��ݶȷ���ǣ�imageGaussianԭʼ�Ҷ�ͼ��SobelX��X�����ݶ�ͼ��SobelY��Y�����ݶ�ͼ��pointDrection���ݶȷ��������ָ��
	void SobelGradDirction(unsigned char * imageGaussian, unsigned char * SobelX, unsigned char * SobelY, double * pointDirection);
	//����X��Y�����ݶ��ںϷ�ֵ��SobelX��SobelY�ֱ���XY�����ݶ�ͼ��SobelXY��XY�ݶ��ںϷ�ֵ
	void SobelAmplitude(unsigned char * SobelX, unsigned char * SobelY, unsigned char * SobelXY);
	//�ֲ�����ֵ���ƣ�SobelXY�����Sobel�ݶ�ͼ��imageLocalMax������ľֲ�����ֵ����ͼ��pointDrection��ͼ����ÿ������ݶȷ�������ָ��
	void LocalMaxValue(unsigned char * SobelXY, unsigned char * imageLocalMax, double * pointDirection);
	//˫��ֵ����imageLocalMax�ǷǼ���ֵ����֮���ͼ��lowThreshold�ǵ���ֵ��highThreshold�Ǹ���ֵ  
	void DoubleThreshold(unsigned char * imageLocalMax, double lowThreshold, double highThreshold);
	//˫��ֵ�м��������Ӵ���FinalImage�ǽ���˫��ֵ����֮���ͼ��lowThreshold�ǵ���ֵ��highThreshold�Ǹ���ֵ  
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

