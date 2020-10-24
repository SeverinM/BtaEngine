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
	Transform() : m_mRotation(glm::mat4(1.0f)), m_vScale(glm::vec3(1.0f)), m_vPosition(glm::vec3(0.0f)) {};
	void SetPosition(glm::vec3 vNewPosition, bool bRelative = false);
	void SetScale(glm::vec3 vNewScale, bool bRelative = false);
	void Rotate(glm::vec3 vAxis, float fDegrees);
	glm::mat4 GetModelMatrix()
	{
		glm::mat4 mOutput = glm::mat4(1.0f);
		mOutput = glm::scale(mOutput, m_vScale);
		mOutput = m_mRotation * mOutput;
		mOutput = glm::translate(mOutput, m_vPosition);
		return mOutput;
	}
};


#endif