#include "Transform.h"

Transform::Transform()
{
	m_mRotation = glm::mat4(1.0f);
	m_vScale = glm::vec3(1.0f);
	m_vPosition = glm::vec3(0.0f);
}

void Transform::SetPosition(glm::vec3 vNewPosition, bool bRelative /*= false*/)
{
	m_vPosition = (bRelative ? m_vPosition : glm::vec3(0.0f)) + vNewPosition;
}

void Transform::SetScale(glm::vec3 vNewScale, bool bRelative /*= false*/)
{
	m_vScale = (bRelative ? m_vScale : glm::vec3(0.0f)) + vNewScale;
}

void Transform::Rotate(glm::vec3 vAxis, float fDegrees)
{
	m_mRotation = glm::rotate(m_mRotation, glm::radians(fDegrees), vAxis);
}

glm::mat4 Transform::GetModelMatrix()
{
	glm::mat4 mOutput = glm::mat4(1.0f);
	mOutput = glm::scale(mOutput, m_vScale);
	mOutput = m_mRotation * mOutput;
	mOutput = glm::translate(mOutput, m_vPosition);
	return mOutput;
}

