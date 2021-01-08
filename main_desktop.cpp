#include "opencv2/opencv.hpp"
#include  "cartoon.h"

#define DEBUG 0
#define EDGES 0
#define DEVEL 0
#define ALIEN 1
int main(int argc, char* argv[]) {
	int cameraNumber = 0;
	if (argc > 1)
		cameraNumber = atoi(argv[1]);

	cv::VideoCapture camera;
	camera.open(cameraNumber);
	if (!camera.isOpened()) {
		std::cerr << "ERROR: Could not access the camera or video!" << std::endl;
		exit(1);
	}

	camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

	while (true) {
		cv::Mat cameraFrame;
		camera >> cameraFrame;
		if (cameraFrame.empty()) {
			std::cerr << "ERROR: Couldn't grab a camera frame." << std::endl;
			exit(1);
		}

		cv::Mat displayedFrame(cameraFrame.size(), CV_8UC3);
#if EDGES == 1
		cartoonifyImage(cameraFrame, displayedFrame);
#endif
#if DEVEL == 1
		cartoonDevImage(cameraFrame, displayedFrame);
#endif
#if ALIEN == 1
		cartoonifyImage(cameraFrame, displayedFrame, true);
		cartoonAlien(displayedFrame, displayedFrame);
#endif
#if DEBUG == 1
		imshow("original", cameraFrame);
#endif

		imshow("Cartoonifier", displayedFrame);

		char keypress = cv::waitKey(20);
		if (keypress == 27)
			break;
	}
	destroyAllWindows();
}