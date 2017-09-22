#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <shaderman.h>

//okay, I am gonna put the shader in a header file lol
class AnimShader : ShaderMan {
	//classes like this are used to setup uniforms more easily, so the
	//corresponding applications know which shader it is atually using
private:
	
public:
	AnimShader();
	~AnimShader();
};
