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

void Bone::setChild(std::shared_ptr<Bone> child) 
{
	m_child = child;
}

glm::vec3 Bone::RunIK(glm::vec3 target) 
{
	// If there is no child this is the end point
	if (not m_child) {
		std::cout << std::endl;
		return m_position;
	}

	// Convert target to local space. Local space includes a skewed coordinate system in this case (unlike when getting coordinates for drawing)
	glm::mat4 toLocalSpace = glm::toMat4(glm::inverse(m_angle)) * glm::translate(glm::mat4(1.0f), -m_position);
	glm::vec3 localTarget = glm::vec3(toLocalSpace * glm::vec4(target, 1.0f));

	//std::cout << "Target:      " << target.x << "," << target.y << "," << target.z << "," << std::endl;
	//std::cout << "LocalTarget: " << localTarget.x << "," << localTarget.y << "," << localTarget.z << "," << std::endl;
	//std::cout << std::endl;

	// Recursion
	glm::vec3 endPoint = m_child->RunIK(localTarget);

	//std::cout << "EndPoint: " << endPoint.x << "," << endPoint.y << "," << endPoint.z << "," << std::endl;
	//std::cout << std::endl;

	// Point towards the endpoint
	glm::quat shiftAngle = glm::rotation(glm::normalize(endPoint), glm::normalize(localTarget));
	m_angle *= shiftAngle;

	// Convert endpoint to parents local space
	glm::mat4 toParentSpace = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(m_angle);
	return glm::vec3(toParentSpace * glm::vec4(endPoint, 1.0f));
}

// TODO point with addition instead of translationMatrix, or apply matrixes better (aka, without ApplyMatrix function)
std::vector<glm::vec3> Bone::getCoordinates(std::vector<glm::vec3> output, glm::mat4 translationMatrix, glm::mat4 rotationMatrix)
{
	// Apply combined rotation to local position, then apply translation to get world location
	glm::vec3 worldSpace = ApplyMatrix(ApplyMatrix(m_position, rotationMatrix), translationMatrix);
	output.emplace_back(worldSpace);

	if (m_child)
	{
		// Update rotation matrix and create new translation matrix from the current position
		rotationMatrix = rotationMatrix * glm::toMat4(m_angle);
		return m_child->getCoordinates(output, glm::translate(glm::mat4(1.0f), worldSpace), rotationMatrix);
	}
	return output;
}