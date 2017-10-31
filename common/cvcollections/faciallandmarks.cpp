#include <vector>
#include <exception>

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

class FacialLandmark {
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
}

int main(int argc, char *argv[])
{
	cv::Mat img = cv::imread(argv[1]);
	FacialLandmark face_tracker(argv[2]);
	face_tracker.getPoseModel(img);
	
	return 0;
}
