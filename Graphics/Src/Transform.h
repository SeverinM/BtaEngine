#ifndef H_TRANSFORM
#define H_TRANSFORM
#include "GLM/glm.hpp"
#include "GLM/ext/matrix_transform.hpp"

class Transform
{
private:
	glm::vec3 m_vPosition;
	glm::vec3 m_vScale;
	glm::mat4 m_mRotation;
public:
	Transform();
	void SetPosition(glm::vec3 vNewPosition, bool bRelative = false);
	void SetScale(glm::vec3 vNewScale, bool bRelative = false);
	void Rotate(glm::vec3 vAxis, float fDegrees);
	glm::mat4 GetModelMatrix();
};


#endif