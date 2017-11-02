#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


int main(int argc, char *argv[])
{
	cv::VideoCapture cap;
	cap.open(0);
	if (!cap.isOpened()) {
		std::cerr << "err openning camera" << std::endl;
		return -1;
	}
	cv::namedWindow("camera capture");
	while (1) {
		cv::Mat stream;
		cap >> stream;
		cv::imshow("camera capture", stream);
		if (cv::waitKey(20) == 'n')
			break;
	}
	return 0;
}
