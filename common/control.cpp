#include <assert.h>
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

//The first simple control api, rotating arcball

//arc-ball camera
//remeber, we only accept xpos, ypos in opengl coordinates system
