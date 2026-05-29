#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function
	glm::vec2 tempLeft = glm::vec2(1.0f, x);
	glm::mat2 tempMiddle = glm::mat2(1.0f, -1.0f,
									 0.0f, 1.0f);
	glm::mat3x2 tempRight = glm::mat3x2(p0.x, p1.x,
										p0.y, p1.y,
										p0.z, p1.z);
	glm::vec3 result = tempLeft * tempMiddle * tempRight;

	return result;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	//! \todo Implement this function
	glm::vec4 tempLeft = glm::vec4(1.0f, x, glm::pow(x,2.0f), glm::pow(x,3.0f));
	glm::mat4 tempMiddle = glm::mat4(0.0f, -t, 2.0f*t, -t,
									 1.0f, 0.0f, t-3.0f, 2.0f-t,
									 0.0f, t, 3.0f-2.0f*t, t-2.0f,
									 0.0f, 0.0f, -t, t);
	glm::mat3x4 tempRight = glm::mat3x4(p0.x, p1.x, p2.x, p3.x,
										p0.y, p1.y, p2.y, p3.y,
										p0.z, p1.z, p2.z, p3.z);
	glm::vec3 result = tempLeft * tempMiddle * tempRight;

	return result;
}
