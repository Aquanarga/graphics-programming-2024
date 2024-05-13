#include "IKStructure.h"

// Helper function to apply mat4 to vec3
static glm::vec3 ApplyMatrix(glm::vec3 point, glm::mat4 matrix) {
	return glm::vec3(matrix * glm::vec4(point, 1.0f));
}

Joint::Joint(glm::vec3 position, glm::quat angle, std::shared_ptr<Joint> child)
{
	m_position = position;
	m_angle = angle;
	m_child = child;
}

void Joint::setChild(std::shared_ptr<Joint> child)
{
	m_child = child;
}

void Joint::Resize(int childAmount)
{
	if (childAmount == 0)
	{
		if(m_child)
			m_child = nullptr;
		return;
	}

	if (not m_child)
		m_child = std::make_shared<Joint>(Joint(glm::vec3(1.5, 0, 0), glm::quat(1.0f, glm::vec3()), nullptr));
	
	m_child->Resize(childAmount - 1);
}

glm::vec3 Joint::RunIK(glm::vec3 target)
{
	// If there is no child this is the end point
	if (not m_child) 
		return m_position;

	// Convert target to local space. Local space includes a skewed coordinate system in this case (unlike in getCoordinates())
	glm::mat4 toLocalSpace = glm::toMat4(glm::inverse(m_angle)) * glm::translate(glm::mat4(1.0f), -m_position);
	glm::vec3 localTarget = ApplyMatrix(target, toLocalSpace);

	// Recursion
	glm::vec3 endPoint = m_child->RunIK(localTarget);

	// Point the endpint towards the target
	glm::quat shiftAngle = glm::rotation(glm::normalize(endPoint), glm::normalize(localTarget));
	m_angle *= shiftAngle;

	// Convert endpoint to parents local space
	glm::mat4 toParentSpace = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(m_angle);
	return ApplyMatrix(endPoint, toParentSpace);
}

std::vector<glm::vec3> Joint::getCoordinates(std::vector<glm::vec3> output, glm::mat4 translationMatrix, glm::mat4 rotationMatrix)
{
	// Apply combined rotation to local position to get translation from parent
	// then apply the translation to get to world space
	glm::vec3 worldSpace = ApplyMatrix(ApplyMatrix(m_position, rotationMatrix), translationMatrix);
	output.emplace_back(worldSpace);

	// Recursion
	if (m_child)
	{
		// Update rotation matrix and create new translation matrix from the current world space position
		rotationMatrix = rotationMatrix * glm::toMat4(m_angle);
		return m_child->getCoordinates(output, glm::translate(glm::mat4(1.0f), worldSpace), rotationMatrix);
	}
	return output;
}

// Based on RunIK(), without actually doing the IK alogrithm part
glm::vec3 Joint::GetEndPoint()
{
	// If there is no child this is the end point
	if (not m_child)
		return m_position;

	// Recursion
	glm::vec3 endPoint = m_child->GetEndPoint();

	// Convert endpoint to parents local space
	glm::mat4 toParentSpace = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(m_angle);
	return glm::vec3(toParentSpace * glm::vec4(endPoint, 1.0f));
}