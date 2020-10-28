#include "Transform.h"

Transform::Transform(glm::mat4& mInitialModel)
{
	m_mModel = mInitialModel;
}

Transform::Transform()
{
	m_mModel = glm::mat4(1.0f);
}

void Transform::SetPosition(glm::vec3 vNewPosition, bool bRelative /*= false*/)
{
	if (!bRelative)
	{
		m_mModel[3][0] = 0.0f;
		m_mModel[3][1] = 0.0f;
		m_mModel[3][2] = 0.0f;
	}

	m_mModel = glm::translate(m_mModel, vNewPosition);
}

void Transform::SetScale(glm::vec3 vNewScale, bool bRelative /*= false*/)
{
	m_mModel = glm::scale(m_mModel, vNewScale);
}

void Transform::Rotate(glm::vec3 vAxis, float fDegrees)
{
	m_mModel = glm::rotate(m_mModel, glm::radians(fDegrees), vAxis);
}

glm::vec3 Transform::GetForward() const
{
	return glm::vec3(m_mModel[0][2], m_mModel[1][2], m_mModel[2][2]);
}

glm::vec3 Transform::GetUp() const
{
	return glm::vec3(m_mModel[0][1], m_mModel[1][1], m_mModel[2][1]);
}

glm::vec3 Transform::GetRight() const
{
	return glm::vec3(m_mModel[0][0], m_mModel[1][0], m_mModel[2][0]);
}

glm::vec3 Transform::GetPosition() const
{
	return glm::vec3(m_mModel[3][0], m_mModel[3][1], m_mModel[3][2]);
}

