#ifndef UTILS_H
#define UTILS_H


#include <vector>
#include <GL/glew.h>


#include <GLFW/glfw3.h>
#include <string>


int read_file(const char *fname, std::string *str);

/**
 * @brief glfw and glew init code, all state machine code
 *
 */

GLFWwindow* tutorial_init(int width, int height,
			  void (*keyboard_callback)(GLFWwindow*, int, int, int, int),
			  void (*cursor_callback) (GLFWwindow *, double, double));

void tutorial_terminate(void);
void update_window(GLFWwindow* window);
#endif /* UTILS_H */
