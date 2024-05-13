#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <vector>

class Joint
{
public:
	Joint(glm::vec3 position, glm::quat angle, std::shared_ptr<Joint> child);
	void setChild(std::shared_ptr<Joint> child);

	// Iterates through all children, removing or adding them as needed to match the input number
	void Resize(int childAmount);

	// Runs a single cycle of the Inverse Kinematics algorithm Cyclic Coordinate Descent, rotating all the joints to reach the target
	glm::vec3 RunIK(glm::vec3 target);

	// Returns a list of coordinates in world space (assuming the root is in world space). Used by the sdf in order to render the arm
	std::vector<glm::vec3> getCoordinates(std::vector<glm::vec3> output, glm::mat4 translationMatrix, glm::mat4 rotationMatrix);

	// Returns the coordiantes for the end point in world space
	glm::vec3 GetEndPoint();

private:
	glm::vec3 m_position;
	glm::quat m_angle;
	std::shared_ptr<Joint> m_child;
};