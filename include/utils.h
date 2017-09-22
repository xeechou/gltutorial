#ifndef UTILS_H
#define UTILS_H


#include <cmath>
#include <vector>
#include <GL/glew.h>


#include <GLFW/glfw3.h>
#include <string>

#include <assimp/scene.h>
#include <assimp/postprocess.h>


template<typename T>
T value_at_precision(const T val, unsigned int precision)
{
	return (std::round(val * std::pow(10, precision))  /
		std::pow(10, precision));
}

int read_file(const std::string& fname, std::string& str);

/**
 * @brief glfw and glew init code, all state machine code
 *
 */
GLFWwindow* tutorial_init(int width, int height,
			  void (*keyboard_callback)(GLFWwindow*, int, int, int, int) = NULL,
			  void (*cursor_callback) (GLFWwindow *, double, double) = NULL,
			  void (*scroll_callback) (GLFWwindow *, double, double) = NULL);

void tutorial_terminate(void);
void update_window(GLFWwindow* window);


#endif /* UTILS_H */
