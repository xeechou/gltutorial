#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


GLuint
load_texture(const char *image_path)
{
	GLuint textureID;
	unsigned int width, height;
	      
	//load image first, return 0 if we failed to load image.
	//okay, how can I load the image
	cv::Mat rgb_img;
	cv::Mat img = cv::imread(image_path, CV_LOAD_IMAGE_COLOR);
	
	//this should give me 3byte pixel data
	if (!img.data) {
		fprintf(stderr, "Failed to load image: %s\n", image_path);
		return 0;
	}
	img.convertTo(rgb_img, CV_8UC3);
	
	fprintf(stdout, "the image byte is %d\n", img.depth());
	width = img.cols;
	height= img.rows;
	//wait a minute, it would not be as simple as that, what is the size of pixel?
	//After loading the image
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0/*border*/, GL_BGR, GL_UNSIGNED_BYTE, rgb_img.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	return textureID;
	//img will be deleted here
}
