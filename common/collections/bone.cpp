#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <random>

#include <types.hpp>
#include <property.hpp>
#include <model.hpp>
#include <shaderman.h>


class JointAnim : OBJproperty {
protected:
	
public:
	virtual ~JointAnim() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
	
};
