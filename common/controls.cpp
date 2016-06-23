#include <assert.h>
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


class MVP
{
protected:	
	glm::mat4 model_mat;
	glm::mat4 view_mat;
	glm::mat4 projection_mat;
public:
	virtual glm::mat4 getMVP(void);
};

class SteadyCamera : MVP
{
	/* steady camera moves object, seriously? */
public:
	glm::mat4 getMVP(float delta_time, float speed, glm::vec3& direction);
	glm::vec3 getDIR(void);
};

class FPSCamera : MVP
{
	glm::vec3 camera_pos; /** the last camera position */
	glm::vec3 up; /** in most case, the direction is (0,1,0) */
	/* the thing about fps camera is interesting, it always points to 1 unit
	 * in front of its position */
	glm::vec3 direction;  /** a unit vector which is the same as where the
				  camera points to, so the center becomes
				  camera_pos + direction */
	FPSCamera(glm::vec3& init_pos) {
		up = glm::vec3(0.0, 1.0, 0.0);
		camera_pos = glm::vec3(init_pos);
	}
public:
	glm::mat4 getMVP(float delta_time, float speed, glm::vec3& direction);
};

glm::mat4
SteadyCamera::getMVP(float delta_time, float speed, glm::vec3& direction)
{
	assert(delta_time > 0);
	assert(speed >= 0);
	/* direction should have norm 1 */
	this->model_mat = translate(direction * speed * (float)delta_time);
	return this->projection_mat * this->view_mat * this->model_mat;

}


/**
 * @brief fps version of get mvp
 *
 * The world matrix doesn't change, camera moves to any direction, within the shortime period
 */
glm::mat4
FPSCamera::getMVP(float delta_time, float speed, glm::vec3& direction)
{
	assert(delta_time > 0);
	assert(speed >= 0);

	this->camera_pos += direction * speed * delta_time;
	glm::mat4 mod_mat = glm::lookAt(camera_pos, camera_pos+ direction, up);
}
