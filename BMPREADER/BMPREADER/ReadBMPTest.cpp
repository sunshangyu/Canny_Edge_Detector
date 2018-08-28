#include "bmp.h"

int main()
{
	BitMap bmpImage;
	//bmpImage.loadBmpFile("../input8.bmp");
	/*
	bmpImage.loadBmpFile("../input8.bmp");
	bmpImage.local_smoothing_method(3);
	bmpImage.saveBmpFile("out_local_smoothing_8.bmp");
	bmpImage.loadBmpFile("../input8.bmp");
	bmpImage.unlimited_pixel_smoothing_method(3, 64);
	bmpImage.saveBmpFile("out_unlimited_pixel_smoothing_8.bmp");
	bmpImage.loadBmpFile("../input24.bmp");
	bmpImage.local_smoothing_method(3);
	bmpImage.saveBmpFile("out_local_smoothing_24.bmp");
	bmpImage.loadBmpFile("../input24.bmp");
	bmpImage.unlimited_pixel_smoothing_method(3, 64);
	bmpImage.saveBmpFile("out_unlimited_pixel_smoothing_24.bmp");
	*/
	//bmpImage.change_light_level(1.2);
	//bmpImage.dowhite();
	//bmpImage.saveBmpFile("out_input.bmp");
	//system("pause");

	/*
	bmpImage.loadBmpFile("../adaptive_median_filtering_24_color.bmp");
	bmpImage.adaptive_median_filtering(3, 7);
	bmpImage.saveBmpFile("out_adaptive_median_filtering_24_color.bmp");
	bmpImage.loadBmpFile("../adaptive_median_filtering_8_gray.bmp");
	bmpImage.adaptive_median_filtering(3, 7);
	bmpImage.saveBmpFile("out_adaptive_median_filtering_8_gray.bmp");
	*/
	bmpImage.loadBmpFile("../canny_edge_detector.bmp");
	bmpImage.Canny_Edge_Detector();
	bmpImage.saveBmpFile("out_canny_edge_detector.bmp");
	bmpImage.loadBmpFile("../lena.bmp");
	bmpImage.Canny_Edge_Detector();
	bmpImage.saveBmpFile("out_lena.bmp");
	//system("pause");
	return 0;
}