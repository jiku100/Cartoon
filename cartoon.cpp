#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

#define DEBUG 0
void cartoonifyImage(InputArray src, OutputArray dst, bool isAlien = false) {
	Mat srcColor = src.getMat();

// 에지 검출
	Mat gray;
	cvtColor(srcColor, gray, COLOR_BGR2GRAY);
#if DEBUG == 1
	imshow("gray", gray);
#endif
	const int MEDIAN_BLUR_FILTER_SIZE = 7;
	medianBlur(gray, gray, MEDIAN_BLUR_FILTER_SIZE);
	//GaussianBlur(gray, gray, Size(), 1);
#if DEBUG == 1
	imshow("medianBlur", gray);
#endif
	Mat edges;
	const int LAPLACIAN_FILTER_SIZE = 5;
	Laplacian(gray, edges, CV_8U, LAPLACIAN_FILTER_SIZE);
#if DEBUG == 1
	imshow("edges", edges);
#endif
	Mat mask;
	const int EDGES_THRESHOLD = 80;
	threshold(edges, mask, EDGES_THRESHOLD, 255, THRESH_BINARY_INV);
	dst.create(mask.size(), mask.type());

#if DEBUG == 1
	imshow("Mask", mask);
#endif
// 컬러 페인팅
	Size size = srcColor.size();
	Size smallSize;
	smallSize.width = size.width / 2;
	smallSize.height = size.height / 2;
	Mat smallImg = Mat(smallSize, CV_8UC3);
	resize(srcColor, smallImg, smallSize, 0, 0, INTER_LINEAR);

	Mat tmp = Mat(smallSize, CV_8UC3);
	int repetitions = 7;
	for (int i = 0; i < repetitions; i++) {
		int ksize = 7;
		double sigmaColor = 9;
		double sigmaSpace = 5;
		bilateralFilter(smallImg, tmp, ksize, sigmaColor, sigmaSpace);
		bilateralFilter(tmp, smallImg, ksize, sigmaColor, sigmaSpace);
	}
#if DEBUG == 1
	imshow("bilateral", smallImg);
#endif
	Mat bigImg;
	resize(smallImg, bigImg, size, 0, 0, INTER_LINEAR);
	dst.setTo(0);
	bigImg.copyTo(dst, mask);

	if (isAlien) {
		Mat yuv = Mat(smallSize, CV_8UC3);
		cvtColor(smallImg, yuv, COLOR_BGR2YCrCb);

		int sw = smallSize.width;
		int sh = smallSize.height;
		Mat maskAlien, maskPlusBorder;
		maskPlusBorder = Mat::zeros(sh + 2, sw + 2, CV_8UC1);
		maskAlien = maskPlusBorder(Rect(1, 1, sw, sh));
		resize(edges, maskAlien, smallSize);
		threshold(maskAlien, maskAlien, EDGES_THRESHOLD, 255, THRESH_BINARY);
		dilate(maskAlien, maskAlien, Mat());
		erode(maskAlien, maskAlien, Mat());

		const int NUM_SKIN_POINTS = 6;
		Point skinPts[NUM_SKIN_POINTS];
		skinPts[0] = Point(sw / 2, sh / 2 - sh / 6);
		skinPts[1] = Point(sw / 2 - sw / 11, sh / 2 - sh / 6);
		skinPts[2] = Point(sw / 2 + sw / 11, sh / 2 - sh / 6);
		skinPts[3] = Point(sw / 2, sh / 2 + sh / 16);
		skinPts[4] = Point(sw / 2 - sw / 9, sh / 2 + sh / 16);
		skinPts[5] = Point(sw / 2 + sw / 9, sh / 2 + sh / 16);

		const int LOWER_Y = 60;
		const int UPPER_Y = 70;
		const int LOWER_Cr = 25;
		const int UPPER_Cr = 15;
		const int LOWER_Cb = 20;
		const int UPPER_Cb = 15;
		Scalar lowerDiff = Scalar(LOWER_Y, LOWER_Cr, LOWER_Cb);
		Scalar upperDiff = Scalar(UPPER_Y, UPPER_Cr, UPPER_Cb);

		const int CONNECTED_COMPONENTS = 4;	//대각선까지 하려면 8
		const int flags = CONNECTED_COMPONENTS | FLOODFILL_FIXED_RANGE | FLOODFILL_MASK_ONLY;
		Mat edgeMask = maskAlien.clone();
		for (int i = 0; i < NUM_SKIN_POINTS; i++) {
			floodFill(yuv, maskPlusBorder, skinPts[i], Scalar(), NULL, lowerDiff, upperDiff, flags);
		}
		maskAlien -= edgeMask;
		int Red = 0;
		int Green = 70;
		int Blue = 0;
		add(smallImg, CV_RGB(Red, Green, Blue), smallImg, maskAlien);

		resize(smallImg, bigImg, size, 0, 0, INTER_LINEAR);
		dst.setTo(0);
		bigImg.copyTo(dst, mask);
	}
}
void cartoonDevImage(InputArray src, OutputArray dst) {
	Mat srcColor = src.getMat();

	// 에지 검출
	Mat gray;
	cvtColor(srcColor, gray, COLOR_BGR2GRAY);
#if DEBUG == 1
	imshow("gray", gray);
#endif
	const int MEDIAN_BLUR_FILTER_SIZE = 7;
	medianBlur(gray, gray, MEDIAN_BLUR_FILTER_SIZE);
#if DEBUG == 1
	imshow("medianBlur", gray);
#endif
	Mat edges, edges2;
	Scharr(gray, edges, CV_8U, 1, 0);
	Scharr(gray, edges2, CV_8U, 0, 1);
	edges += edges2;
#if DEBUG == 1
	imshow("edge", edges);
#endif
	Mat mask;
	const int EVIL_EDGE_THRESHOLD = 12;
	threshold(edges, mask, EVIL_EDGE_THRESHOLD, 255, THRESH_BINARY_INV);
	medianBlur(mask, mask, 3);
#if DEBUG == 1
	imshow("mask", mask);
#endif
	dst.setTo(0);
	srcColor.copyTo(dst, mask);
}
void cartoonAlien(InputArray src, OutputArray dst) {
	Mat srcColor = src.getMat();
	Size size = srcColor.size();
	Mat faceOutline = Mat::zeros(size, CV_8UC3);
	Scalar color = CV_RGB(255, 255, 0); // == Scalar(0,255,255)
	int thickness = 4;
	int sw = size.width;
	int sh = size.height;
	int faceH = sh / 2 * 70 / 100;
	int faceW = faceH * 72 / 100;
	ellipse(faceOutline, Point(sw / 2, sh / 2), Size(faceW, faceH), 0, 0, 360, color, thickness, LINE_AA);

	int eyeW = faceW * 23 / 100;
	int eyeH = faceH * 11 / 200;
	int eyeX = faceW * 48 / 100;
	int eyeY = faceH * 13 / 100;
	Size eyeSize = Size(eyeW, eyeH);
	int eyeA = 15;
	int eyeYshift = 5;
	ellipse(faceOutline, Point(sw / 2 - eyeX, sh / 2 - eyeY), eyeSize, 0, 180 + eyeA, 360 - eyeA, color, thickness, LINE_AA);
	ellipse(faceOutline, Point(sw / 2 - eyeX, sh / 2 - eyeY - eyeYshift), eyeSize, 0, 0 + eyeA, 180 - eyeA, color, thickness, LINE_AA);
	ellipse(faceOutline, Point(sw / 2 + eyeX, sh / 2 - eyeY), eyeSize, 0, 180 + eyeA, 360 - eyeA, color, thickness, LINE_AA);
	ellipse(faceOutline, Point(sw / 2 + eyeX, sh / 2 - eyeY - eyeYshift), eyeSize, 0, 0 + eyeA, 180 - eyeA, color, thickness, LINE_AA);

	int mouthY = faceH * 48 / 100;
	int mouthW = faceW * 45 / 100;
	int mouthH = faceH * 6 / 100;
	ellipse(faceOutline, Point(sw / 2, sh / 2 + mouthY), Size(mouthW, mouthH), 0, 0, 180, color, thickness, LINE_AA);

	int fontFace = FONT_HERSHEY_COMPLEX;
	float fontScale = 1.0f;
	int fontThickness = 2;
	String szMsg = "Put your face here";
	putText(faceOutline, szMsg, Point(sw * 23 / 100, sh * 10 / 100), fontFace, fontScale, color, fontThickness, LINE_AA);
	dst.create(srcColor.size(), srcColor.type());
	addWeighted(srcColor, 1.0, faceOutline, 0.7, 0, dst, CV_8UC3);
}