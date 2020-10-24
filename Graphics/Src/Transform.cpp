#include "Transform.h"

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

