

struct KeyFrame {
	//for the animation's convience, we need the datatype cooperates with time
	float timeStamp;
	std::map<std::string, JointTransform> transforms;
//	std::vector<JointTransform> transforms;
};

class JointAnim : public OBJproperty {
protected:
	std::vector<KeyFrame> keyframes;
public:
	virtual ~JointAnim() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
	
};



//gota decide whether to 
class JointTransform {
public:
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;
	std::string joint_name;

	JointTransform(const std::string& name = "",
		const glm::vec3& t = glm::vec3(0.0f),
		const glm::quat& r = glm::quat(glm::vec3(0.0f)),
		const glm::vec3& s = glm::vec3(1.0f));
	const glm::mat4 getLocalTransform();
	static JointTransform interpolate(const JointTransform& a, const JointTransform& b, float progression);
	static glm::vec3 interpolate(const glm::vec3& a, const glm::vec3& b, float progression);
};


JointTransform::JointTransform(const std::string& name,
			       const glm::vec3& t, const glm::quat& r, const glm::vec3& s)
{
	this->joint_name = name;
	this->translation  = t;
	this->rotation  = r;
	this->scale = s;
}

JointTransform
JointTransform::interpolate(const JointTransform &a, const JointTransform &b, float progression)
{
	assert(a.joint_name == b.joint_name);
	glm::vec3 trans = (1-progression) * a.translation + progression * b.translation;
	glm::vec3 scale = (a.scale != b.scale) ?
		((1-progression) * a.scale + progression * b.scale)
		: a.scale;
	glm::quat quaternion = glm::slerp(a.rotation, b.rotation, progression);
	return JointTransform(a.joint_name, trans, quaternion, scale);
}

glm::vec3
JointTransform::interpolate(const glm::vec3 &a, const glm::vec3 &b, float progression)
{
	return (1-progression) * a + progression * b;
}



struct Animation {
	double seconds;
	//you r gonna have memory problem with it.
	std::vector<KeyFrame> keyframes;
};

static bool need_interpolate(const double current_time, double& prev_frame, double& next_frame,
			     const std::set<double>& timestamps)
{
	std::set<double>::const_iterator itr = timestamps.lower_bound(current_time);
	if (itr == timestamps.end() || *itr == current_time)
		return false;
	next_frame = *itr;
	//this only happens when we don't have 0 frame and we inserted 0. We also want avoid this
	prev_frame = (itr != timestamps.begin()) ? (*(--itr)) : current_time;
	if (prev_frame == current_time)
		return false;
	else if (next_frame > current_time && current_time > prev_frame)
		return true;
	else
		return false;
}


int
Model::loadAnimations(const aiScene* scene)
{
//	std::cout << "I am here, with " << scene->mNumAnimations << " animations" << std::endl;
//	this->animations.resize(scene->mNumAnimations);
	for (uint i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation *anim = scene->mAnimations[i];
		Animation local_anim;
		std::set<double> global_timestamps;
		
		std::map<double, KeyFrame> all_keyframes;

		//XXX: so not true!!! it is harmless to insert the 0 timestamp
//		KeyFrame first_frame;
//		first_frame.timeStamp = 0;
//		for (auto bone_itr = this->bones.cbegin(); bone_itr != this->bones.end(); bone_itr++)
//			first_frame.transforms[bone_itr->first] = JointTransform(bone_itr->first);
//		all_keyframes[0.0] = first_frame;
//		local_anim.seconds = anim->mTicksPerSecond * anim->mDuration;
//		global_timestamps.insert(0);
		
		for (uint k = 0; k < anim->mNumChannels; k++) {
			std::set<double> timestamps;
			aiNodeAnim *bone_anim = anim->mChannels[k];
			std::string bone_name = bone_anim->mNodeName.C_Str();
			//now I need to interpolate keyframes
			for (uint itt = 0; itt < bone_anim->mNumPositionKeys; itt++) {
//				double current_time = bone_anim->mPositionKeys[itt].mTime;
				double current_time = value_at_precision(bone_anim->mPositionKeys[itt].mTime, 2);
				aiVector3D value = bone_anim->mPositionKeys[itt].mValue;
				KeyFrame& keyframe = all_keyframes[current_time];
				keyframe.timeStamp = current_time;
				keyframe.transforms[bone_name] = JointTransform(bone_name, glm::vec3(value.x, value.y, value.z));
				
				global_timestamps.insert(current_time);
				timestamps.insert(current_time);
			}
			for (uint itr = 0; itr < bone_anim->mNumRotationKeys; itr++) {
//				double current_time = bone_anim->mPositionKeys[itr].mTime;				
				double current_time = value_at_precision(bone_anim->mRotationKeys[itr].mTime, 2);				
				aiQuaternion value = bone_anim->mRotationKeys[itr].mValue;
				KeyFrame& keyframe = all_keyframes[current_time];
				keyframe.timeStamp = current_time;
				JointTransform& batframe = keyframe.transforms[bone_name];
				batframe.joint_name = bone_name;
				batframe.rotation = glm::quat(value.w, value.x, value.y ,value.z);

				double next_tstamp, prev_tstamp;
				if (need_interpolate(current_time, prev_tstamp, next_tstamp, timestamps)) {
					glm::vec3 last_transform = all_keyframes[prev_tstamp].transforms[bone_name].translation;
					glm::vec3 next_transform = all_keyframes[next_tstamp].transforms[bone_name].translation;
					batframe.translation = JointTransform::interpolate(last_transform, next_transform,
											   (current_time - prev_tstamp)/(next_tstamp - prev_tstamp) );
					if (glm::isnan(batframe.translation)[0])
						std::cerr << "lol, found a bug" << std::endl;
					
				}
				global_timestamps.insert(current_time);
				timestamps.insert(current_time);
			}
			for (uint its = 0; its < bone_anim->mNumScalingKeys; its++) {
//				double current_time = bone_anim->mPositionKeys[its].mTime;
				double current_time = value_at_precision(bone_anim->mScalingKeys[its].mTime, 2);
				aiVector3D value = bone_anim->mScalingKeys[its].mValue;
				KeyFrame& keyframe = all_keyframes[current_time];
				keyframe.timeStamp = current_time;
				JointTransform& batframe = keyframe.transforms[bone_name];
				batframe.joint_name = bone_name;
				batframe.scale = glm::vec3(value.x, value.y, value.z);

				double next_tstamp, prev_tstamp;
				if (need_interpolate(current_time, prev_tstamp, next_tstamp, timestamps)) {
					JointTransform& last_transform = all_keyframes[prev_tstamp].transforms[bone_name];
					JointTransform& next_transform = all_keyframes[next_tstamp].transforms[bone_name];
					batframe = JointTransform::interpolate(last_transform, next_transform,
											   (current_time - prev_tstamp)/(next_tstamp - prev_tstamp) );
				}
				global_timestamps.insert(current_time);
				timestamps.insert(current_time);
			}
//			std::cout << bone_anim->mNumPositionKeys << " translations, ";
//			std::cout << bone_anim->mNumRotationKeys << " rotations, and ";
//			std::cout << bone_anim->mNumScalingKeys << " scales\n";
//			(void)bone_anim->mNumPositionKeys;
//			(void)bone_anim->mNumRotationKeys;
//			(void)bone_anim->mNumScalingKeys;
		}
		/*
		std::cerr << "number of keyframes for this model: " << all_keyframes.size() << std::endl;
		for (auto itr = all_keyframes.begin(); itr != all_keyframes.end(); itr++) {
			std::cerr << "current timestamp" << itr->first << std::endl;
			for (auto itj = itr->second.transforms.begin(); itj != itr->second.transforms.end(); itj++) {
				std::cerr << "\tbone: " << itj->first << "\t" << glm::to_string(itj->second.translation);
				std::cerr << '\t' << glm::to_string(itj->second.rotation) <<'\t' << glm::to_string(itj->second.scale) << std::endl;
			}
		}
		*/
		//copy constructor is called
		this->animations[std::string(anim->mName.C_Str())] = local_anim;
	}
}
