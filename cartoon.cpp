#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

#define DEBUG 0
void cartoonifyImage(InputArray src, OutputArray dst) {
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
	/*Mat output = dst.getMat();
	for (int j = 0; j < mask.rows; j++) {
		for (int i = 0; i < mask.cols; i++) {
			output.at<uchar>(j, i) = mask.at<uchar>(j, i);
		}
	}*/
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
}