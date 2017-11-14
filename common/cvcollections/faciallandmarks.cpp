#include <iostream>
#include <vector>
#include <exception>
#include <thread>
#include <functional>
#include <chrono>
#include <mutex>
#include <condition_variable>

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

#include <Eigen/Core>
#include <Eigen/Dense>


#include <context.hpp>

class FacialLandmark {
	//you need to thread this thing. Because you need to process detect the
	//head rectangle before you do landmark, the second one is faster...
protected:
	cv::Rect rect_ann, rect_blob;
	dlib::frontal_face_detector face_detector;
	dlib::shape_predictor pose_model;
	dlib::image_window win;
public:
	//or we can do the samething like with the shader, but not really a good idea, it is 1.2M
	FacialLandmark(const std::string& landmark_path);
	~FacialLandmark();

//	void getFacePoints(cv::Mat &frame);
	void getPoseModel(cv::Mat& frame);
	void copyPoints(const dlib::full_object_detection&);
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
		win.clear_overlay();
		win.set_image(cimg);
		win.add_overlay(dlib::render_face_detections(detected));
	}
}

void
FacialLandmark::copyPoints(const dlib::full_object_detection& detection)
{
	Eigen::Matrix<float, 2, Eigen::Dynamic> mat(2, detection.num_parts());
	std::vector<dlib::point> face_points(mat.cols());
	for (int i = 0; i < 68; i ++) {
		detection.part(i);
	}

}


//in this every special case a 40ms timer will start increase the semaphore(data_avaliable).
//the first thread is never get locked by others. It only wait for 40ms then increase a lock of others

static std::condition_variable CV;
static std::mutex TMTX;
static bool nextFrame;
static bool quit=false;
static std::string landmark_file;

bool worker(void)
{
	FacialLandmark face_tracker(landmark_file);
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
		face_tracker.getPoseModel(img);
	}
	return true;
}

#include <thread>
#include <chrono>
#include <mutex>



int main(int argc, char *argv[])
{
	int manytimes=1000000;
	//create our thread as well
	std::thread twork(worker);
	landmark_file = argv[1];

	while(manytimes-- > 0) {
		{
			std::lock_guard<std::mutex> lk(TMTX);
			nextFrame = true;
			CV.notify_all();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		//std::cout << "sleeped" << std::endl;
	}
	quit = true;
	return 0;
}
