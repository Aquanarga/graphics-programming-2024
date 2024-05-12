#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <vector>

class Bone
{
public:
	Bone(glm::vec3 position, glm::quat angle, std::shared_ptr<Bone> child);
	void setChild(std::shared_ptr<Bone> child);
	glm::vec3 RunIK(glm::vec3 target);
	std::vector<glm::vec3> getCoordinates(std::vector<glm::vec3> output, glm::mat4 translationMatrix, glm::mat4 rotationMatrix);

private:
	glm::vec3 m_position;
	glm::quat m_angle;
	std::shared_ptr<Bone> m_child;
};