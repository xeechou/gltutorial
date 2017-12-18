#ifndef T_GEOMETRY_HPP
#define T_GEOMETRY_HPP


#include "../model.hpp"
/* this is actually a bad design. I can either write public functions:
   makeCube, makeUVSphere, makeCylinder, makePyramid something like that */
class CubeModel : public Model {

public:
	//this will give you a one-by-one cube
	CubeModel(const glm::vec3 translation = glm::vec3(0.0f),
		  const glm::vec3 scale = glm::vec3(1.0f),
		  const glm::quat rotation = glm::quat(glm::vec3(0.0f)));
	//void SetColor(glm::vec4 color);
};

//basically we need to write the use the different mesh property
class isoSphere : public Model {
public:
	isoSphere(float radius);
};


#endif /* EOF */
