#include "IKStructure.h"

static glm::vec3 ApplyMatrix(glm::vec3 point, glm::mat4 matrix) {
	return glm::vec3(matrix * glm::vec4(point, 1.0f));
}

Bone::Bone(glm::vec3 position, glm::quat angle, std::shared_ptr<Bone> child)
{
	m_position = position;
	m_angle = angle;
	m_child = child;
}

void Bone::setChild(std::shared_ptr<Bone> child) {
	m_child = child;
}

glm::vec3 Bone::RunIK(glm::vec3 target) {
	glm::mat4 transform_matrix = glm::toMat4(-m_angle) * glm::translate(glm::mat4(1.0f), -m_position);
	glm::vec3 localTarget = glm::vec3(transform_matrix * glm::vec4(target, 1.0f));

	// std::cout << "Target:      " << target.x << "," << target.y << "," << target.z << "," << std::endl;
	// std::cout << "LocalTarget: " << localTarget.x << "," << localTarget.y << "," << localTarget.z << "," << std::endl;

	glm::vec3 endPoint;

	if (m_child)
		endPoint = m_child->RunIK(localTarget);
	else 
		// base case:  the end point is the end of the current bone
		return m_position;

	// TODO The rest of the function
	// point towards the endpoint
	//const shiftAngle = angle(localTarget) - angle(endPoint);
	//this.angle += shiftAngle;

	// convert back to parent coordinate space
	//return translatePoint(rotatePoint(endPoint, this.angle), this.x, this.y);

	return target;
}

// TODO Apply transformation to world space
std::vector<glm::vec3> Bone::getCoordinates(std::vector<glm::vec3> output, glm::mat4 viewmatrix) {
	output.emplace_back(ApplyMatrix(m_position, viewmatrix));
	if (m_child)
		return m_child->getCoordinates(output, viewmatrix);
	return output;
}