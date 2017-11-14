#include <iostream>
#include <vector>
#include <exception>
#include <thread>
#include <functional>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

std::condition_variable CV;
std::mutex TMTX;
bool nextFrame;
bool quit=false;

bool worker(void)
{
	cv::VideoCapture cap;
	if(cap.open(0) == false) {
		std::cout << "never got a chance to execute" << std::endl;
		return false;
	}
	cv::namedWindow("cap");

	while(!quit) {
		std::unique_lock<std::mutex> lk(TMTX);
		CV.wait(lk);
		//now we have the data
		nextFrame = false;
		lk.unlock();
		//no need to notify, I am the only dude do this now
		cv::Mat img;
		cap >> img;
		cv::imshow("cap", img); // you don't need waitKeyAnymore
		cv::waitKey(1);//you always need this.I don't know why
		std::cout << "get a frame" << std::endl;
		//		CV.notify_one();
	}
}

int main(int argc, char *argv[])
{
	int manytimes=1000000;
	//create our thread as well
	std::thread twork(worker);


	while(manytimes-- > 0) {
		{
			std::lock_guard<std::mutex> lk(TMTX);
			nextFrame = true;
			CV.notify_one();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
		//std::cout << "sleeped" << std::endl;
	}
	quit = true;
	return 0;
}
