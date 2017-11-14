#include <iostream>
#include <vector>
#include <exception>
#include <thread>
#include <functional>

#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/gui_widgets.h>


#include <context.hpp>

class FacialLandmark {
	//you need to thread this thing. Because you need to process detect the
	//head rectangle before you do landmark, the second one is faster...
protected:
	cv::Rect rect_ann, rect_blob;
	dlib::frontal_face_detector face_detector;
	dlib::shape_predictor pose_model;
public:
	//or we can do the samething like with the shader, but not really a good idea, it is 1.2M
	FacialLandmark(const std::string& landmark_path);
	~FacialLandmark();

//	void getFacePoints(cv::Mat &frame);
	void getPoseModel(cv::Mat& frame);
};



FacialLandmark::FacialLandmark(const std::string& landmark_path)
{
	//it has exceptions, so I will skip
	dlib::deserialize(landmark_path) >>  pose_model;
	this->face_detector = dlib::get_frontal_face_detector();
	//since it has a frontal face detector, I guess the process should be easy enough
}

FacialLandmark::~FacialLandmark()
{

}

void
FacialLandmark::getPoseModel(cv::Mat &frame)
{
	dlib::full_object_detection detected;
	dlib::cv_image<dlib::bgr_pixel> cimg(frame);
	std::vector<dlib::rectangle> faces = this->face_detector(cimg);
	//we just use the first face
	if (faces.size() > 0) {
		detected = this->pose_model(cimg, faces[0]);
	}

	dlib::image_window win;
	win.clear_overlay();
	win.set_image(cimg);
	win.add_overlay(dlib::render_face_detections(detected));
	int c;
	std::cin >> c;
}

#include <thread>
#include <chrono>
#include <mutex>


//for one thing, I can use template and pluging some of function. But it is useless, because
class ThreadedContext : public context {
public:

};

class Timer {
protected:
	int _ms;
	void timer(void) {
		while(true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(this->_ms));
			std::cout << "the timer is ticking" << std::endl;
		}
	}

	std::shared_ptr<std::thread> time_thread;

public:
	Timer(unsigned int ms) {
		_ms = ms;
		this->time_thread = std::make_shared<std::thread>(std::bind(&Timer::timer, this));
		//looks like this will be very specific
		//you will have to use
	}
	~Timer(void) {
		this->time_thread->join();
	}

	//I can friend myself
//	friend void Timer::timer(Timer *th);
};


int main(int argc, char *argv[])
{

	cv::VideoCapture cap;
//	if (!cap.open(0))
//		return -1;
//	std::thread t1(timer, 40);
//	t1.join();
//	cv::Mat img = cv::imread(argv[1]);
//	FacialLandmark face_tracker(argv[2]);
//	face_tracker.getPoseModel(img);

	return 0;
}
